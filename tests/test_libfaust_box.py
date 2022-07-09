# adapted from:
# https://github.com/grame-cncm/faust/blob/master-dev/tools/benchmark/box-tester.cpp

from dawdreamer_utils import *
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

def decimalpart(f) -> daw.Box:

    return f.boxSub(f.boxWire(), f.boxIntCast(f.boxWire()))

def phasor(f, freq: daw.Box) -> daw.Box:

    return f.boxSeq(f.boxDiv(freq, f.boxSampleRate()), f.boxRec(f.boxSplit(f.boxAdd(), decimalpart(f)), f.boxWire()))


def test16():
    """
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = phasor(f, f.boxReal(440))

    f.compile_box("test", box)
    my_render(engine, f)


def osc(f, freq: daw.Box) -> daw.Box:
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


# skip soundfile
# def test18():
#     """
#     process = 0,0 : soundfile("sound[url:{'tango.wav'}]", 2);
#     """

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")

#     box = f.boxSoundfile("sound[url:{'tango.wav'}]", f.boxInt(2), f.boxInt(0), f.boxInt(0))

#     f.compile_box("test", box)
#     my_render(engine, f)


def test19():
    """
    process = 10,1,int(_) : rdtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box = f.boxReadOnlyTable(f.boxInt(10), f.boxInt(1), f.boxIntCast(f.boxWire()))

    f.compile_box("test", box)
    my_render(engine, f)


def test19b():
    """
    process = 10,1,int(_) : rdtable;
    """

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    waveform_content = f.boxSeq(f.boxWaveform([-1., 0., 1., 0.]), f.boxPar(f.boxCut(), f.boxWire()))

    box = f.boxReadOnlyTable(f.boxInt(4), waveform_content, f.boxWire())

    f.boxToCPP(box)


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


def test25a():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    filter, inputs, outputs = f.dsp_to_box('process = si.smooth;');
    assert inputs == 2
    assert outputs == 1
    cutoffAndInput, inputs, outputs = f.dsp_to_box('process = hslider("cutoff", 300, 100, 2000, .01), _;')
    assert inputs == 1
    assert outputs == 2

    filteredInput = f.boxSeq(cutoffAndInput, filter)
    f.compile_box("test", filteredInput)
    my_render(engine, f)


def test25b():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    filter, inputs, outputs = f.dsp_to_box('process = si.smooth;');
    cutoffAndInput, inputs, outputs = f.dsp_to_box('process = hslider("cutoff", 300, 100, 2000, .01), _;')

    filteredInput = f.boxSeq(cutoffAndInput, filter)
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

    box1 = f.boxWire()

    box2 = f.boxMul(box1, f.boxReal(0.5))
    box3 = f.boxRem(box1, f.boxReal(0.8))

    box4 = f.boxSeq(box2, box1)
    box5 = f.boxSeq(box3, box1)

    box6 = f.boxPar3(box4, box5, box3)


def test27():

    engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
    f = engine.make_faust_processor("my_faust")

    box4 = f.boxFromDSP('process = os.osc;')
    box5 = f.boxFromDSP('process = en.adsr;')
    box6 = f.boxFromDSP('process = en.adsre;')
    box7 = f.boxFromDSP('process = fi.lowpass(5);')

def test28():

    """Serum-like synthesizer"""

    # <enums>
    class FilterChoice(Enum):
        LOWPASS_12   = 1
        LOWPASS_24   = 2
        HIGHPASS_12 = 3
        HIGHPASS_24 = 4
        LOWSHELF_12  = 5
        HIGHSHELF_12 = 6

    class OscChoice(Enum):
        SAWTOOTH    = 1
        SINE        = 2
        TRIANGLE    = 3
        SQUARE      = 4
        WHITE_NOISE = 5
    # </enums>

    # <compile time constants>

    OSC_A_TOGGLE = True
    OSC_A_CHOICE = OscChoice.SAWTOOTH
    OSC_A_UNISON = 7

    OSC_B_TOGGLE = False
    OSC_B_CHOICE = OscChoice.SAWTOOTH
    OSC_B_UNISON = 1

    SUB_TOGGLE = True
    SUB_CHOICE = OscChoice.SINE

    NOISE_TOGGLE = False

    FILTER_TOGGLE = True
    FILTER_CHOICE = FilterChoice.LOWPASS_12
    FILTER_OSC_A = True
    FILTER_OSC_B = True
    FILTER_NOISE = True
    FILTER_SUB = True

    NUM_MACROS = 4
    NUM_ENVS = 4
    NUM_LFOS = 4
    NUM_VOICES = 12

    TABLE_SIZE = 16_384
    BLOCK_SIZE = 512  # only really matters for automation
    LAGRANGE_ORDER = 2  # for quality of anti-aliasing oscillators

    settings = [
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

    MACRO_MODULATIONS = [
        # source should be macro, gain, gate, freq
        ("macro1", "env1_A", 0, False),
        ("macro2", "oscA_gain", 0, False),
        # ("gain", "oscA_gain", .1, False),
        # ("gain", "oscB_gain", .1, False),
        # ("gain", "oscA_detune_amt", 1., False),
    ]

    OTHER_MODULATIONS = [
        # source must be env or LFO
        # todo: what if env is connected to LFO or vice versa?
        ("env1", "oscA_gain", .5, False),
        ("env1", "sub_gain", .2, False),
        ("env2", "filter_cutoff", 10_000, False),
        # ("env2", "oscA_freq", 12., False),  # semitone units
        # ("lfo1", "oscA_freq", 12, True),
        # ("lfo1", "oscB_gain", .5, False),
    ]

    EFFECTS_MODULATIONS = [

    ]

    # ordered list of post-fx to use
    EFFECTS = [
        'delay'
    ]

    # </compile time constants>

    ##### No need to modify below. #####

    engine = daw.RenderEngine(SAMPLE_RATE, BLOCK_SIZE)
    f = engine.make_faust_processor("my_faust")

    def boxWire() -> daw.Box:
        return f.boxWire()

    def boxCut() -> daw.Box:
        return f.boxCut()

    def boxReal(val: float) -> daw.Box:
        return f.boxReal(val)

    def boxInt(val: int) -> daw.Box:
        return f.boxInt(val)

    def boxPow(box1: daw.Box, box2: daw.Box) -> daw.Box:
        return f.boxPow(box1, box2)

    def boxSeq(box1: daw.Box, box2: daw.Box) -> daw.Box:
        return f.boxSeq(box1, box2)

    def boxSplit(box1: daw.Box, box2: daw.Box) -> daw.Box:
        return f.boxSplit(box1, box2)

    def boxMerge(box1: daw.Box, box2: daw.Box) -> daw.Box:
        return f.boxMerge(box1, box2)

    def boxPar(box1: daw.Box, box2: daw.Box) -> daw.Box:
        return f.boxPar(box1, box2)

    def boxPar3(box1: daw.Box, box2: daw.Box, box3: daw.Box) -> daw.Box:
        return f.boxPar3(box1, box2, box3)

    def boxPar4(box1: daw.Box, box2: daw.Box, box3: daw.Box, box4: daw.Box) -> daw.Box:
        return f.boxPar4(box1, box2, box3, box4)

    def boxPar5(box1: daw.Box, box2: daw.Box, box3: daw.Box, box4: daw.Box, box5: daw.Box) -> daw.Box:
        return f.boxPar5(box1, box2, box3, box4, box5)

    def boxHSlider(label: str, default: float, minVal: float, maxVal: float, step: float) -> daw.Box:
        return f.boxHSlider(label, boxReal(default), boxReal(minVal), boxReal(maxVal), boxReal(step))

    def boxButton(label: str):
        return f.boxButton(label)

    ### convenience functions like faust libraries:

    def semiToRatio(box: daw.Box) -> daw.Box:
        return boxSeq(box, boxPow(boxReal(2.), boxWire()/12.))

    def boxParN(boxes: List[daw.Box]):
        N = len(boxes)
        assert N > 0
        box = boxes.pop(0)
        while boxes:
            box = boxPar(box, boxes.pop(0))

        return box

    def bus(n: int) -> daw.Box:
        if n == 0:
            raise ValueError("Can't make a bus of size zero.")
        else:
            return boxParN([boxWire() for _ in range(n)])

    def parallel_add(box1: daw.Box, box2: daw.Box) -> daw.Box:
        return boxSeq(boxPar(box1, box2), boxMerge(bus(4), bus(2)))

    #################################################

    MODS = {
        'freq': boxHSlider("freq", 100, 100, 20000, .001),
        'gate': boxButton("gate"),
        'gain': boxHSlider("gain", .5, 0, 1, .001)
    }

    def process_modulations(modulations):
        for source, dst, amt, symmetric in modulations:

            if source not in MODS:
                warnings.warn(f"""warning: source "{source}" was modulated but isn't used for DSP.""")
                continue
            if dst not in MODS:
                warnings.warn(f"""warning: destination "{dst}" was modulated but isn't used for DSP.""")
                continue

            # note that there's a bug preventing us from using +=.
            if symmetric:
                MODS[dst] = MODS[dst] + (MODS[source]-.5)*amt*2.
            else:
                MODS[dst] = MODS[dst] + MODS[source]*amt


    def make_macro(i: int):

        MODS[f'macro{i}'] = boxHSlider(f"h:Macro/Macro {i}", 0, 0, 1, .001)


    def make_env(i: int):

        # time units are milliseconds
        MODS[f'env{i}_A'] = (boxWire() + boxHSlider(f"h:Env {i}/[0]Attack", 5, 0., 10_000, .001)) / 1_000
        MODS[f'env{i}_H'] = (boxWire() + boxHSlider(f"h:Env {i}/[1]Hold", 0, 0., 10_000, .001)) / 1_000
        MODS[f'env{i}_D'] = (boxWire() + boxHSlider(f"h:Env {i}/[2]Decay", 20, 0., 10_000, .001)) / 1_000
        MODS[f'env{i}_S'] = (boxWire() + boxHSlider(f"h:Env {i}/[3]Sustain", 0.5, 0., 10_000, .001))
        MODS[f'env{i}_R'] = (boxWire() + boxHSlider(f"h:Env {i}/[4]Release", 200, 0., 10_000, .001)) / 1_000
        MODS[f'env{i}'] = f.boxFromDSP(f"process = en.ahdsre;")


    def make_lfo(i: int):

        MODS[f'lfo{i}_gain'] = boxWire() + boxHSlider(f"h:LFO {i}/[0]Gain", 1, 0, 10, .001)
        MODS[f'lfo{i}_freq'] = boxWire() + boxHSlider(f"h:LFO {i}/[1]Freq", 2, 0, 10, .001)

        MODS[f'lfo{i}'] = f.boxFromDSP(f"""process(gain, freq, gate) = gain * os.osc(freq);""")

    def get_wavecycle_data(choice):

        if choice == OscChoice.SINE:
            # sine wave
            wavecycle_data = np.sin(np.pi*2*np.linspace(0, 1, TABLE_SIZE, endpoint=False))
        elif choice == OscChoice.SAWTOOTH:
            # sawtooth
            t = np.linspace(0, 1, TABLE_SIZE, endpoint=False)
            t = np.concatenate([t[TABLE_SIZE//2:], t[:TABLE_SIZE//2]])
            assert t.shape[0] == TABLE_SIZE
            wavecycle_data = -1. + 2.*np.linspace(0, 1, TABLE_SIZE, endpoint=False)
        elif choice == OscChoice.TRIANGLE:
            # triangle
            t = np.linspace(0, 1, TABLE_SIZE, endpoint=False)
            t = np.concatenate([t[TABLE_SIZE//4:], t[:TABLE_SIZE//4]])
            assert t.shape[0] == TABLE_SIZE
            wavecycle_data = signal.sawtooth(2 * np.pi * t, 0.5)
        elif choicoe == OscChoice.SQUARE:
            t = np.linspace(0, 1, TABLE_SIZE, endpoint=False)
            wavecycle_data = -1.+2.*(t > .5).astype(np.float32)
        elif choice == OscChoice.WHITE_NOISE:
            wavecycle_data = -1.+2.*np.random.rand(TABLE_SIZE)
        else:
            raise ValueError(f"Unexpected oscillator choice: {choice}.")

        return wavecycle_data.tolist()


    def make_noise():

        MODS['noise_gain'] = boxWire() + boxHSlider(f"h:Noise/[0]Gain", 0, 0, 10., .001)

        wavecycle_data = get_wavecycle_data(OscChoice.WHITE_NOISE)

        waveform_content = boxSeq(f.boxWaveform(wavecycle_data), boxPar(boxCut(), boxWire()))

        readTable = boxWire() * f.boxReadOnlyTable(boxInt(TABLE_SIZE), waveform_content, phasor(f, TABLE_SIZE/f.boxSampleRate())*TABLE_SIZE)

        MODS['noise'] = boxSplit(readTable, bus(2)) # split to stereo


    def make_sub(choice):

        MODS[f'sub_gain']       = boxWire()             + boxHSlider(f"h:Sub/[0]Gain", 0, 0, 10, .001)
        MODS[f'sub_freq']       = semiToRatio(boxWire() + boxHSlider(f"h:Sub/[1]Freq", 0, -72, 72, .001)) * MODS['freq']
        MODS[f'sub_pan']        = boxWire()             + boxHSlider(f"h:Sub/[2]Pan", 0, 0, 10, .001)

        wavecycle_data = get_wavecycle_data(choice)

        waveform_content = boxSeq(f.boxWaveform(wavecycle_data), boxPar(boxCut(), boxWire()))

        readTable = boxWire() * f.boxReadOnlyTable(boxInt(TABLE_SIZE), waveform_content, phasor(f, boxWire() + boxWire())*TABLE_SIZE)

        MODS[f'sub'] = boxSplit(readTable, bus(2)) # split to stereo


    def make_osc(x: str, choice, unison: int):

        MODS[f'osc{x}_gain']       = boxWire() + boxHSlider(f"h:Osc {x}/[0]Gain", 0, 0, 10, .001)
        MODS[f'osc{x}_freq']       = semiToRatio(boxWire() + boxHSlider(f"Osc {x} [1]Freq", 0, 0, 10, .001)) * MODS['freq']
        MODS[f'osc{x}_detune_amt'] = boxWire() + boxHSlider(f"h:Osc {x}/[2]Detune", 0.5, 0, 10, .001)
        MODS[f'osc{x}_blend']      = boxWire() + boxHSlider(f"h:Osc {x}/[3]Blend", 0.5, 0, 10, .001)
        MODS[f'osc{x}_pan']        = (boxWire() + boxHSlider(f"h:Osc {x}/[4]Pan", .5, 0, 1, .001))*2.-1
        MODS[f'osc{x}_wt_pos']     = boxWire() + boxHSlider(f"h:Osc {x}/[5]WT Pos", 0., 0, 1, .001)
        MODS[f'osc{x}_wt_bend']    = boxWire() + boxHSlider(f"h:Osc {x}/[6]WT Bend", 0., 0, 1, .001)
        MODS[f'osc{x}_phase']      = boxWire() + boxHSlider(f"h:Osc {x}/[7]Phase", 0, 0, 1, .001)
        MODS[f'osc{x}_rand']       = boxWire() + boxHSlider(f"h:Osc {x}/[8]Rand", 0, 0, 1, .001)
        MODS[f'osc{x}_stereo_width'] = boxWire() + boxHSlider(f"h:Osc {x}/[9]Stereo Width", 1, 0, 1, .001)

        MODS[f'osc{x}_waveform'] = f.boxWaveform(get_wavecycle_data(choice))

        dsp_code = f"""

        NUM_UNISON = {unison};
        LAGRANGE_ORDER = {LAGRANGE_ORDER};

        //-----------------------`unisonHelper`------------------------
        // A helper function for creating a detuned and panned unison voice.
        //
        // #### Usage
        //
        // ```
        // unisonHelper(nUnison, i, S, waveform_data, stereoWidth, detuneAmt, blendAmt, freq, pan, gate) : _, _
        // ```
        //
        // Where:
        //
        // * `nVoices`: int. Must be fixed at compile time and greater than 0
        // * `i`: int. The index of this unison voices among all of the voices
        // * `S`: int. The waveform size
        // * `waveform_data`: The waveform data
        // * `stereoWidth`: float [0-1]. Stereo width of all unison voices
        // * `detuneAmt`: float [0-1]. Detune in semitones. The most detuned voice will have this much detuning
        // * `blendAmt`: float [0-1]. The mixing between the detuned voices and the non-detuned voices. 1 means only detuned
        // * `freq`: Hertz to play
        // * `pan`: float [-1, 1]. Stereo panning where -1 is left and 1 is right.
        // * `gate`: float [0, 1]. Gate of note.
        //------------------------------------------------------------
        unisonHelper(nUnison, i, S, waveform_data, stereoWidth, detuneAmt, blendAmt, freq, pan, gate) = result          
        with {{

          unisonGt1 = nUnison > 1;        
          // symmetricVal is now [-1, 1]      
          symmetricVal = -1.+2.*float(i) / float(max(1, nUnison-1));      
                  
          numVoicesOdd = nUnison%2;       
                  
          cond = (numVoicesOdd & (i % 2)) | ((1-numVoicesOdd) & ((i >= nUnison/2) xor (i%2)));            
                  
          panOut = select2(unisonGt1, 0.5, select2(cond, 1., -1.) *symmetricVal*.5*stereoWidth+.5) + pan : max(0.) : min(1.);       
                  
          ratio = select2(unisonGt1, 1., ba.semi2ratio(symmetricVal * detuneAmt));        
                  
          // gain related things          
          sideVolume = blendAmt / select2(numVoicesOdd, float(max(nUnison-2, 1)), float(max(nUnison-1, 1)));      
          centerVolume = select2(numVoicesOdd, (1.-blendAmt)/ 2., 1.-blendAmt);       
                  
          centerIndex = int((nUnison)/2);     
                  
          gain = select2(nUnison > 2, 1./float(nUnison),      
                         select2( 
                                 select2(numVoicesOdd,    
                                         (i+1==centerIndex)|(i==centerIndex),
                                         i==((nUnison-1)/2)),
                                 sideVolume,  
                                 centerVolume)    
              );

          ridx = os.hs_phasor(S, freq*ratio, 0 );  // (gate : ba.impulsify)
          result = it.frdtable(LAGRANGE_ORDER, S, waveform_data, ridx) * gain <: sp.panner(panOut);
        }};

        process(waveform_N, waveform_data, gain, width, amount, blend, freq, pan, gate) = result         
        with {{
          result = par(i, NUM_UNISON, unisonHelper(NUM_UNISON, i, waveform_N, waveform_data, width, amount, blend, freq, pan, gate)) :> sp.stereoize( _ * gain);  
        }};
        """

        MODS[f'osc{x}'] = f.boxFromDSP(dsp_code)

    def make_filter(choice):

        MODS['filter_cutoff']    = boxWire()+boxHSlider(f"h:Filter/Cutoff", 5000., 20., 20000, .001)
        MODS['filter_gain']      = boxWire()+boxHSlider(f"h:Filter/Gain", 0., -80, 24, .001)
        MODS['filter_resonance'] = boxWire()+boxHSlider(f"h:Filter/Resonance", 1, 0, 2, .001)

        if choice == FilterChoice.LOWPASS_12:
            dsp = "process(cutoff, gain, res) = si.bus(2) : sp.stereoize(fi.lowpass(5, cutoff));"
        elif choice == FilterChoice.LOWPASS_24:
            dsp = "process(cutoff, gain, res) = si.bus(2) : sp.stereoize(fi.lowpass(15, cutoff));"
        elif choice == FilterChoice.HIGHPASS_12:
            dsp = "process(cutoff, gain, res) = si.bus(2) : sp.stereoize(fi.highpass(5, cutoff));"
        elif choice == FilterChoice.HIGHPASS_24:
            dsp = "process(cutoff, gain, res) = si.bus(2) : sp.stereoize(fi.highpass(15, cutoff));"
        elif choice == FilterChoice.LOWSHELF_12:
            dsp = "process(cutoff, gain, res) = si.bus(2) : sp.stereoize(fi.lowshelf(3, gain, cutoff));"
        elif choice == FilterChoice.HIGHSHELF_12:
            dsp = "process(cutoff, gain, res) = si.bus(2) : sp.stereoize(fi.highshelf(3, gain, cutoff));"
        else:
            raise ValueError(f"Unexpected filter choice: {choice}.")

        MODS['filter'] = f.boxFromDSP(dsp)


    def make_reverb():
        pass  # todo:
        MODS['reverb_cutoff']  = boxWire()+boxHSlider(f"h:Reverb/Filter Cutoff", 5000., 20., 20000, .001)
        MODS['reverb_size']    = boxWire()+boxHSlider(f"h:Reverb/Size", 0, 0, 1, .001)
        MODS['reverb_mix']     = boxWire()+boxHSlider(f"h:Reverb/Mix", 0, 0, 1, .001)

        MODS['reverb'] = boxMerge(bus(3), boxWire())


    def make_chorus():
        pass  # todo:


    def make_delay():

        MAXDELAY = 1.
        DELAYORDER = 5;

        MODS['delay_dtime']    = boxHSlider("h:Delay/Time", .125, 0., MAXDELAY, 0);
        MODS['delay_level']    = boxHSlider("h:Delay/Level", 1, 0, 1, 0)
        MODS['delay_feedback'] = boxHSlider("h:Delay/Feedback", 0.8, 0, 1, 0)
        MODS['delay_stereo']   = boxHSlider("h:Delay/Stereo", 1, 0, 1, 0)
        MODS['delay_wet']      = boxHSlider("h:Delay/Wet", .5, 0, 1, 0)

        dsp_code = f"""

        /* Stereo delay with feedback. */

        declare name "echo -- stereo delay effect";
        declare author "Albert Graef";
        declare version "1.0";
        import("stdfaust.lib");

        // revised by David Braun
        MAXDELAY = {MAXDELAY};
        DELAYORDER = {DELAYORDER};

        // do not change:
        MINDELAY = (DELAYORDER-1)/2+1;

        /* The stereo parameter controls the amount of stereo spread. For stereo=0 you
           get a plain delay, without crosstalk between the channels. For stereo=1 you
           get a pure ping-pong delay where the echos from the left first appear on
           the right channel and vice versa. Note that you'll hear the stereo effects
           only if the input signal already has some stereo spread to begin with; if
           necessary, you can just pan the input signal to the left or the right to
           achieve that. */
           
        echo(dtime,level,feedback,stereo,x,y)
                = f(x,y) // the echo loop
                // mix
                : (\\(u,v).(x+level*(d(u)+c(v)),
                       y+level*(d(v)+c(u))))
                // compensate for gain level
                : (/(1+level), /(1+level))
        with {{
            f   = g ~ (*(feedback),*(feedback));
            g(u,v,x,y)
                = h(x+d(u)+c(v)), h(y+d(v)+c(u));
                dtimeSafe = max(MINDELAY, ma.SR*dtime);
            h   = de.fdelayltv(DELAYORDER, ma.SR*MAXDELAY, dtimeSafe);
            c(x)    = x*stereo;
            d(x)    = x*(1-stereo);
        }};

        wet_dry_mixer(wet_amt, fx, x, y) = result
        with {{
          wet = x, y : fx :> _, _;
          dry_amt = 1.-wet_amt;
          result = (wet : sp.stereoize(_*wet_amt)), (x, y : sp.stereoize(_*dry_amt)) :> _, _;
        }};

        process(dtime, level, feedback, stereo, wet) = si.bus(2) : wet_dry_mixer(wet, echo(dtime, level, feedback, stereo)); 

        """

        MODS['delay'] = f.boxFromDSP(dsp_code)


    for i in range(NUM_MACROS):
        make_macro(i+1)

    for i in range(NUM_ENVS):
        make_env(i+1)

    for i in range(NUM_LFOS):
        make_lfo(i+1)

    if OSC_A_TOGGLE:
        make_osc('A', OSC_A_CHOICE, OSC_A_UNISON)

    if OSC_B_TOGGLE:
        make_osc('B', OSC_B_CHOICE, OSC_B_UNISON)

    if SUB_TOGGLE:
        make_sub(SUB_CHOICE)

    if FILTER_TOGGLE:
        make_filter(FILTER_CHOICE)

    if NOISE_TOGGLE:
        make_noise()

    if 'reverb' in EFFECTS:
        make_reverb()

    if 'chorus' in EFFECTS:
        make_chorus()

    if 'delay' in EFFECTS:
        make_delay()

    # modulate the destinations we just made.
    process_modulations(MACRO_MODULATIONS)

    # cook the envelopes
    for i in range(1, NUM_ENVS+1):
        MODS[f'env{i}'] = boxSeq(
            boxParN([MODS[f'env{i}_A'],MODS[f'env{i}_H'], MODS[f'env{i}_D'], 
                MODS[f'env{i}_S'], MODS[f'env{i}_R'], MODS['gate']]),
            MODS[f'env{i}']
            )

    # cook the LFOs
    for i in range(1, NUM_LFOS+1):
        MODS[f'lfo{i}'] = boxSeq(
            boxPar3(MODS[f'lfo{i}_gain'], MODS[f'lfo{i}_freq'], MODS['gate']),
            MODS[f'lfo{i}'])

    # modulate the destinations that have envelopes and LFOs as a source.
    process_modulations(OTHER_MODULATIONS)
        
    to_filter = bus(2)
    after_filter = bus(2)

    # cook the noise
    if NOISE_TOGGLE:
        MODS['noise'] = boxSeq(MODS['noise_gain'], MODS['noise'])
        if FILTER_TOGGLE and FILTER_NOISE:
            to_filter = parallel_add(to_filter, MODS['noise'])
        else:
            after_filter = parallel_add(after_filter, MODS['noise'])

    # cook the sub
    if SUB_TOGGLE:
        MODS['sub'] = boxSeq(boxPar3(MODS['sub_gain'], MODS[f'sub_freq'], boxReal(0)), MODS['sub'])
        if FILTER_TOGGLE and FILTER_SUB:
            to_filter = parallel_add(to_filter, MODS['sub'])
        else:
            after_filter = parallel_add(after_filter, MODS['sub'])

    # cook the oscillators
    for name, osc_toggle, filter_osc_toggle in [('oscA', OSC_A_TOGGLE, FILTER_OSC_A), ('oscB', OSC_B_TOGGLE, FILTER_OSC_B)]:
        if osc_toggle:

            MODS[name] = boxSplit(
                boxParN([
                    MODS[f'{name}_waveform'], MODS[f'{name}_gain'], MODS[f'{name}_stereo_width'],
                    MODS[f'{name}_detune_amt'], MODS[f'{name}_blend'], MODS[f'{name}_freq'],
                    MODS[f'{name}_pan'], MODS['gate']
                    ]),
                MODS[name])

            if FILTER_TOGGLE and filter_osc_toggle:
                to_filter = parallel_add(to_filter, MODS[name])
            else:
                after_filter = parallel_add(after_filter, MODS[name])

    if FILTER_TOGGLE:
        to_filter = boxSeq(boxPar4(MODS['filter_cutoff'], MODS['filter_gain'], MODS['filter_resonance'], to_filter), MODS['filter'])

        after_filter = parallel_add(after_filter, to_filter)
    
    # plug up the leftover inputs with zero
    instrument = boxSplit(boxReal(0.), after_filter)

    # todo: use post fx
    process_modulations(EFFECTS_MODULATIONS)

    for effect in EFFECTS:
        
        if effect == 'reverb':
            pass
        elif effect == 'chorus':
            pass
        elif effect == 'delay':
            instrument = boxSeq(
                boxParN([MODS['delay_dtime'], MODS['delay_level'], MODS['delay_feedback'], MODS['delay_stereo'], MODS['delay_wet'], instrument]),
                MODS['delay'])
        else:
            raise ValueError(f'Unexpected effect named "{effect}".')

    # Done building the instrument.
    # f.boxToCPP(instrument)
    # return

    midi_basename = 'MIDI-Unprocessed_SMF_02_R1_2004_01-05_ORIG_MID--AUDIO_02_R1_2004_05_Track05_wav.midi'
    f.load_midi(abspath(ASSETS / midi_basename))

    f.num_voices = NUM_VOICES
    # f.dynamic_voices = False
    f.compile_box("test", instrument)

    desc = f.get_parameters_description()
    for parameter in desc:
        print(parameter)

    # todo: figure out why this prefix is dummy
    prefix = '/Polyphonic/Voices/dummy/'
    for parname, val in settings:
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

    box = f.boxFromDSP(f"""process = en.ahdsre(.1,.1,.1,.1);""")
    f.boxToCPP(box)


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
