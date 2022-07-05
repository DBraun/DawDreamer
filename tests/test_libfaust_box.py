# adapted from:
# https://github.com/grame-cncm/faust/blob/master-dev/tools/benchmark/box-tester.cpp

from dawdreamer_utils import *
from typing import List
import inspect

BUFFER_SIZE = 1
SAMPLE_RATE = 44100


def my_render(engine, f, duration=5.):

    engine.load_graph([(f, [])])
    test_name = inspect.stack()[1][3]
    file_path = OUTPUT / f"test_box_{test_name}.wav"
    
    render(engine, file_path=file_path, duration=duration)    


def test1():
    """
    process = 7,3.14;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("faust")

    box = f.boxPar(f.boxInt(7), f.boxReal(3.14))
    f.compile_box("test", box)
    my_render(engine, f)


def test2():

    """
    process = _,3.14 : +;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("faust")

    box = f.boxSeq(f.boxPar(f.boxWire(), f.boxReal(3.14)), f.boxAdd())

    f.compile_box("test", box)
    my_render(engine, f)


def test3():

    """
    // Alternate version with the binary 'boxAdd' version
    process = +(_,3.14);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxAdd(f.boxWire(), f.boxReal(3.14))

    f.compile_box("test", box)
    my_render(engine, f)


def test4():

    """
    process = _,_ : +;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxSeq(f.boxPar(f.boxWire(), f.boxWire()), f.boxAdd())

    f.compile_box("test", box)
    my_render(engine, f)


# def test5():

#     """
#     // Connection error
#     process = _ : +;
#     """

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")

#     box = f.boxSeq(f.boxWire(), f.boxMul())

#     f.compile_box("test", box)


def test6():

    """
    process = @(_,7);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = f.boxDelay(f.boxWire(), f.boxInt(7))

    f.compile_box("test", box)
    my_render(engine, f)


# def test7():

#     """
#     process = @(_,7);
#     """

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")
    
#     box = f.boxDelay(f.boxWire(), f.boxInt(7))
     
#     argv = ["-vec", "-lv", "1"]

#     f.compile_box("test", box, argv)


def test8():

    """
    process = _ <: @(500) + 0.5, @(3000) * 1.5;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = f.boxSplit(f.boxWire(), f.boxPar(f.boxAdd(f.boxDelay(f.boxWire(), f.boxReal(500)), f.boxReal(0.5)),
                                             f.boxMul(f.boxDelay(f.boxWire(), f.boxReal(3000)), f.boxReal(1.5))))

    f.compile_box("test", box)
    my_render(engine, f)


def test_equivalent1():

    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    b1 = f.boxAdd(f.boxDelay(f.boxWire(), f.boxReal(500)), f.boxReal(0.5))
    box = f.boxPar(b1, b1)

    f.compile_box("test", box)
    my_render(engine, f)


def test_equivalent2():

    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = f.boxPar(f.boxAdd(f.boxDelay(f.boxWire(), f.boxReal(500)), f.boxReal(0.5)),
                         f.boxAdd(f.boxDelay(f.boxWire(), f.boxReal(500)), f.boxReal(0.5)))

    f.compile_box("test", box)
    my_render(engine, f)


def test9():

    """
    process = _,hslider("Freq [midi:ctrl 7][style:knob]", 100, 100, 2000, 1) : *;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = f.boxMul(f.boxWire(),
        f.boxHSlider("Freq [midi:ctrl 7][style:knob]", f.boxReal(100), f.boxReal(100), f.boxReal(2000), f.boxReal(1)))

    f.compile_box("test", box)
    my_render(engine, f)


def test10():

    """
    import("stdfaust.lib");

    freq = vslider("h:Oscillator/freq", 440, 50, 1000, 0.1);
    gain = vslider("h:Oscillator/gain", 0, 0, 1, 0.01);

    process = freq * gain;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = f.boxMul(f.boxVSlider("h:Oscillator/freq", f.boxReal(440), f.boxReal(50), f.boxReal(1000), f.boxReal(0.1)),
                         f.boxVSlider("h:Oscillator/gain", f.boxReal(0), f.boxReal(0), f.boxReal(1), f.boxReal(0.01)))

    f.compile_box("test", box)
    my_render(engine, f)


def test11():

    """
    import("stdfaust.lib");
    process = ma.SR, ma.BS;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = f.boxPar(f.boxSampleRate(), f.boxBufferSize());

    f.compile_box("test", box)
    my_render(engine, f)


def test12():
    """
    process = waveform { 0, 100, 200, 300, 400 };
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxWaveform([i*100. for i in range(5)])

    f.compile_box("test", box)
    my_render(engine, f)


def test13():
    """
    process = _ <: +;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxSplit(f.boxWire(), f.boxAdd())

    f.compile_box("test", box)
    my_render(engine, f)


def test14():
    """
    process = _,_ <: !,_,_,! :> _,_;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxSplit(f.boxPar(f.boxWire(), f.boxWire()),
                           f.boxMerge(f.boxPar4(f.boxCut(), f.boxWire(), f.boxWire(), f.boxCut()),
                                    f.boxPar(f.boxWire(), f.boxWire())))

    f.compile_box("test", box)
    my_render(engine, f)


def test15():
    """
    process = + ~ _;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxRec(f.boxAdd(), f.boxWire())

    f.compile_box("test", box)
    my_render(engine, f)


"""
import("stdfaust.lib");
process = phasor(440)
with {
    decimalpart = _,int(_) : -;
    phasor(f) = f/ma.SR : (+ <: decimalpart) ~ _;
};
"""

def decimalpart(f):

    return f.boxSub(f.boxWire(), f.boxIntCast(f.boxWire()))

def phasor(f, freq):

    return f.boxSeq(f.boxDiv(freq, f.boxSampleRate()), f.boxRec(f.boxSplit(f.boxAdd(), decimalpart(f)), f.boxWire()))


def test16():
    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = phasor(f, f.boxReal(440))

    f.compile_box("test", box)
    my_render(engine, f)


def osc(f, freq):
    return f.boxSin(f.boxMul(f.boxMul(f.boxReal(2.0), f.boxReal(3.141592653)), phasor(f, freq)))


def test17():
    """
    import("stdfaust.lib");
    process = osc(440), osc(440)
    with {
        decimalpart(x) = x-int(x);
        phasor(f) = f/ma.SR : (+ : decimalpart) ~ _;
        osc(f) = sin(2 * ma.PI * phasor(f));
    };
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxPar(osc(f, f.boxReal(440)), osc(f, f.boxReal(440)))

    f.compile_box("test", box)
    my_render(engine, f)


def test18():
    """
    process = 0,0 : soundfile("sound[url:{'tango.wav'}]", 2);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxSoundfile("sound[url:{'tango.wav'}]", f.boxInt(2), f.boxInt(0), f.boxInt(0))

    f.compile_box("test", box)
    my_render(engine, f)


def test19():
    """
    process = 10,1,int(_) : rdtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxReadOnlyTable(f.boxInt(10), f.boxInt(1), f.boxIntCast(f.boxWire()))

    f.compile_box("test", box)
    my_render(engine, f)


def test20():
    """
    process = 10,1,int(_),int(_),int(_) : rwtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxWriteReadTable(f.boxInt(10), f.boxInt(1), f.boxIntCast(f.boxWire()), f.boxIntCast(f.boxWire()), f.boxIntCast(f.boxWire()))

    f.compile_box("test", box)
    my_render(engine, f)


def test24():
    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    # Follow the freq/gate/gain convention, see: https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters
    freq = f.boxNumEntry("freq", f.boxReal(100), f.boxReal(100), f.boxReal(3000), f.boxReal(0.01))
    gate = f.boxButton("gate")
    gain = f.boxNumEntry("gain", f.boxReal(0.5), f.boxReal(0), f.boxReal(1), f.boxReal(0.01))
    organ = f.boxMul(gate, f.boxAdd(f.boxMul(osc(f, freq), gain), f.boxMul(osc(f, f.boxMul(freq, f.boxInt(2))), gain)))
    organ *= 0.2
    # Stereo
    box = f.boxPar(organ, organ)

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'
    f.load_midi(abspath(ASSETS / midi_basename))

    f.num_voices = 10
    f.compile_box("test", box)

    my_render(engine, f)
    audio = engine.get_audio().T
    assert np.mean(np.abs(audio)) > .01


def test_overload_add1():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = f.boxWire()
    box = (in1 + f.boxReal(0.5)) + (in1+ f.boxInt(1))

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_add2():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = f.boxWire()
    box = (in1 + 0.5) + (in1 + 1)

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_add3():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = f.boxWire()
    box = sum([in1, f.boxReal(0.5)]) + sum([in1, f.boxInt(1)])

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_sub1():
    """
    //
    process(x) = (x - .5) + (x - 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = f.boxWire()
    box = (in1 - f.boxReal(0.5)) + (in1 - f.boxInt(1))

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_sub2():
    """
    //
    process(x) = (x - .5) + (x - 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = f.boxWire()
    box = (in1 - 0.5) + (in1 - 1)

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_mul1():
    """
    //
    process = _ * 0.5
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxWire() * f.boxReal(0.5)

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_mul2():
    """
    //
    process = _ * 0.5
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxWire() * 0.5

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_mul3():
    """
    //
    process = _ * 2
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxWire() * 2

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_mul3():
    """
    //
    process = _ * 2
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxWire() * f.boxInt(2)

    f.compile_box("test", box)
    my_render(engine, f)


if __name__ == "__main__":
    test_overload_add2()
