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

// The following functions (lagrange_h, lagrangeN, frdtable)
// were written by Dario Sanfilippo and were merged into Faust here:
// https://github.com/grame-cncm/faustlibraries/pull/74
// They are reproduced here because the latest distribution of Faust
// still doesn't include them.

declare frdtable author "Dario Sanfilippo";
declare frdtable copyright "Copyright (C) 2021 Dario Sanfilippo
    <sanfilippo.dario@gmail.com>";
declare frdtable license "LGPL v3.0 license";

lagrange_h(N, idx) = par(n, N + 1, prod(k, N + 1, f(n, k)))
    with {
        f(n, k) = ((idx - k) * (n != k) + (n == k)) / ((n - k) + (n == k));
    };

lagrangeN(N, idx) = lagrange_h(N, idx) ,
                    si.bus(N + 1) : ro.interleave(N + 1, 2) : par(i, N + 1, *) :> _;
frdtable(N, S, init, idx) =
    lagrangeN(N, f_idx, par(i, N+1, table(i_idx - int(N / 2) + i)))
    with {
        table(j) = rdtable(S, init, int(ma.modulo(j, S)));
        f_idx = ma.frac(idx) + int(N / 2);
        i_idx = int(idx);
    };

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

process = frdtable(LAGRANGE_ORDER, SAMPLE_LENGTH, SAMPLE_L_SEQ, ridx)*gain*envVol*0.5,
          frdtable(LAGRANGE_ORDER, SAMPLE_LENGTH, SAMPLE_R_SEQ, ridx)*gain*envVol*0.5;
// polyphonic DSP code must declare a stereo effect
effect = _, _;