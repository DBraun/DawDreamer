from dawdreamer_utils import *

BUFFER_SIZE = 1

def test_add_processor():

    """
    This example isn't meant to sound meaningful. It just demonstrates taking a single
    input ("drums"), passing it to multiple other processors, and then mixing those
    processors into a single stereo out. It's meaningful to test a system
    that has more internal channels (8) than inputs (2) or outputs (2).
    """

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)

    drums = engine.make_playback_processor("drums",
      load_audio_file(ASSETS / "Music Delta - Disco" / "drums.wav", duration=DURATION))

    def get_filter(i):
      return engine.make_filter_processor(f"filter{i}", "low", 18_000, .5, 1.)

    add_processor = engine.make_add_processor('eq', [.25, .25, .25, .25])

    graph = [
        (drums, []),
        (get_filter(0), ["drums"]),
        (get_filter(1), ["drums"]),
        (get_filter(2), ["drums"]),
        (get_filter(3), ["drums"]),
        (add_processor, ["filter0", "filter1", "filter2", "filter3"])
    ]

    assert add_processor.get_num_input_channels() == 8
    assert add_processor.get_num_output_channels() == 2

    engine.load_graph(graph)


    engine.render(DURATION)

    audio = engine.get_audio()

    wavfile.write(OUTPUT / 'test_add_processor.wav', SAMPLE_RATE, audio.transpose())

    assert(abs(audio.shape[1]-int(DURATION*SAMPLE_RATE)) <= 1)

    assert(np.mean(np.abs(audio)) > .01)

# if __name__ == '__main__':
#     test_add_processor()
