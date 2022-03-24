# author: David Braun (braun@ccrma.stanford.edu)

"""
This file takes the bitKlavier Grand Piano dataset and turns it into 88 wav files, named 0.wav through 87.wav.
You can customize a few parameters directly below:
"""


def main():

    print("Making piano samples.")

    import soundfile as sf
    import pyrubberband as pyrb
    import os.path

    velocity = 'v8'
    src_dir = 'E:/data/bitKlavierGrand_PianoBar_44k16b'
    duration_seconds = 10.
    output_dir = os.path.abspath('bitKlavierGrand_PianoBar_converted')

    # No need to modify below here:

    os.makedirs(output_dir, exist_ok=True)

    pairs = [
        ('A0', 0),  # A
        ('A0', 1),  # A#
        ('C1', -1),  # B
        ('C1', 0),  # C
        ('C1', 1),  # C#
        ('D#1', -1),  # D
        ('D#1', 0),  # D#
        ('D#1', 1),  # E
        ('F#1', -1),  # F
        ('F#1', 0),  # F#
        ('F#1', 1),  # G
        ('A1', -1),  # G#
    ]

    num_completed = 0
    is_done = False
    octave = 0

    while not is_done:

        for i, (input_note, shift) in enumerate(pairs):

            note = input_note[:-1] + str(octave if i < 2 else octave + 1)

            filepath = os.path.join(os.path.normpath(src_dir), f"{note}{velocity}.wav")
            print(filepath)
            y, sr = sf.read(filepath, always_2d=True)

            y = y[:int(sr*(duration_seconds+1.)), :]  # limit the time

            y_stretch = pyrb.pitch_shift(y, sr, shift)

            y_stretch = y_stretch[:int(sr*duration_seconds), :]  # limit the time

            output_filepath = os.path.join(output_dir, f"{num_completed}{velocity}.wav")
            sf.write(output_filepath, y_stretch, sr)

            num_completed += 1
            is_done = num_completed >= 88
            if is_done:
                break

        octave += 1

if __name__ == '__main__':
    main()

