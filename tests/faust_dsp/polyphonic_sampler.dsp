declare name "MyInstrument";

declare options "[nvoices:8]"; // FaustProcessor has a property which will override this.
import("stdfaust.lib");

// This example demonstrates a "sampler" in Faust, and it happens to use
// Lagrange interpolation. The interpolation probably only matters
// when the sampler is played at a MIDI note other than the "center_note".

// The following variable is excluded from this file because they come
// from substitution with Python.
// LAGRANGE_ORDER = 4; // lagrange order. [2-4] are good choices.

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

soundfile_full = soundfile("mySample",2): _, !, _, _;
S = 0, 0 : soundfile_full : _, !, !;
soundfile_table_L = 0, _ : soundfile_full : !, _, !;
soundfile_table_R = 0, _ : soundfile_full : !, !, _;

length_sec = S / ma.SR;

freq = ratio / length_sec;

envVol = en.adsr(.002, 0.1, 0.9, .1, gate);

ridx = my_phasor(S, freq, gate);

declare lagrangeCoeffs author "Dario Sanfilippo";
declare lagrangeCoeffs copyright "Copyright (C) 2021 Dario Sanfilippo
    <sanfilippo.dario@gmail.com>";
declare lagrangeCoeffs license "MIT license";
// NOTE: this is a modification of the original it.frdtable so that
// it works with a soundfile
// https://github.com/grame-cncm/faustlibraries/blob/master/interpolators.lib
frdtable(N, S, init, idx) =
    it.lagrangeN(N, f_idx, par(i, N + 1, table(i_idx - int(N / 2) + i)))
    with {
        table(j) = int(ma.modulo(j, S)) : init;
        f_idx = ma.frac(idx) + int(N / 2);
        i_idx = int(idx);
    };

process = frdtable(LAGRANGE_ORDER, S, soundfile_table_L, ridx),
          frdtable(LAGRANGE_ORDER, S, soundfile_table_R, ridx) <: _*finalGain, _*finalGain
with {
	finalGain = gain*envVol*0.5;
};
// polyphonic DSP code must declare a stereo effect
effect = _, _;