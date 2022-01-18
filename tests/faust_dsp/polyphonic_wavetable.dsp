declare name "MyInstrument";

declare options "[nvoices:8]"; // FaustProcessor has a property which will override this.
import("stdfaust.lib");

// This example demonstrates using lagrange interpolation to improve
// the sampling of a short wavetable.
// Specifically, from Python we pass myCycle, 4 samples of a sine wave,
// so the values are {0, 1, 0, -1}.
// However, with Lagrange interpolation, we can turn these into a smooth sine wave!

// The following variable is excluded from this file because they come
// from substitution with Python.
// LAGRANGE_ORDER = 4; // lagrange order. [2-4] are good choices.

freq = hslider("freq",200,50,1000,0.01); // note pitch
gain = hslider("gain",0.1,0,1,0.01);     // note velocity
gate = button("gate");                   // note on/off

soundfile_full = soundfile("myCycle",1): _, !, _;

S = 0, 0 : soundfile_full : _, !;
soundfile_table = 0, _ : soundfile_full : !, _;

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

envVol = en.adsr(.002, 0.1, 0.9, .1, gate);

ridx = os.hs_phasor(S, freq, envVol == 0.); // or (abs(envVol) < 1e-4)

process = frdtable(LAGRANGE_ORDER, S, soundfile_table, ridx)*gain*envVol*0.5 <: _, _;
effect = _, _;