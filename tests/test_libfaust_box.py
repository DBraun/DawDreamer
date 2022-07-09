# adapted from:
# https://github.com/grame-cncm/faust/blob/master-dev/tools/benchmark/box-tester.cpp

from dawdreamer_utils import *
from dawdreamer.faust import *
from box_instruments import *

from typing import List
import inspect

from scipy import signal
import numpy as np
from enum import Enum
import warnings

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

    box = boxPar(boxInt(7), boxReal(3.14))
    f.compile_box("test", box)
    my_render(engine, f)


def test2():

    """
    process = _,3.14 : +;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("faust")

    box = boxSeq(boxPar(boxWire(), boxReal(3.14)), boxAdd())

    f.compile_box("test", box)
    my_render(engine, f)


def test3():

    """
    // Alternate version with the binary 'boxAdd' version
    process = +(_,3.14);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxAdd(boxWire(), boxReal(3.14))

    f.compile_box("test", box)
    my_render(engine, f)


def test4():

    """
    process = _,_ : +;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxSeq(boxPar(boxWire(), boxWire()), boxAdd())

    f.compile_box("test", box)
    my_render(engine, f)


# def test5():

#     """
#     // Connection error
#     process = _ : +;
#     """

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")

#     box = boxSeq(boxWire(), boxMul())

#     f.compile_box("test", box)


def test6():

    """
    process = @(_,7);
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = boxDelay(boxWire(), boxInt(7))

    f.compile_box("test", box)
    my_render(engine, f)


# def test7():

#     """
#     process = @(_,7);
#     """

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")
    
#     box = boxDelay(boxWire(), boxInt(7))
     
#     argv = ["-vec", "-lv", "1"]

#     f.compile_box("test", box, argv)


def test8():

    """
    process = _ <: @(500) + 0.5, @(3000) * 1.5;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = boxSplit(boxWire(), boxPar(boxAdd(boxDelay(boxWire(), boxReal(500)), boxReal(0.5)),
                                             boxMul(boxDelay(boxWire(), boxReal(3000)), boxReal(1.5))))

    f.compile_box("test", box)
    my_render(engine, f)


def test_equivalent1():

    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    b1 = boxAdd(boxDelay(boxWire(), boxReal(500)), boxReal(0.5))
    box = boxPar(b1, b1)

    f.compile_box("test", box)
    my_render(engine, f)


def test_equivalent2():

    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = boxPar(boxAdd(boxDelay(boxWire(), boxReal(500)), boxReal(0.5)),
                         boxAdd(boxDelay(boxWire(), boxReal(500)), boxReal(0.5)))

    f.compile_box("test", box)
    my_render(engine, f)


def test9():

    """
    process = _,hslider("Freq [midi:ctrl 7][style:knob]", 100, 100, 2000, 1) : *;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = boxMul(boxWire(),
        boxHSlider("Freq [midi:ctrl 7][style:knob]", boxReal(100), boxReal(100), boxReal(2000), boxReal(1)))

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
    
    box = boxMul(boxVSlider("h:Oscillator/freq", boxReal(440), boxReal(50), boxReal(1000), boxReal(0.1)),
                         boxVSlider("h:Oscillator/gain", boxReal(0), boxReal(0), boxReal(1), boxReal(0.01)))

    f.compile_box("test", box)
    my_render(engine, f)


def test11():

    """
    import("stdfaust.lib");
    process = ma.SR, ma.BS;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")
    
    box = boxPar(boxSampleRate(), boxBufferSize());

    f.compile_box("test", box)
    my_render(engine, f)


def test12():
    """
    process = waveform { 0, 100, 200, 300, 400 };
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxWaveform([i*100. for i in range(5)])

    f.compile_box("test", box)
    my_render(engine, f)


def test13():
    """
    process = _ <: +;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxSplit(boxWire(), boxAdd())

    f.compile_box("test", box)
    my_render(engine, f)


def test14():
    """
    process = _,_ <: !,_,_,! :> _,_;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxSplit(boxPar(boxWire(), boxWire()),
                           boxMerge(boxPar4(boxCut(), boxWire(), boxWire(), boxCut()),
                                    boxPar(boxWire(), boxWire())))

    f.compile_box("test", box)
    my_render(engine, f)


def test15():
    """
    process = + ~ _;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxRec(boxAdd(), boxWire())

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

def decimalpart(f) -> Box:

    return boxSub(boxWire(), boxIntCast(boxWire()))

def phasor(f, freq: Box) -> Box:

    return boxSeq(boxDiv(freq, boxSampleRate()), boxRec(boxSplit(boxAdd(), decimalpart(f)), boxWire()))


def test16():
    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = phasor(f, boxReal(440))

    f.compile_box("test", box)
    my_render(engine, f)


def osc(f, freq: Box) -> Box:
    return boxSin(boxMul(boxMul(boxReal(2.0), boxReal(3.141592653)), phasor(f, freq)))


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

    box = boxPar(osc(f, boxReal(440)), osc(f, boxReal(440)))

    f.compile_box("test", box)
    my_render(engine, f)


# skip soundfile
# def test18():
#     """
#     process = 0,0 : soundfile("sound[url:{'tango.wav'}]", 2);
#     """

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")

#     box = boxSoundfile("sound[url:{'tango.wav'}]", boxInt(2), boxInt(0), boxInt(0))

#     f.compile_box("test", box)
#     my_render(engine, f)


def test19():
    """
    process = 10,1,int(_) : rdtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxReadOnlyTable(boxInt(10), boxInt(1), boxIntCast(boxWire()))

    f.compile_box("test", box)
    my_render(engine, f)


def test19b():
    """
    process = 10,1,int(_) : rdtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    waveform_content = boxSeq(boxWaveform([-1., 0., 1., 0.]), boxPar(boxCut(), boxWire()))

    box = boxReadOnlyTable(boxInt(4), waveform_content, boxWire())

    f.boxToCPP(box)


def test20():
    """
    process = 10,1,int(_),int(_),int(_) : rwtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxWriteReadTable(boxInt(10), boxInt(1), boxIntCast(boxWire()), boxIntCast(boxWire()), boxIntCast(boxWire()))

    f.compile_box("test", box)
    my_render(engine, f)


def test24():
    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    # Follow the freq/gate/gain convention, see: https://faustdoc.grame.fr/manual/midi/#standard-polyphony-parameters
    freq = boxNumEntry("freq", boxReal(100), boxReal(100), boxReal(3000), boxReal(0.01))
    gate = boxButton("gate")
    gain = boxNumEntry("gain", boxReal(0.5), boxReal(0), boxReal(1), boxReal(0.01))
    organ = boxMul(gate, boxAdd(boxMul(osc(f, freq), gain), boxMul(osc(f, boxMul(freq, boxInt(2))), gain)))
    organ *= 0.2
    # Stereo
    box = boxPar(organ, organ)

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'
    f.load_midi(abspath(ASSETS / midi_basename))

    f.num_voices = 10
    f.compile_box("test", box)

    my_render(engine, f)
    audio = engine.get_audio().T
    assert np.mean(np.abs(audio)) > .01


def test25a():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    filter, inputs, outputs = f.dsp_to_box('process = si.smooth;');
    assert inputs == 2
    assert outputs == 1
    cutoffAndInput, inputs, outputs = f.dsp_to_box('process = hslider("cutoff", 300, 100, 2000, .01), _;')
    assert inputs == 1
    assert outputs == 2

    filteredInput = boxSeq(cutoffAndInput, filter)
    f.compile_box("test", filteredInput)
    my_render(engine, f)


def test25b():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    filter, inputs, outputs = f.dsp_to_box('process = si.smooth;');
    cutoffAndInput, inputs, outputs = f.dsp_to_box('process = hslider("cutoff", 300, 100, 2000, .01), _;')

    filteredInput = boxSeq(cutoffAndInput, filter)
    f.compile_box("test", filteredInput)
    my_render(engine, f)


def test25c():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    def test_bus(n):
        box, inputs, outputs = f.dsp_to_box(f'process = si.bus({n});');
        print(f'exepcted {n}, and got inputs: {inputs}, outputs: {outputs}')
        assert inputs ==  n and outputs == n

    # test ascending and then descending.
    test_bus(3)
    test_bus(4)
    test_bus(2)


def test26():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box1 = boxWire()

    box2 = boxMul(box1, boxReal(0.5))
    box3 = boxRem(box1, boxReal(0.8))

    box4 = boxSeq(box2, box1)
    box5 = boxSeq(box3, box1)

    box6 = boxPar3(box4, box5, box3)


def test27():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box4 = boxFromDSP('process = os.osc;')
    box5 = boxFromDSP('process = en.adsr;')
    box6 = boxFromDSP('process = en.adsre;')
    box7 = boxFromDSP('process = fi.lowpass(5);')

def test28(num_voices=12, dynamic_voices=True):

    cfg = {

        "OSC_A_TOGGLE": True,
        "OSC_A_CHOICE": OscChoice.SAWTOOTH,
        "OSC_A_UNISON": 7,

        "OSC_B_TOGGLE": False,
        "OSC_B_CHOICE": OscChoice.SAWTOOTH,
        "OSC_B_UNISON": 1,

        "SUB_TOGGLE": True,
        "SUB_CHOICE": OscChoice.SINE,

        "NOISE_TOGGLE": False,

        "FILTER_TOGGLE": True,
        "FILTER_CHOICE": FilterChoice.LOWPASS_12,
        "FILTER_OSC_A": True,
        "FILTER_OSC_B": True,
        "FILTER_NOISE": True,
        "FILTER_SUB": True,

        "MODULATION_MATRIX": [
            # source should be macro, gain, gate, freq
            ("macro1", "env1_A", 0.1, False),
            ("macro2", "oscA_gain", 0, False),
            # ("gain", "oscA_gain", .1, False),
            # ("gain", "oscB_gain", .1, False),
            # ("gain", "oscA_detune_amt", 1., False),


            # source must be env or LFO
            # todo: what if env is connected to LFO or vice versa?
            ("env1", "oscA_gain", .5, False),
            ("env1", "sub_gain", .2, False),
            ("env2", "filter_cutoff", 10_000, False),
            # ("env2", "oscA_freq", 12., False),  # semitone units
            # ("lfo1", "oscA_freq", 12, True),
            # ("lfo1", "oscB_gain", .5, False),
        ],

        "EFFECTS": [
            'delay'
        ],
    }

    parameter_settings = [
        ('Env_1/Attack', 3),
        ('Env_1/Hold', 0),
        ('Env_1/Decay', 1000),
        ('Env_1/Sustain', 0.8),
        ('Env_1/Release', 200),

        ('Env_2/Attack', 20),
        ('Env_2/Hold', 0),
        ('Env_2/Decay', 125),
        ('Env_2/Sustain', 0),
        ('Env_2/Release', 200),

        ('Filter/Cutoff', 100),

        ('Sub/Freq', -12),
        # ('LFO_1/Freq', 20),
    ]

    #############

    engine = daw.RenderEngine(SAMPLE_RATE, 512)
    f = engine.make_faust_processor("my_faust")

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'
    f.load_midi(abspath(ASSETS / midi_basename))

    f.num_voices = num_voices
    f.dynamic_voices = dynamic_voices

    modular_synth = ModularSynth()
    box = modular_synth.build(cfg)

    # f.boxToCPP(box)

    f.compile_box("test", box)

    desc = f.get_parameters_description()
    for parameter in desc:
        print(parameter)

    # todo: figure out why this prefix is dummy
    prefix = '/Polyphonic/Voices/dummy/'
    for parname, val in parameter_settings:
        try:
            f.set_parameter(prefix + parname, val)
        except:
            warnings.warn("Unable to find parameter named: " + str(prefix+parname))

    my_render(engine, f)
    audio = engine.get_audio().T
    assert np.mean(np.abs(audio)) > .001


def test29():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box, inputs, outputs = f.dsp_to_box(f"""process = en.ahdsre(.1,.1,.1,.1);""")
    assert inputs == 2
    assert outputs == 1


def test30():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxFromDSP(f"""process = en.ahdsre(.1,.1,.1,.1);""")
    f.boxToCPP(box)


def test_overload_add1():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = boxWire()
    box = (in1 + boxReal(0.5)) + (in1+ boxInt(1))

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_add2():
    """
    //
    process(x) = (x + .5) + (x + 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = boxWire()
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

    in1 = boxWire()
    box = sum([in1, boxReal(0.5)]) + sum([in1, boxInt(1)])

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_sub1():
    """
    //
    process(x) = (x - .5) + (x - 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = boxWire()
    box = (in1 - boxReal(0.5)) + (in1 - boxInt(1))

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_sub2():
    """
    //
    process(x) = (x - .5) + (x - 1)
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    in1 = boxWire()
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

    box = boxWire() * boxReal(0.5)

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_mul2():
    """
    //
    process = _ * 0.5
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxWire() * 0.5

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_mul3():
    """
    //
    process = _ * 2
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxWire() * 2

    f.compile_box("test", box)
    my_render(engine, f)


def test_overload_mul3():
    """
    //
    process = _ * 2
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = boxWire() * boxInt(2)

    f.compile_box("test", box)
    my_render(engine, f)


if __name__ == "__main__":
    test_overload_add2()
