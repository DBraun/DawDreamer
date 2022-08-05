# adapted from:
# https://github.com/grame-cncm/faust/blob/master-dev/tools/benchmark/signal-tester.cpp

from dawdreamer_utils import *
from dawdreamer.faust.signal import *
from typing import List
import inspect

from dawdreamer.faust import createLibContext, destroyLibContext

BUFFER_SIZE = 1
SAMPLE_RATE = 44100


def with_lib_context(func):

    """
    The safest way to use either the signal API or box API is to wrap
    the function in a call that creates the lib context and a call
    that destroys the lib context.
    """

    def wrapped(*args, **kwargs):
        createLibContext()
        result = func(*args, **kwargs)
        destroyLibContext()
        return result

    return wrapped


def my_render(engine, f, duration=5.):

    engine.load_graph([(f, [])])
    test_name = inspect.stack()[1][3]
    file_path = OUTPUT / f"test_signals_{test_name}.wav"
    
    render(engine, file_path=file_path, duration=duration)    


@with_lib_context
def test1():
    """
    process = 0.5;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    signals = []
    f = engine.make_faust_processor("faust")

    signals.append(sigReal(0.5))
    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test2():

    """
    process = _ <: +(0.5), *(1.5);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    signals = []
    f = engine.make_faust_processor("faust")

    in1 = sigInput(0)
    signals.append(sigAdd(in1, sigReal(0.5)))
    signals.append(sigMul(in1, sigReal(1.5)))
    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test3():

    """
    process = _ <: @(+(0.5), 500), @(*(1.5), 3000);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = sigInput(0)
    signals.append(sigDelay(sigAdd(in1, sigReal(0.5)), sigReal(500)))
    signals.append(sigDelay(sigMul(in1, sigReal(1.5)), sigReal(3000)))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test4():

    """
    process = _ <: @(500) + 0.5, @(3000) * 1.5;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = sigInput(0)

    signals.append(sigAdd(sigDelay(in1, sigReal(500)), sigReal(0.5)))
    signals.append(sigMul(sigDelay(in1, sigReal(3000)), sigReal(1.5)))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test6():

    """
    process = _ <: @(+(0.5), 500), @(*(1.5), 3000);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = sigInput(0)
    signals.append(sigDelay(sigAdd(in1, sigReal(0.5)), sigReal(500)))
    signals.append(sigDelay(sigMul(in1, sigReal(1.5)), sigReal(3000)))

    # todo: use more argv
    argv = ["-vec"]

    f.compile_signals("test", signals, argv)
    my_render(engine, f)


@with_lib_context
def test_equivalent1():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = sigInput(0)
    s1 = sigDelay(sigAdd(in1, sigReal(0.5)), sigReal(500))
    signals.append(s1)
    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_equivalent2():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = sigInput(0)
    signals.append(sigDelay(sigAdd(in1, sigReal(0.5)), sigReal(500)))
    signals.append(sigDelay(sigAdd(in1, sigReal(0.5)), sigReal(500)))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test8():
    """
    process = @(+(0.5), 500) * vslider("Vol", 0.5, 0, 1, 0.01);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = sigInput(0)

    slider = sigVSlider("Vol", sigReal(0.5), sigReal(0.), sigReal(1.), sigReal(.01))

    signals.append(sigMul(slider, sigDelay(sigAdd(in1, sigReal(0.5)), sigReal(500))))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test9():
    """
    import("stdfaust.lib");
     
    freq = vslider("h:Oscillator/freq", 440, 50, 1000, 0.1);
    gain = vslider("h:Oscillator/gain", 0, 0, 1, 0.01);
     
    process = freq * gain;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    freq = sigVSlider("h:Oscillator/freq", sigReal(440), sigReal(50), sigReal(1000.), sigReal(.1))
    gain = sigVSlider("h:Oscillator/gain", sigReal(0), sigReal(0), sigReal(1), sigReal(.011))

    signals.append(sigMul(freq, sigMul(gain, sigInput(0))))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test10():
    """
    process = + ~ _;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = sigInput(0)

    signals.append(sigRecursion(sigAdd(sigSelf(), in1)))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test11():
    """
    import("stdfaust.lib");
    process = ma.SR, ma.BS;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    signals.append(sigSampleRate())
    signals.append(sigBufferSize())

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test12():
    """
    process = waveform { 0, 100, 200, 300, 400 };
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    signals += sigWaveform([i*100. for i in range(5)])

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test14():
    """
    process = _ <: +;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = sigInput(0)
    signals.append(sigAdd(in1, in1))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test15():
    """
    process = _,_ <: !,_,_,! :> _,_;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = sigInput(0)
    in2 = sigInput(1)

    signals.append(in2)
    signals.append(in1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test16():
    """
    process = _,_,_,_ : _,!,!,_;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = sigInput(0)
    in3 = sigInput(3)

    signals.append(in1)
    signals.append(in3)

    f.compile_signals("test", signals)
    my_render(engine, f)


# @with_lib_context
# def test19():

#     # todo: 'soundfile' primitive not yet supported for interp
#     # but we could print out the C++ code.

#     """
#     // soundfile test:
#     process = 0,0 : soundfile("sound[url:{'tango.wav'}]", 2);
#     """

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")
#     signals = []

#     rdx = sigInt(0)
#     chan = sigInt(0)
#     part = sigInt(0)

#     signals += sigSoundfile("sound[url:{'tango.wav'}]", rdx, chan, part)

#     f.compile_signals("test", signals)
#     my_render(engine, f)


@with_lib_context
def test20():

    """
    process = 10,1,int(_) : rdtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    signals.append(sigReadOnlyTable(sigInt(10), sigInt(1), sigInput(0)))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test21():

    """
    process = 10,1,int(_),int(_),int(_) : rwtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    signals.append(sigWriteReadTable(sigInt(10), sigInt(1), sigInput(0), sigInput(1), sigInput(2)))

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test22():

    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    signals.append(osc(f, sigVSlider("h:Oscillator/Freq1", sigReal(300), sigReal(100), sigReal(2000), sigReal(0.01))))
    signals.append(osc(f, sigVSlider("h:Oscillator/Freq2", sigReal(500), sigReal(100), sigReal(2000), sigReal(0.01))))

    f.compile_signals("test", signals)
    desc = f.get_parameters_description()
    assert len(desc) >= 2
    my_render(engine, f)
    audio = engine.get_audio().T
    assert np.mean(np.abs(audio)) > .1


@with_lib_context
def test24():

    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    # Follow the freq/gate/gain convention, see: https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters
    freq = sigNumEntry("freq", sigReal(100), sigReal(100), sigReal(3000), sigReal(0.01))
    gate = sigButton("gate")
    gain = sigNumEntry("gain", sigReal(0.5), sigReal(0), sigReal(1), sigReal(0.01))
    organ = sigMul(gate, sigAdd(sigMul(osc(f, freq), gain), sigMul(osc(f, sigMul(freq, sigInt(2))), gain)))
    organ *= 0.2
    signals = [organ, organ]

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'
    f.load_midi(abspath(ASSETS / midi_basename))

    f.num_voices = 10
    f.compile_signals("test", signals)

    my_render(engine, f)
    audio = engine.get_audio().T
    assert np.mean(np.abs(audio)) > .01


def decimalpart(f, x):
    return sigSub(x, sigIntCast(x))


def phasor(f, freq):
    return sigRecursion(decimalpart(f, sigAdd(sigSelf(), sigDiv(freq, sigSampleRate()))))    


def osc(f, freq):
    return sigSin(sigMul(phasor(f, freq), sigMul(sigReal(2.0), sigReal(3.141592653))))


@with_lib_context
def test_overload_add1():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = (sigInput(0) + sigReal(0.5)) + (sigInput(0) + sigInt(1))

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_overload_add2():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = (sigInput(0) + 0.5) + (sigInput(0) + 1)

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_overload_add3():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = sum([sigInput(0), sigReal(0.5)]) + sum([sigInput(0), sigInt(1)])

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_overload_sub1():
    """
    //
    process(x) = (x - .5) + (x - 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = (sigInput(0) - sigReal(0.5)) + (sigInput(0) - sigInt(1))

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_overload_sub2():
    """
    //
    process(x) = (x - .5) + (x - 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = (sigInput(0) - 0.5) + (sigInput(0) - 1)

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_overload_mul1():
    """
    //
    process = _ * 0.5
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = sigInput(0) * sigReal(0.5)

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_overload_mul2():
    """
    //
    process = _ * 0.5
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = sigInput(0) * 0.5

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_overload_mul3():
    """
    //
    process = _ * 2
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = sigInput(0) * 2

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


@with_lib_context
def test_overload_mul3():
    """
    //
    process = _ * 2
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = sigInput(0) * sigInt(2)

    signals.append(s1)

    f.compile_signals("test", signals)
    my_render(engine, f)


if __name__ == "__main__":
    test_overload_add2()
