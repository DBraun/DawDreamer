declare name "MyInstrument";

declare options "[nvoices:8]"; // FaustProcessor has a property which will override this.
import("stdfaust.lib");

// This example demonstrates using lagrange interpolation to improve
// the sampling of a short waveform.
// Specifically, we simulate only having 4 samples of a sine wave,
// so the values are {0, 1, 0, -1}.
// However, with Lagrange interpolation, we can turn these into a smooth sine wave!

// The following variables are excluded from this file because they come
// from substitution with Python.
// LAGRANGE_ORDER = 4; // lagrange order. [2-4] are good choices.
// CYCLE_SEQ = waveform{0.0, 1.0, 0.0, -1.0} : !, _;
// CYCLE_LENGTH = 4; // the length of CYCLE_SEQ

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

freq = hslider("freq",200,50,1000,0.01); // note pitch
gain = hslider("gain",0.1,0,1,0.01);     // note velocity
gate = button("gate");                   // note on/off

envVol = en.adsr(.002, 0.1, 0.9, .1, gate);

ridx = os.hs_phasor(CYCLE_LENGTH, freq, envVol == 0.); // or (abs(envVol) < 1e-4)

process = frdtable(LAGRANGE_ORDER, CYCLE_LENGTH, CYCLE_SEQ, ridx)*gain*envVol*0.5 <: _, _;
// polyphonic DSP code must declare a stereo effect
effect = _, _;