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
    # filter, inputs, outputs = f.dsp_to_box('process = fi.lowpass3e(1);')
    # filter, inputs, outputs = f.dsp_to_box('process = _,_;');

    # assert inputs == 1
    # assert outputs == 1
    # print(f'inputs: {inputs}, outputs: {outputs}')

    cutoff = f.boxHSlider("cutoff", f.boxReal(300), f.boxReal(100), f.boxReal(2000), f.boxReal(0.01))
    cutoffAndInput = f.boxPar(cutoff, f.boxWire())
    # cutoffAndInput, inputs, outputs = f.dsp_to_box('process = hslider("cutoff", 300, 100, 2000, .01), _;')
    # cutoffAndInput, inputs, outputs = f.dsp_to_box('process = _, _, _;')

    # print(f'inputs: {inputs}, outputs: {outputs}')

    filteredInput = f.boxSeq(cutoffAndInput, filter)
    f.compile_box("test", filteredInput)
    my_render(engine, f)


# def test25b():

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")

#     filter, inputs, outputs = f.dsp_to_box('process = si.smooth;');
#     cutoffAndInput, inputs, outputs = f.dsp_to_box('process = hslider("cutoff", 300, 100, 2000, .01), _;')

#     filteredInput = f.boxSeq(cutoffAndInput, filter)
#     f.compile_box("test", filteredInput)
#     my_render(engine, f)


# def test25c():

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")

#     bus_size = 2
#     box, inputs, outputs = f.dsp_to_box(f'process = si.bus({bus_size});');
#     print(f'1) inputs: {inputs}, outputs: {outputs}')

#     assert inputs ==  bus_size and outputs == bus_size


#     bus_size = 3
#     box, inputs, outputs = f.dsp_to_box(f'process = si.bus({bus_size});');
#     print(f'2) inputs: {inputs}, outputs: {outputs}')

#     assert inputs ==  bus_size and outputs == bus_size

#     f.compile_box("test", box)
#     my_render(engine, f)


# def test26():

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")

#     box1 = f.boxWire()

#     box2 = f.boxMul(box1, f.boxReal(0.5))
#     box3 = f.boxRem(box1, f.boxReal(0.8))

#     box4 = f.boxSeq(box2, box1)
#     box5 = f.boxSeq(box3, box1)

#     box6 = f.boxPar3(box4, box5, box3)


# def test27():

#     engine = daw.RenderEngine(SAMPLE_RATE, BUFFER_SIZE)
#     f = engine.make_faust_processor("my_faust")

#     box4 = f.boxFromDSP('process = os.osc;')
#     box5 = f.boxFromDSP('process = en.adsr;')
#     box6 = f.boxFromDSP('process = en.adsr;')
#     box7 = f.boxFromDSP('process = fi.lowpass(5);')

def test28():

    """Serum-like synthesizer"""

    ###### compile time constants ######

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
        WHITE_NOISE = 4

    OSC_A_TOGGLE = True
    OSC_A_CHOICE = OscChoice.SAWTOOTH
    OSC_A_UNISON = 1

    OSC_B_TOGGLE = False
    OSC_B_CHOICE = OscChoice.SAWTOOTH
    OSC_B_UNISON = 1

    SUB_TOGGLE = True
    SUB_CHOICE = OscChoice.SINE

    NOISE_TOGGLE = False

    FILTER_TOGGLE = False
    FILTER_CHOICE = FilterChoice.LOWPASS_12
    FILTER_OSC_A = True
    FILTER_OSC_B = True
    FILTER_NOISE = True
    FILTER_SUB = True

    NUM_MACROS = 4
    NUM_ENVS = 4
    NUM_LFOS = 4
    NUM_VOICES = 12

    MACRO_MODULATIONS = [
        # source should be macro, gain, gate, freq
        ("macro1", "env1_A", 1, False),
        ("macro2", "oscA_gain", .3, False),
        # ("gain", "oscA_gain", .1, False),
        # ("gain", "oscB_gain", .1, False),
        # ("gain", "oscA_detune_amt", 1., False),
    ]

    OTHER_MODULATIONS = [
        # source must be env or LFO
        # todo: what if env is connected to LFO or vice versa?
        ("env1", "oscA_gain", .1, False),
        # ("env2", "oscA_freq", 12., False),  # semitone units
        # ("lfo1", "oscB_gain", .5, False),
        # ("lfo1", "oscB_gain", .5, False),
        # ("env3", "filter_cutoff", 0.2, False)
    ]

    # ordered list of post-fx to use
    EFFECTS = []
    # EFFECTS = ['reverb', 'chorus']  # todo: add these as features
    EFFECTS_MODULATIONS = []  # todo:

    TABLE_SIZE = 16_384

    #####################################

    ##### No need to modify below.  #####

    engine = daw.RenderEngine(SAMPLE_RATE, 512)
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

    def boxHSlider(label: str, default: float, minVal: float, maxVal: float, step: float):
        return f.boxHSlider(label, boxReal(default), boxReal(minVal), boxReal(maxVal), boxReal(step))

    ### convenience functions like faust libraries:

    def semiToRatio(box: daw.Box) -> daw.Box:
        return boxSeq(box, boxPow(boxReal(2.), boxWire()/12.))

    def bus(n: int) -> daw.Box:
        if n == 0:
            raise ValueError("Can't make a bus of size zero.")
        elif n == 1:
            return boxWire()
        else:
            box = boxWire()
            for i in range(n-1):
                box = boxPar(box, boxWire())

            return box


    def parallel_add(box1: daw.Box, box2: daw.Box) -> daw.Box:
        return boxSeq(boxPar(box1, box2), boxMerge(bus(4), bus(2)))

    #################################################


    MODS = {
        'freq': boxHSlider("freq", 100, 100, 20000, .001),
        'gate': f.boxButton("gate"),
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

            if symmetric:
                MODS[dst] += (MODS[source]-.5)*amt*2.
            else:
                MODS[dst] += MODS[source]*amt


    def make_macro(i: int):

        MODS[f'macro{i}'] = boxHSlider(f"[{i}]Macro {i}", 0, -1, 1, .001)


    def make_env(i: int):

        # time units are milliseconds
        MODS[f'env{i}_A'] = (boxWire() + boxHSlider(f"Env {i} [0]Attack", 5, 0., 10_000, .001)) / 1_000
        MODS[f'env{i}_H'] = (boxWire() + boxHSlider(f"Env {i} [1]Hold", 0, 0., 10_000, .001)) / 1_000
        MODS[f'env{i}_D'] = (boxWire() + boxHSlider(f"Env {i} [2]Decay", 20, 0., 10_000, .001)) / 1_000
        MODS[f'env{i}_S'] = (boxWire() + boxHSlider(f"Env {i} [3]Sustain", 0.5, 0., 10_000, .001))
        MODS[f'env{i}_R'] = (boxWire() + boxHSlider(f"Env {i} [4]Release", 200, 0., 10_000, .001)) / 1_000
        MODS[f'env{i}'] = f.boxFromDSP(f"process = en.ahdsre;")

    def make_lfo(i: int):

        MODS[f'lfo{i}_gain'] = boxWire() + boxHSlider(f"LFO {i} [0]Gain", 0, 0, 10, .001)
        MODS[f'lfo{i}_freq'] = boxWire() + boxHSlider(f"LFO {i} [1]Freq", 0, 0, 10, .001)

        # todo: use boxFromDSP
        # lfo = f.boxFromDSP(f"""process(gain, freq) = (gain + hslider("[{i}]LFO %{i} gain",  0., 0., 10., .001)) * os.osc(freq);""")
        # MODS[f'lfo{i}'] = f.boxMerge(f.boxPar(f.boxWire(), f.boxWire()), f.boxWire())
        MODS[f'lfo{i}'] = boxWire() * osc(f, boxWire())


    def get_wavecycle_data(choice):

        if choice == OscChoice.SINE:
            # sine wave
            wavecycle_data = np.sin(np.pi*2*np.linspace(0, 1, TABLE_SIZE, endpoint=False))
        elif choice == OscChoice.SAWTOOTH:
            # sawtooth
            wavecycle_data = -1. + 2.*np.linspace(0, 1, TABLE_SIZE, endpoint=False)
        elif choice == OscChoice.TRIANGLE:
            # triangle
            t = np.linspace(0, 1, TABLE_SIZE, endpoint=False)
            t = np.concatenate([t[TABLE_SIZE//4:], t[:TABLE_SIZE//4]])
            assert t.shape[0] == TABLE_SIZE
            wavecycle_data = signal.sawtooth(2 * np.pi * t, 0.5)
        elif choice == OscChoice.WHITE_NOISE:
            wavecycle_data = -1.+2.*np.random.rand(TABLE_SIZE)
        else:
            raise ValueError(f"Unexpected oscillator choice: {choice}.")

        return wavecycle_data.tolist()


    def make_noise():

        MODS['noise_gain'] = boxWire() + boxHSlider(f"Noise [0]Gain", 0, 0, 10., .001)

        wavecycle_data = get_wavecycle_data(OscChoice.WHITE_NOISE)

        waveform_content = boxSeq(f.boxWaveform(wavecycle_data), boxPar(boxCut(), boxWire()))

        readTable = boxWire() * f.boxReadOnlyTable(boxInt(TABLE_SIZE), waveform_content, phasor(f, TABLE_SIZE/f.boxSampleRate())*TABLE_SIZE)

        MODS['noise'] = boxSplit(readTable, bus(2)) # split to stereo


    def make_sub(choice):

        MODS[f'sub_gain']       = boxWire()                + boxHSlider(f"Sub [0]Gain", 0, 0, 10, .001)
        MODS[f'sub_freq']       = semiToRatio(boxWire() + boxHSlider(f"Sub [1]Freq", 0, 0, 10, .001)) * MODS['freq']
        MODS[f'sub_pan']        = boxWire()                + boxHSlider(f"Sub [2]Pan", 0, 0, 10, .001)

        wavecycle_data = get_wavecycle_data(choice)

        waveform_content = boxSeq(f.boxWaveform(wavecycle_data), boxPar(boxCut(), boxWire()))

        readTable = boxWire() * f.boxReadOnlyTable(boxInt(TABLE_SIZE), waveform_content, phasor(f, boxWire() + boxWire())*TABLE_SIZE)

        MODS[f'sub'] = boxSplit(readTable, bus(2)) # split to stereo


    def make_osc(x: str, choice, unison: int):

        MODS[f'osc{x}_gain']       = boxWire() + boxHSlider(f"Osc {x} [0]Gain", 0, 0, 10, .001)
        MODS[f'osc{x}_freq']       = semiToRatio(boxWire() + boxHSlider(f"Osc {x} [1]Freq", 0, 0, 10, .001)) * MODS['freq']
        MODS[f'osc{x}_detune_amt'] = boxWire() + boxHSlider(f"Osc {x} [2]Detune", 0, 0, 10, .001)
        MODS[f'osc{x}_blend']      = boxWire() + boxHSlider(f"Osc {x} [3]Blend", 0, 0, 10, .001)
        MODS[f'osc{x}_pan']        = boxWire() + boxHSlider(f"Osc {x} [4]Pan", 0, 0, 10, .001)

        wavecycle_data = get_wavecycle_data(choice)

        waveform_content = boxSeq(f.boxWaveform(wavecycle_data), boxPar(boxCut(), boxWire()))

        readTable = boxWire() * f.boxReadOnlyTable(boxInt(TABLE_SIZE), waveform_content, phasor(f, boxWire() + boxWire())*TABLE_SIZE)

        MODS[f'osc{x}'] = boxSplit(readTable, bus(2)) # split to stereo

        # MODS[f'osc{x}'] = f.boxFromDSP("process(gain, freq, detune_amt) = gain* os.sawtooth(freq) + detune_amt <: _, _;")
        # MODS[f'osc{x}'] = f.boxSplit(f.boxMerge(bus(3), f.boxWire()), bus(2))
        # MODS[f'osc{x}'] = f.boxSplit(f.boxMerge(bus(3), f.boxWire()), f.boxPar(f.boxWire(), f.boxWire()*f.boxHSlider(f"Osc {x} Blah", f.boxReal(1.), f.boxReal(0.), f.boxReal(10.), f.boxReal(.001))))

        # if unison == 1:
        #     # no detune
        #     detune_calc = "detune_calc(i, freq, detuneAmt, blendAmt) = freq;"
        # elif unison % 2 == 0:
        #     # even: todo
        #     detune_calc = "detune_calc(i, freq, detuneAmt, blendAmt) = freq;"
        # else:
        #     # odd: todo
        #     detune_calc = "detune_calc(i, freq, detuneAmt, blendAmt) = freq;" 

        # osc_gain = f.boxFromDSP(f"""process = _ * hslider("OSC {X} Gain", 0, 0., 10., .001);""")
        # osc_freq = f.boxFromDSP(f"""process = _ + hslider("OSC {X} Octave", 0, -12., 12., .001) : ba.semi2ratio;""") * freq
        # osc_detune_amt = f.boxFromDSP(f"""process = _ + hslider("OSC {X} Detune Amt", 0, 0., 1., .001);""")           

        # if choice == 0:
        #     oscA = f.boxFromDSP(f"{detune_calc} process(gain, freq, detuneAmt, blendAmt) = gain * par(i, {unison}, (detune_calc(i, freq, detuneAmt, blendAmt) : os.sawtooth)) :> _ / {unison} <: _, _;")
        # else:
        #     oscA = f.boxFromDSP(f"{detune_calc} process(gain, freq, detuneAmt, blendAmt) = gain * par(i, {unison}, (detune_calc(i, freq, detuneAmt, blendAmt) : os.osc)) :> _ / {unison} <: _, _;")


    def make_filter(choice):

        MODS['filter_cutoff']    = boxWire()+boxHSlider(f"Filter Cutoff", 5000., 20., 20000, .001)
        MODS['filter_gain']      = boxWire()+boxHSlider(f"Filter Gain", 0., -80, 24, .001)
        MODS['filter_resonance'] = boxWire()+boxHSlider(f"Filter Resonance", 1, 0, 2, .001)

        if choice == FilterChoice.LOWPASS_12:
            dsp = "process(cutoff, gain, res, sig) = fi.lowpass(5, cutoff, sig);"
        elif choice == FilterChoice.LOWPASS_24:
            dsp = "process(cutoff, gain, res, sig) = fi.lowpass(15, cutoff, sig);"
        elif choice == FilterChoice.HIGHPASS_12:
            dsp = "process(cutoff, gain, res, sig) = fi.highpass(5, cutoff, sig);"
        elif choice == FilterChoice.HIGHPASS_24:
            dsp = "process(cutoff, gain, res, sig) = fi.highpass(15, cutoff, sig);"
        elif choice == FilterChoice.LOWSHELF_12:
            dsp = "process(cutoff, gain, res, sig) = fi.lowshelf(3, gain, cutoff, sig);"
        elif choice == FilterChoice.HIGHSHELF_12:
            dsp = "process(cutoff, gain, res, sig) = fi.highshelf(3, gain, cutoff, sig);"
        else:
            raise ValueError(f"Unexpected filter choice: {choice}.")

        # todo: note that this is just a mono filter!
        MODS['filter'] = f.boxFromDSP(dsp)


    def make_reverb():
        pass  # todo:
        MODS['reverb_cutoff']  = boxWire()+boxHSlider(f"Reverb Filter Cutoff", 5000., 20., 20000, .001)
        MODS['reverb_size']    = boxWire()+boxHSlider(f"Reverb Size", 0, 0, 1, .001)
        MODS['reverb_mix']     = boxWire()+boxHSlider(f"Reverb Mix", 0, 0, 1, .001)

        MODS['reverb'] = boxMerge(bus(3), boxWire())

    def make_chorus():
        pass  # todo:


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

    # modulate the destinations we just made.
    process_modulations(MACRO_MODULATIONS)

    # cook the envelopes
    for i in range(1, NUM_ENVS+1):
        MODS[f'env{i}'] = boxSeq(
            boxPar(
                boxPar3(MODS[f'env{i}_A'],MODS[f'env{i}_H'], MODS[f'env{i}_D']), 
                boxPar3(MODS[f'env{i}_S'], MODS[f'env{i}_R'], MODS['gate'])
            ),
            MODS[f'env{i}'])

    # cook the LFOs
    for i in range(1, NUM_LFOS+1):
        MODS[f'lfo{i}'] = boxSeq(
            boxPar(MODS[f'lfo{i}_gain'], MODS[f'lfo{i}_freq']),
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

            MODS[name] = boxSeq(boxPar3(MODS[f'{name}_gain'], MODS[f'{name}_freq'], MODS[f'{name}_detune_amt']), MODS[name])

            if FILTER_TOGGLE and filter_osc_toggle:
                to_filter = parallel_add(to_filter, MODS[name])
            else:
                after_filter = parallel_add(after_filter, MODS[name])

    if FILTER_TOGGLE:
        # todo: more elegant way that doesn't split channels like this.
        # It would be easier if the *stereo* filter actually came from boxFromDSP.
        to_filter_L = boxSeq(to_filter, boxPar(boxWire(), boxCut()))
        to_filter_R = boxSeq(to_filter, boxPar(boxCut(), boxWire()))

        L = boxSeq(boxPar4(MODS['filter_cutoff'], MODS['filter_gain'], MODS['filter_resonance'], to_filter_L), MODS['filter'])
        R = boxSeq(boxPar4(MODS['filter_cutoff'], MODS['filter_gain'], MODS['filter_resonance'], to_filter_R), MODS['filter'])

        after_filter = parallel_add(after_filter, boxPar(L, R))
    
    # plug up the leftover inputs with zero
    instrument = boxSplit(boxReal(0.), after_filter)

    # todo: use post fx
    process_modulations(EFFECTS_MODULATIONS)

    for effect in EFFECTS:
        
        if effect == 'reverb':
            pass
        elif effect == 'chorus':
            pass
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

    my_render(engine, f)
    audio = engine.get_audio().T
    assert np.mean(np.abs(audio)) > .01

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
