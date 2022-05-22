# DJ Mixing

This script demonstrates transitioning from one audio file to another. You should create Ableton Live warp markers (`.asd` files) for the audio files which are being warped. If you don't have Ableton Live warp markers, you will need to modify the script to set properties on each of the PlaybackWarp Processors. These are the properties:

* .warp_markers (np.array [N, 2]) : List of pairs of (time in samples, time in beats)
* .start_marker (float) : Start marker position in beats relative to 1.1.1
* .end_marker (float) : End marker position in beats relative to 1.1.1
* .loop_start (float) : Loop start position in beats relative to 1.1.1
* .loop_end (float) : Loop end position in beats relative to 1.1.1
* .warp_on (bool) : Whether warping is enabled
* .loop_on (bool) : Whether looping is enabled

Some simple math and knowledge of the audio file's BPM sample rate can help you create the minimum two warp markers. Suppose the warp markers are (44100, 0.) and (66150, 1.). This would imply that the 1.1.1 starting position (the first quarter note) is at the 44100th audio sample and that the second quarter note is at the 66150th sample.