declare name "MyInstrument";

declare options "[nvoices:8]"; // FaustProcessor has a property which will override this.
import("stdfaust.lib");

// This example demonstrates a "sampler" in Faust, and it happens to use
// Lagrange interpolation. The interpolation probably only matters
// when the sampler is played at a MIDI note other than the "center_note".

// The following variables are excluded from this file because they come
// from substitution with Python.
// LAGRANGE_ORDER = 4; // lagrange order. [2-4] are good choices.
// SAMPLE_L_SEQ = waveform{0.0, 0.0} : !, _;
// SAMPLE_R_SEQ = waveform{0.0, 0.0} : !, _;
// SAMPLE_LENGTH = 4; // the length of SAMPLE_L_SEQ and SAMPLE_R_SEQ

// variation of hs_phasor that doesn't loop. It's like a one-shot trigger.
my_phasor(tablesize,freq,c) = inc*on_memory : + ~ (_*(1-start_pulse)) : min(1.) *(tablesize)
with {
    is_on = c>0;
    
    start_pulse = is_on & (1-is_on');
    on_memory = is_on : max ~ (_*(1-start_pulse));
    
    inc = freq/float(ma.SR);
};

gain = hslider("gain",0.1,0,1,0.01);     // note velocity
gate = button("gate");                   // note on/off
key = hslider("freq", 60, 1, 127, 1) : ba.hz2midikey;

root_midi = hslider("center_note", 60., 1., 128., 0.01);
semitones = key - root_midi;
ratio = semitones : ba.semi2ratio;
length_sec = SAMPLE_LENGTH / ma.SR;

freq = ratio / length_sec;

envVol = en.adsr(.002, 0.1, 0.9, .1, gate);

ridx = my_phasor(SAMPLE_LENGTH, freq, gate);

process = it.frdtable(LAGRANGE_ORDER, SAMPLE_LENGTH, SAMPLE_L_SEQ, ridx)*gain*envVol*0.5,
          it.frdtable(LAGRANGE_ORDER, SAMPLE_LENGTH, SAMPLE_R_SEQ, ridx)*gain*envVol*0.5;
// polyphonic DSP code must declare a stereo effect
effect = _, _;