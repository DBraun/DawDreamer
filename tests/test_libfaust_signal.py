# adapted from:
# https://github.com/grame-cncm/faust/blob/master-dev/tools/benchmark/signal-tester.cpp

from dawdreamer_utils import *
from typing import List

BUFFER_SIZE = 1
SAMPLE_RATE = 44100


def test1():
    """
    process = 0.5;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    signals = []
    f = engine.make_faust_processor("faust")

    signals.append(f.sigReal(0.5))
    f.compile_signals("test", signals)


def test2():

    """
    process = _ <: +(0.5), *(1.5);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    signals = []
    f = engine.make_faust_processor("faust")

    in1 = f.sigInput(0)
    signals.append(f.sigAdd(in1, f.sigReal(0.5)))
    signals.append(f.sigMul(in1, f.sigReal(1.5)))
    f.compile_signals("test", signals)


def test3():

    """
    process = _ <: @(+(0.5), 500), @(*(1.5), 3000);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = f.sigInput(0)
    signals.append(f.sigDelay(f.sigAdd(in1, f.sigReal(0.5)), f.sigReal(500)))
    signals.append(f.sigDelay(f.sigMul(in1, f.sigReal(1.5)), f.sigReal(3000)))

    f.compile_signals("test", signals)


def test4():

    """
    process = _ <: @(500) + 0.5, @(3000) * 1.5;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = f.sigInput(0)

    signals.append(f.sigAdd(f.sigDelay(in1, f.sigReal(500)), f.sigReal(0.5)))
    signals.append(f.sigMul(f.sigDelay(in1, f.sigReal(3000)), f.sigReal(1.5)))

    f.compile_signals("test", signals)


# def test6():

#     """
#     process = _ <: @(+(0.5), 500), @(*(1.5), 3000);
#     """

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")
#     signals = []

#     in1 = f.sigInput(0)
#     signals.append(f.sigDelay(f.sigAdd(in1, f.sigReal(0.5)), f.sigReal(500)))
#     signals.append(f.sigDelay(f.sigMul(in1, f.sigReal(1.5)), f.sigReal(3000)))

#     argv = ["-vec", "-lv", "1", "-double"]

#     f.compile_signals("test", signals, argv)


def test_equivalent1():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = f.sigInput(0)
    s1 = f.sigDelay(f.sigAdd(in1, f.sigReal(0.5)), f.sigReal(500))
    signals.append(s1)
    signals.append(s1)

    f.compile_signals("test", signals)


def test_equivalent2():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = f.sigInput(0)
    signals.append(f.sigDelay(f.sigAdd(in1, f.sigReal(0.5)), f.sigReal(500)))
    signals.append(f.sigDelay(f.sigAdd(in1, f.sigReal(0.5)), f.sigReal(500)))

    f.compile_signals("test", signals)


def test8():
    """
    process = @(+(0.5), 500) * vslider("Vol", 0.5, 0, 1, 0.01);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = f.sigInput(0)

    slider = f.sigVSlider("Vol", f.sigReal(0.5), f.sigReal(0.), f.sigReal(1.), f.sigReal(.01))

    signals.append(f.sigMul(slider, f.sigDelay(f.sigAdd(in1, f.sigReal(0.5)), f.sigReal(500))))

    f.compile_signals("test", signals)


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

    freq = f.sigVSlider("h:Oscillator/freq", f.sigReal(440), f.sigReal(50), f.sigReal(1000.), f.sigReal(.1))
    gain = f.sigVSlider("h:Oscillator/gain", f.sigReal(0), f.sigReal(0), f.sigReal(1), f.sigReal(.011))

    signals.append(f.sigMul(freq, f.sigMul(gain, f.sigInput(0))))

    f.compile_signals("test", signals)


def test10():
    """
    process = + ~ _;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []
    in1 = f.sigInput(0)

    signals.append(f.sigRecursion(f.sigAdd(f.sigSelf(), in1)))

    f.compile_signals("test", signals)


def test11():
    """
    import("stdfaust.lib");
    process = ma.SR, ma.BS;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    signals.append(f.sigSampleRate())
    signals.append(f.sigBufferSize())

    f.compile_signals("test", signals)


def test12():
    """
    process = waveform { 0, 100, 200, 300, 400 };
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    signals += f.sigWaveform([i*100. for i in range(5)])

    f.compile_signals("test", signals)


def test14():
    """
    process = _ <: +;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = f.sigInput(0)
    signals.append(f.sigAdd(in1, in1))

    f.compile_signals("test", signals)


def test15():
    """
    process = _,_ <: !,_,_,! :> _,_;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = f.sigInput(0)
    in2 = f.sigInput(1)

    signals.append(in2)
    signals.append(in1)

    f.compile_signals("test", signals)


def test16():
    """
    process = _,_,_,_ : _,!,!,_;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    in1 = f.sigInput(0)
    in3 = f.sigInput(3)

    signals.append(in1)
    signals.append(in3)

    f.compile_signals("test", signals)


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

#     rdx = f.sigInt(0)
#     chan = f.sigInt(0)
#     part = f.sigInt(0)

#     signals += f.sigSoundfile("sound[url:{'tango.wav'}]", rdx, chan, part)

#     f.compile_signals("test", signals)


def test_overload_add1():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = (f.sigInput(0) + f.sigReal(0.5)) + (f.sigInput(0) + f.sigInt(1))

    signals.append(s1)

    f.compile_signals("test", signals)

def test_overload_add2():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = (f.sigInput(0) + 0.5) + (f.sigInput(0) + 1)

    signals.append(s1)

    f.compile_signals("test", signals)

def test_overload_add3():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = sum([f.sigInput(0), f.sigReal(0.5)]) + sum([f.sigInput(0), f.sigInt(1)])

    signals.append(s1)

    f.compile_signals("test", signals)

def test_overload_sub1():
    """
    //
    process(x) = (x - .5) + (x - 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = (f.sigInput(0) - f.sigReal(0.5)) + (f.sigInput(0) - f.sigInt(1))

    signals.append(s1)

    f.compile_signals("test", signals)

def test_overload_sub2():
    """
    //
    process(x) = (x - .5) + (x - 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = (f.sigInput(0) - 0.5) + (f.sigInput(0) - 1)

    signals.append(s1)

    f.compile_signals("test", signals)


def test_overload_mul1():
    """
    //
    process = _ * 0.5
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = f.sigInput(0) * f.sigReal(0.5)

    signals.append(s1)

    f.compile_signals("test", signals)


def test_overload_mul2():
    """
    //
    process = _ * 0.5
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = f.sigInput(0) * 0.5

    signals.append(s1)

    f.compile_signals("test", signals)


def test_overload_mul3():
    """
    //
    process = _ * 2
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = f.sigInput(0) * 2

    signals.append(s1)

    f.compile_signals("test", signals)

def test_overload_mul3():
    """
    //
    process = _ * 2
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    signals = []

    s1 = f.sigInput(0) * f.sigInt(2)

    signals.append(s1)

    f.compile_signals("test", signals)


if __name__ == "__main__":
    test_overload_add2()
