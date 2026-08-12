#
# This file is part of the DawDreamer distribution (https://github.com/DBraun/DawDreamer).
# Copyright (c) 2023 David Braun.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

import logging
import os
import queue
from collections import namedtuple
from concurrent.futures import ThreadPoolExecutor
from glob import glob
from pathlib import Path

from scipy.io import wavfile
from tqdm import tqdm

# extra libraries to install with pip
import dawdreamer as daw

Item = namedtuple("Item", "preset_path")


class Worker:
    """A worker with a persistent RenderEngine that renders presets from a shared queue.

    DawDreamer releases the GIL while rendering, so one worker per thread
    renders in parallel inside a single process.
    """

    def __init__(
        self,
        work_queue: queue.Queue,
        pbar: tqdm,
        plugin_path: str,
        sample_rate: int = 44100,
        block_size: int = 512,
        bpm: float = 120,
        note_duration: float = 2,
        render_duration: float = 5,
        pitch_low: int = 60,
        pitch_high: int = 72,
        velocity: int = 100,
        output_dir: str = "output",
    ):
        self.queue = work_queue
        self.pbar = pbar
        self.sample_rate = sample_rate
        self.block_size = block_size
        self.bpm = bpm
        self.plugin_path = plugin_path
        self.note_duration = note_duration
        self.render_duration = render_duration
        self.pitch_low, self.pitch_high = pitch_low, pitch_high
        self.velocity = velocity
        self.output_dir = Path(output_dir)

    def startup(self) -> None:
        """Create this worker's RenderEngine and load the plugin."""
        engine = daw.RenderEngine(self.sample_rate, self.block_size)
        engine.set_bpm(self.bpm)

        synth = engine.make_plugin_processor("synth", self.plugin_path)

        graph = [(synth, [])]
        engine.load_graph(graph)

        self.engine = engine
        self.synth = synth

    def process_item(self, item: Item) -> None:
        """Render one preset across the configured pitch range and write WAV files."""
        preset_path = item.preset_path
        self.synth.load_preset(preset_path)
        basename = os.path.basename(preset_path)

        for pitch in range(self.pitch_low, self.pitch_high + 1):
            self.synth.add_midi_note(pitch, self.velocity, 0.0, self.note_duration)
            self.engine.render(self.render_duration)
            self.synth.clear_midi()
            audio = self.engine.get_audio()
            output_path = self.output_dir / f"{pitch}_{basename}.wav"
            wavfile.write(str(output_path), self.sample_rate, audio.transpose())

    def run(self) -> None:
        """Consume the queue until it's empty."""
        self.startup()
        while True:
            try:
                item = self.queue.get_nowait()
            except queue.Empty:
                break
            self.process_item(item)
            self.pbar.update(1)


def main(
    plugin_path,
    preset_dir,
    sample_rate=44100,
    bpm=120,
    note_duration=2,
    render_duration=4,
    pitch_low=60,
    pitch_high=60,
    num_workers=None,
    output_dir="output",
    logging_level="INFO",
):
    # Create logger
    logging.basicConfig()
    logger = logging.getLogger("dawdreamer")
    logger.setLevel(logging_level.upper())

    # Glob all the preset file paths, looking shallowly only
    preset_paths = list(glob(str(Path(preset_dir) / "*.fxp")))

    # Get num items so that the progress bar works well
    num_items = len(preset_paths)

    # Create a Queue and add items
    input_queue = queue.Queue()
    for preset_path in preset_paths:
        input_queue.put(Item(preset_path))

    # The number of worker threads
    num_threads = num_workers or os.cpu_count()

    # Log info
    logger.info(f"Note duration: {note_duration}")
    logger.info(f"Render duration: {render_duration}")
    logger.info(f"Using num workers: {num_threads}")
    logger.info(f"Pitch low: {pitch_low}")
    logger.info(f"Pitch high: {pitch_high}")
    logger.info(f"Output directory: {output_dir}")

    os.makedirs(output_dir, exist_ok=True)

    # tqdm's update() is thread-safe, so all workers share one progress bar.
    with tqdm(total=num_items) as pbar, ThreadPoolExecutor(max_workers=num_threads) as executor:
        workers = [
            Worker(
                input_queue,
                pbar,
                plugin_path,
                sample_rate=sample_rate,
                bpm=bpm,
                note_duration=note_duration,
                render_duration=render_duration,
                pitch_low=pitch_low,
                pitch_high=pitch_high,
                output_dir=output_dir,
            )
            for _ in range(num_threads)
        ]
        futures = [executor.submit(worker.run) for worker in workers]

        # Propagate any exception raised in a worker thread.
        for future in futures:
            future.result()

    logger.info("All done!")


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--plugin", required=True, help="Path to plugin instrument (.dll, .vst3).")
    parser.add_argument("--preset-dir", required=True, help="Directory path of plugin presets.")
    parser.add_argument(
        "--sample-rate", default=44100, type=int, help="Sample rate for the plugin."
    )
    parser.add_argument(
        "--bpm", default=120, type=float, help="Beats per minute for the Render Engine."
    )
    parser.add_argument("--note-duration", default=2, type=float, help="Note duration in seconds.")
    parser.add_argument(
        "--pitch-low", default=60, type=int, help="Lowest MIDI pitch to be used (inclusive)."
    )
    parser.add_argument(
        "--pitch-high", default=60, type=int, help="Highest MIDI pitch to be used (inclusive)."
    )
    parser.add_argument(
        "--render-duration", default=4, type=float, help="Render duration in seconds."
    )
    parser.add_argument(
        "--num-workers", default=None, type=int, help="Number of worker threads to use."
    )
    parser.add_argument(
        "--output-dir",
        default=os.path.join(os.path.dirname(__file__), "output"),
        help="Output directory.",
    )
    parser.add_argument(
        "--log-level",
        default="INFO",
        choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "NOTSET"],
        help="Logger level.",
    )
    args = parser.parse_args()

    main(
        args.plugin,
        args.preset_dir,
        args.sample_rate,
        args.bpm,
        args.note_duration,
        args.render_duration,
        args.pitch_low,
        args.pitch_high,
        args.num_workers,
        args.output_dir,
        args.log_level,
    )
