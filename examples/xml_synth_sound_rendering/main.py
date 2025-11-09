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
import multiprocessing
import os
import time
import traceback
from collections import namedtuple
from glob import glob
from os import makedirs
from pathlib import Path

from scipy.io import wavfile
from tqdm import tqdm

# import custom utils for helper functions
from utils import load_xml_preset, make_json_parameter_mapping

# extra libraries to install with pip
import dawdreamer as daw

Item = namedtuple("Item", "preset_path")


class TAL_UNO_Worker:
    def __init__(
        self,
        queue: multiprocessing.Queue,
        plugin_path: str,
        sample_rate=44100,
        block_size=512,
        bpm=120,
        note_duration=2,
        render_duration=5,
        pitch_low=60,
        pitch_high=72,
        velocity=100,
        output_dir="output",
    ):
        self.queue = queue
        self.sample_rate = sample_rate
        self.block_size = block_size
        self.bpm = bpm
        self.plugin_path = plugin_path
        self.note_duration = note_duration
        self.render_duration = render_duration
        self.pitch_low, self.pitch_high = pitch_low, pitch_high
        self.velocity = velocity
        self.output_dir = Path(output_dir)

    def startup(self):
        engine = daw.RenderEngine(self.sample_rate, self.block_size)
        engine.set_bpm(self.bpm)

        synth = engine.make_plugin_processor("synth", self.plugin_path)

        graph = [(synth, [])]
        engine.load_graph(graph)

        self.engine = engine
        self.synth = synth

    def process_item(self, item: Item):
        preset_path = item.preset_path
        json_mapping = make_json_parameter_mapping(
            self.synth, preset_path, os.path.join(os.path.dirname(__file__), "parameter_mappings")
        )
        self.synth = load_xml_preset(self.synth, json_mapping)
        basename = os.path.basename(preset_path)

        for pitch in range(self.pitch_low, self.pitch_high + 1):
            self.synth.add_midi_note(pitch, self.velocity, 0.0, self.note_duration)
            self.engine.render(self.render_duration)
            self.synth.clear_midi()
            audio = self.engine.get_audio()
            output_path = self.output_dir / f"{pitch}_{basename}.wav"
            wavfile.write(str(output_path), self.sample_rate, audio.transpose())

    def run(self):
        try:
            self.startup()
            while True:
                try:
                    item = self.queue.get_nowait()
                    self.process_item(item)
                except multiprocessing.queues.Empty:
                    break
        except Exception:
            return traceback.format_exc()


def main(
    plugin_path,
    preset_dir,
    note_duration=2,
    render_duration=4,
    pitch_low=60,
    pitch_high=60,
    num_workers=None,
    output_dir="output",
    logging_level="DEBUG",
):
    # Create logger
    logging.basicConfig()
    logger = logging.getLogger("dawdreamer")
    logger.setLevel(logging_level.upper())

    # Glob all the preset file paths, looking shallowly only.
    # The .pjunoxl is formatted just like an xml file.
    preset_paths = list(glob(str(Path(preset_dir) / "*.pjunoxl")))

    # Get num items so that the progress bar works well
    num_items = len(preset_paths)

    # Create a Queue and add items
    input_queue = multiprocessing.Manager().Queue()
    for preset_path in preset_paths:
        input_queue.put(Item(preset_path))

    # Create a list to hold the worker processes
    workers = []

    # The number of workers to spawn
    num_processes = num_workers or multiprocessing.cpu_count()

    # Debug info
    logger.info(f"Note duration: {note_duration}")
    logger.info(f"Render duration: {render_duration}")
    logger.info(f"Using num workers: {num_processes}")
    logger.info(f"Pitch low: {pitch_low}")
    logger.info(f"Pitch high: {pitch_high}")
    logger.info(f"Output directory: {output_dir}")

    makedirs(output_dir, exist_ok=True)

    # Create a multiprocessing Pool
    with multiprocessing.Pool(processes=num_processes) as pool:
        # Create and start a worker process for each CPU
        for i in range(num_processes):
            worker = TAL_UNO_Worker(
                input_queue,
                plugin_path,
                note_duration=note_duration,
                render_duration=render_duration,
                pitch_low=pitch_low,
                pitch_high=pitch_high,
                output_dir=output_dir,
            )
            async_result = pool.apply_async(worker.run)
            workers.append(async_result)

        # Use tqdm to track progress. Update the progress bar in each iteration.
        pbar = tqdm(total=num_items)
        while True:
            incomplete_count = sum(1 for w in workers if not w.ready())
            pbar.update(pbar.total - incomplete_count - pbar.n)
            if incomplete_count == 0:
                break
            time.sleep(0.1)
        pbar.close()

    # Check for exceptions in the worker processes
    for i, worker in enumerate(workers):
        exception = worker.get()
        if exception is not None:
            logger.error(f"Exception in worker {i}:\n{exception}")

    logger.info("All done!")


if __name__ == "__main__":
    # We're using multiprocessing.Pool, so our code MUST be inside __main__.
    # See https://docs.python.org/3/library/multiprocessing.html

    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--plugin", required=True, help="Path to plugin instrument (.dll, .vst3).")
    parser.add_argument("--preset-dir", required=True, help="Directory path of plugin presets.")
    parser.add_argument("--note-duration", default=2, help="Note duration in seconds.")
    parser.add_argument("--pitch-low", default=60, help="Lowest MIDI pitch to be used.")
    parser.add_argument("--pitch-high", default=64, help="Highest MIDI pitch to be used.")
    parser.add_argument("--render-duration", default=4, help="Render duration in seconds.")
    parser.add_argument("--num-workers", default=None, help="Number of workers to use.")
    parser.add_argument(
        "--output-dir",
        default=os.path.join(os.path.dirname(__file__), "output"),
        help="Output directory.",
    )
    parser.add_argument(
        "--log-level",
        default="DEBUG",
        choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "NOTSET"],
        help="Logger level.",
    )
    args = parser.parse_args()

    main(
        args.plugin,
        args.preset_dir,
        args.note_duration,
        args.render_duration,
        args.pitch_low,
        args.pitch_high,
        args.num_workers,
        args.output_dir,
        args.log_level,
    )
