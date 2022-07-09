# import the Faust Box API
from dawdreamer.faust import *

# built-in modules
from enum import Enum
import warnings
from typing import List
import math

# other modules
from scipy import signal
import numpy as np


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


### convenience functions like faust libraries:

def semiToRatio(box: Box) -> Box:
    return boxSeq(box, boxPow(boxReal(2.), boxWire()/12.))


def boxParN(boxes: List[Box]):
    N = len(boxes)
    assert N > 0
    box = boxes.pop(0)
    while boxes:
        box = boxPar(box, boxes.pop(0))

    return box


def bus(n: int) -> Box:
    if n == 0:
        raise ValueError("Can't make a bus of size zero.")
    else:
        return boxParN([boxWire() for _ in range(n)])


def parallel_add(box1: Box, box2: Box) -> Box:
    return boxSeq(boxPar(box1, box2), boxMerge(bus(4), bus(2)))


def decimalpart() -> Box:

    return boxSub(boxWire(), boxIntCast(boxWire()))


def phasor(freq: Box) -> Box:

    return boxSeq(boxDiv(freq, boxSampleRate()), boxRec(boxSplit(boxAdd(), decimalpart()), boxWire()))


def osc(freq: Box) -> Box:
    return boxSin(boxMul(boxMul(boxReal(2.0), boxReal(math.pi)), phasor(freq)))


class ModularSynth:

    def __init__(self):
        self._MODS = {}
        self._NUM_MACROS = 4
        self._NUM_ENVS = 4
        self._NUM_LFOS = 4

        self._TABLE_SIZE = 16_384 # for saving wavecycles
        self._LAGRANGE_ORDER = 2  # for quality of anti-aliasing oscillators

    def _process_modulations(self, modulations):
        for source, dst, amt, symmetric in modulations:

            if source not in self._MODS:
                warnings.warn(f"""warning: source "{source}" was modulated but isn't used for DSP.""")
                continue
            if dst not in self._MODS:
                warnings.warn(f"""warning: destination "{dst}" was modulated but isn't used for DSP.""")
                continue

            if symmetric:
                self._MODS[dst] += (self._MODS[source]-.5)*amt*2.
            else:
                self._MODS[dst] += self._MODS[source]*amt

    def _make_macro(self, i: int):

        self._MODS[f'macro{i}'] = boxHSlider(f"h:Macro/Macro {i}", 0, 0, 1, .001)

    def _make_env(self, i: int):

        # time units are milliseconds
        self._MODS[f'env{i}_A'] = (boxWire() + boxHSlider(f"h:Env {i}/[0]Attack", 5, 0., 10_000, .001)) / 1_000
        self._MODS[f'env{i}_H'] = (boxWire() + boxHSlider(f"h:Env {i}/[1]Hold", 0, 0., 10_000, .001)) / 1_000
        self._MODS[f'env{i}_D'] = (boxWire() + boxHSlider(f"h:Env {i}/[2]Decay", 20, 0., 10_000, .001)) / 1_000
        self._MODS[f'env{i}_S'] = (boxWire() + boxHSlider(f"h:Env {i}/[3]Sustain", 0.5, 0., 10_000, .001))
        self._MODS[f'env{i}_R'] = (boxWire() + boxHSlider(f"h:Env {i}/[4]Release", 200, 0., 10_000, .001)) / 1_000
        self._MODS[f'env{i}'] = boxFromDSP(f"process = en.ahdsre;")

    def _make_lfo(self, i: int):

        self._MODS[f'lfo{i}_gain'] = boxWire() + boxHSlider(f"h:LFO {i}/[0]Gain", 1, 0, 10, .001)
        self._MODS[f'lfo{i}_freq'] = boxWire() + boxHSlider(f"h:LFO {i}/[1]Freq", 2, 0, 10, .001)

        self._MODS[f'lfo{i}'] = boxFromDSP(f"""process(gain, freq, gate) = gain * os.osc(freq);""")

    def _get_wavecycle_data(self, choice):

        TABLE_SIZE = self._TABLE_SIZE

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


    def _make_noise(self):

        TABLE_SIZE = self._TABLE_SIZE

        self._MODS['noise_gain'] = boxWire() + boxHSlider(f"h:Noise/[0]Gain", 0, 0, 10., .001)

        wavecycle_data = self._get_wavecycle_data(OscChoice.WHITE_NOISE)

        waveform_content = boxSeq(boxWaveform(wavecycle_data), boxPar(boxCut(), boxWire()))

        readTable = boxWire() * boxReadOnlyTable(boxInt(TABLE_SIZE), waveform_content, phasor(f, TABLE_SIZE/boxSampleRate())*TABLE_SIZE)

        self._MODS['noise'] = boxSplit(readTable, bus(2)) # split to stereo


    def _make_sub(self, choice):

        self._MODS[f'sub_gain']       = boxWire()             + boxHSlider(f"h:Sub/[0]Gain", 0, 0, 10, .001)
        self._MODS[f'sub_freq']       = semiToRatio(boxWire() + boxHSlider(f"h:Sub/[1]Freq", 0, -72, 72, .001)) * self._MODS['freq']
        self._MODS[f'sub_pan']        = boxWire() + boxHSlider(f"h:Sub/[2]Pan", .5, 0, 1, .001)
        self._MODS[f'sub_waveform']   = boxWaveform(self._get_wavecycle_data(choice))

        dsp_code = f"""
        LAGRANGE_ORDER = {self._LAGRANGE_ORDER};
        process(S, waveform_data, gain, freq, pan, gate) = result         
        with {{
          ridx = os.hs_phasor(S, freq, (gate:ba.impulsify) );
          result = it.frdtable(LAGRANGE_ORDER, S, waveform_data, ridx) * gain : sp.panner(pan);
        }};
        """
        self._MODS[f'sub'] = boxFromDSP(dsp_code)


    def _make_osc(self, x: str, choice, unison: int):

        self._MODS[f'osc{x}_gain']         = boxWire() + boxHSlider(f"h:Osc {x}/[0]Gain", 0, 0, 10, .001)
        self._MODS[f'osc{x}_freq']         = semiToRatio(boxWire() + boxHSlider(f"Osc {x} [1]Freq", 0, 0, 10, .001)) * self._MODS['freq']
        self._MODS[f'osc{x}_detune_amt']   = boxWire() + boxHSlider(f"h:Osc {x}/[2]Detune", 0.5, 0, 10, .001)
        self._MODS[f'osc{x}_blend']        = boxWire() + boxHSlider(f"h:Osc {x}/[3]Blend", 0.5, 0, 10, .001)
        self._MODS[f'osc{x}_pan']          = (boxWire() + boxHSlider(f"h:Osc {x}/[4]Pan", .5, 0, 1, .001))*2.-1
        self._MODS[f'osc{x}_wt_pos']       = boxWire() + boxHSlider(f"h:Osc {x}/[5]WT Pos", 0., 0, 1, .001)
        self._MODS[f'osc{x}_wt_bend']      = boxWire() + boxHSlider(f"h:Osc {x}/[6]WT Bend", 0., 0, 1, .001)
        self._MODS[f'osc{x}_phase']        = boxWire() + boxHSlider(f"h:Osc {x}/[7]Phase", 0, 0, 1, .001)
        self._MODS[f'osc{x}_rand']         = boxWire() + boxHSlider(f"h:Osc {x}/[8]Rand", 0, 0, 1, .001)
        self._MODS[f'osc{x}_stereo_width'] = boxWire() + boxHSlider(f"h:Osc {x}/[9]Stereo Width", 1, 0, 1, .001)

        self._MODS[f'osc{x}_waveform']     = boxWaveform(self._get_wavecycle_data(choice))

        dsp_code = f"""

        NUM_UNISON = {unison};
        LAGRANGE_ORDER = {self._LAGRANGE_ORDER};

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

          ridx = os.hs_phasor(S, freq*ratio, (gate:ba.impulsify) );
          result = it.frdtable(LAGRANGE_ORDER, S, waveform_data, ridx) * gain <: sp.panner(panOut);
        }};

        process(waveform_N, waveform_data, gain, width, amount, blend, freq, pan, gate) = result         
        with {{
          result = par(i, NUM_UNISON, unisonHelper(NUM_UNISON, i, waveform_N, waveform_data, width, amount, blend, freq, pan, gate)) :> sp.stereoize( _ * gain);  
        }};
        """

        self._MODS[f'osc{x}'] = boxFromDSP(dsp_code)

    def _make_filter(self, choice):

        self._MODS['filter_cutoff']    = boxWire()+boxHSlider(f"h:Filter/Cutoff", 5000., 20., 20000, .001)
        self._MODS['filter_gain']      = boxWire()+boxHSlider(f"h:Filter/Gain", 0., -80, 24, .001)
        self._MODS['filter_resonance'] = boxWire()+boxHSlider(f"h:Filter/Resonance", 1, 0, 2, .001)

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

        self._MODS['filter'] = boxFromDSP(dsp)


    def _make_reverb(self):
        self._MODS['reverb_cutoff']  = boxWire()+boxHSlider(f"h:Reverb/Filter Cutoff", 5000., 20., 20000, .001)
        self._MODS['reverb_size']    = boxWire()+boxHSlider(f"h:Reverb/Size", 0, 0, 1, .001)
        self._MODS['reverb_mix']     = boxWire()+boxHSlider(f"h:Reverb/Mix", 0, 0, 1, .001)

        self._MODS['reverb'] = boxMerge(bus(3), boxWire())


    def _make_chorus(self):
        pass  # todo:


    def _make_delay(self):

        MAXDELAY = 1.
        DELAYORDER = 5;

        self._MODS['delay_dtime']    = boxHSlider("h:Delay/Time", .125, 0., MAXDELAY, 0);
        self._MODS['delay_level']    = boxHSlider("h:Delay/Level", 1, 0, 1, 0)
        self._MODS['delay_feedback'] = boxHSlider("h:Delay/Feedback", 0.8, 0, 1, 0)
        self._MODS['delay_stereo']   = boxHSlider("h:Delay/Stereo", 1, 0, 1, 0)
        self._MODS['delay_wet']      = boxHSlider("h:Delay/Wet", .5, 0, 1, 0)

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

        self._MODS['delay'] = boxFromDSP(dsp_code)

    @staticmethod
    def _parse_modulations(all_modulations):

        MACRO_MODULATIONS = []
        OTHER_MODULATIONS = []
        EFFECTS_MODULATIONS = []
        for modulation in all_modulations:
            source, dst = modulation[0].lower(), modulation[1].lower()
            if (
                source.startswith('macro') or \
                source.startswith('gain') or \
                source.startswith('gate') or \
                source.startswith('freq')
                ):
                MACRO_MODULATIONS.append(modulation)
            elif source.startswith("env") or source.startswith("lfo"):
                OTHER_MODULATIONS.append(modulation)
            else:
                EFFECTS_MODULATIONS.append(modulation)

        return MACRO_MODULATIONS, OTHER_MODULATIONS, EFFECTS_MODULATIONS

    def build(self, cfg: dict) -> Box:

        MACRO_MODULATIONS, OTHER_MODULATIONS, EFFECTS_MODULATIONS = self._parse_modulations(cfg['MODULATION_MATRIX'])

        OSC_A_TOGGLE = cfg['OSC_A_TOGGLE']
        OSC_A_CHOICE = cfg['OSC_A_CHOICE']
        OSC_A_UNISON = cfg['OSC_A_UNISON']

        OSC_B_TOGGLE = cfg['OSC_B_TOGGLE']
        OSC_B_CHOICE = cfg['OSC_B_CHOICE']
        OSC_B_UNISON = cfg['OSC_B_UNISON']

        SUB_TOGGLE = cfg['SUB_TOGGLE']
        SUB_CHOICE = cfg['SUB_CHOICE']

        NOISE_TOGGLE = cfg['NOISE_TOGGLE']

        FILTER_TOGGLE = cfg['FILTER_TOGGLE']
        FILTER_CHOICE = cfg['FILTER_CHOICE']
        FILTER_OSC_A = cfg['FILTER_OSC_A']
        FILTER_OSC_B = cfg['FILTER_OSC_B']
        FILTER_NOISE = cfg['FILTER_NOISE']
        FILTER_SUB = cfg['FILTER_SUB']

        # ordered list of post-fx to use
        EFFECTS = cfg['EFFECTS']

        self._MODS = {
            'freq': boxHSlider("freq", 100, 100, 20000, .001),
            'gate': boxButton("gate"),
            'gain': boxHSlider("gain", .5, 0, 1, .001)
        }

        for i in range(self._NUM_MACROS):
            self._make_macro(i+1)

        for i in range(self._NUM_ENVS):
            self._make_env(i+1)

        for i in range(self._NUM_LFOS):
            self._make_lfo(i+1)

        if OSC_A_TOGGLE:
            self._make_osc('A', OSC_A_CHOICE, OSC_A_UNISON)

        if OSC_B_TOGGLE:
            self._make_osc('B', OSC_B_CHOICE, OSC_B_UNISON)

        if SUB_TOGGLE:
            self._make_sub(SUB_CHOICE)

        if FILTER_TOGGLE:
            self._make_filter(FILTER_CHOICE)

        if NOISE_TOGGLE:
            self._make_noise()

        if 'reverb' in EFFECTS:
            self._make_reverb()

        if 'chorus' in EFFECTS:
            self._make_chorus()

        if 'delay' in EFFECTS:
            self._make_delay()

        # modulate the destinations we just made.
        self._process_modulations(MACRO_MODULATIONS)

        # cook the envelopes
        for i in range(1, self._NUM_ENVS+1):
            self._MODS[f'env{i}'] = boxSeq(
                boxParN([self._MODS[f'env{i}_A'],self._MODS[f'env{i}_H'], self._MODS[f'env{i}_D'], 
                    self._MODS[f'env{i}_S'], self._MODS[f'env{i}_R'], self._MODS['gate']]),
                self._MODS[f'env{i}']
                )

        # cook the LFOs
        for i in range(1, self._NUM_LFOS+1):
            self._MODS[f'lfo{i}'] = boxSeq(
                boxPar3(self._MODS[f'lfo{i}_gain'], self._MODS[f'lfo{i}_freq'], self._MODS['gate']),
                self._MODS[f'lfo{i}'])

        # modulate the destinations that have envelopes and LFOs as a source.
        self._process_modulations(OTHER_MODULATIONS)
            
        to_filter = bus(2)
        after_filter = bus(2)

        # cook the noise
        if NOISE_TOGGLE:
            self._MODS['noise'] = boxSeq(self._MODS['noise_gain'], self._MODS['noise'])
            if FILTER_TOGGLE and FILTER_NOISE:
                to_filter = parallel_add(to_filter, self._MODS['noise'])
            else:
                after_filter = parallel_add(after_filter, self._MODS['noise'])

        # cook the sub
        if SUB_TOGGLE:
            self._MODS['sub'] = boxSeq(
                boxPar5(self._MODS['sub_waveform'], self._MODS['sub_gain'], self._MODS[f'sub_freq'], self._MODS[f'sub_pan'], self._MODS[f'gate']), self._MODS['sub'])
            if FILTER_TOGGLE and FILTER_SUB:
                to_filter = parallel_add(to_filter, self._MODS['sub'])
            else:
                after_filter = parallel_add(after_filter, self._MODS['sub'])

        # cook the oscillators
        for name, osc_toggle, filter_osc_toggle in [('oscA', OSC_A_TOGGLE, FILTER_OSC_A), ('oscB', OSC_B_TOGGLE, FILTER_OSC_B)]:
            if osc_toggle:

                self._MODS[name] = boxSplit(
                    boxParN([
                        self._MODS[f'{name}_waveform'], self._MODS[f'{name}_gain'], self._MODS[f'{name}_stereo_width'],
                        self._MODS[f'{name}_detune_amt'], self._MODS[f'{name}_blend'], self._MODS[f'{name}_freq'],
                        self._MODS[f'{name}_pan'], self._MODS['gate']
                        ]),
                    self._MODS[name])

                if FILTER_TOGGLE and filter_osc_toggle:
                    to_filter = parallel_add(to_filter, self._MODS[name])
                else:
                    after_filter = parallel_add(after_filter, self._MODS[name])

        if FILTER_TOGGLE:
            to_filter = boxSeq(boxPar4(self._MODS['filter_cutoff'], self._MODS['filter_gain'], self._MODS['filter_resonance'], to_filter), self._MODS['filter'])

            after_filter = parallel_add(after_filter, to_filter)
        
        # plug up the leftover inputs with zero
        instrument = boxSplit(boxReal(0.), after_filter)

        self._process_modulations(EFFECTS_MODULATIONS)

        for effect in EFFECTS:
            
            if effect == 'reverb':
                pass
            elif effect == 'chorus':
                pass
            elif effect == 'delay':
                instrument = boxSeq(
                    boxParN([self._MODS['delay_dtime'], self._MODS['delay_level'], self._MODS['delay_feedback'], self._MODS['delay_stereo'], self._MODS['delay_wet'], instrument]),
                    self._MODS['delay'])
            else:
                raise ValueError(f'Unexpected effect named "{effect}".')

        return instrument
