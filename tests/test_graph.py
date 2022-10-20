from dawdreamer_utils import *

def test_empty_graph():

    """
    Throw an error if you call render on an emtpy graph.
    """

    DURATION = 5.

    engine = daw.RenderEngine(SAMPLE_RATE, 1)

    drums = engine.make_playback_processor("drums",
      load_audio_file(ASSETS / "Music Delta - Disco" / "drums.wav", duration=DURATION))

    with pytest.raises(Exception):
      engine.render(DURATION)
    
    audio = engine.get_audio()
    audio = drums.get_audio()