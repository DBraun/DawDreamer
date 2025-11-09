declare name "MyInstrument";

declare options "[nvoices:8]"; // FaustProcessor has a property which will override this.
import("stdfaust.lib");

// This example demonstrates a "sampler" in Faust, and it happens to use
// Lagrange interpolation. The interpolation mostly matters
// when the sampler is played at a MIDI note other than the "center_note".

// The following variable is excluded from this file because they come
// from substitution with Python.
// LAGRANGE_ORDER = 4; // lagrange order. [2-4] are good choices.

// mathematical hard-syncing phasor (see `phasor_imp` in `faustlibraries/oscillators.lib`)
m_hsp_phasor(freq, reset, phase) = (select2(hard_reset, +(freq/ma.SR), phase) : ma.decimal) ~ _
with {
    hard_reset = (1-1')|reset; // To correctly start at `phase` at the first sample
};

// NOTE: this is a modification of the original it.frdtable so that
// it works with a soundfile
// https://github.com/grame-cncm/faustlibraries/blob/master/interpolators.lib
// Inputs:
// * `sf`: a soundfile primitive such as `soundfile("foo.wav", 2)`
// * `N`: The Lagrange interpolation order (compile-time constant). See docs for `it.frdtable`.
// * `sound_number`: The sound number choice for the `soundfile` primitive.
// * `root_midi`: The MIDI note to consider as the root pitch. If the frequency of this note is played, then the soundfile will be played at its original pitch.
// * `freq`: A frequency in Hz which should probably come from an hslider for "freq" with polyphony.
// * `gain`: A gain in range [0-1] which serves as a note velocity.
// * `gate`: A gate which indicates the note's on/off state.
oneshot_sf(sf, N, sound_number, root_midi, freq, gain, gate) = par(i, N + 1, table(i_idx - int(N / 2) + i)) : ro.interleave(C, N+1) : par(i, C, it.lagrangeN(N, f_idx)) : volBus
with {
    SFC = outputs(sf);
    C = SFC-2; // num output channels. We subtract 2 to skip the outputs of sample rate and num samples
    S = sound_number, 0 : sf : ba.selector(0, SFC); // table size

    noteOn = gate & (1-gate'); // impulse for a note on event
    everPlayed = max(noteOn)~_;  // whether noteOn has ever happened.
    hold_timer = everPlayed : (+~ba.if(noteOn,0,_));
    phasor_freq = ba.hz2midikey(freq) - root_midi : ba.semi2ratio : _*everPlayed*ma.SR/S;
    pct = m_hsp_phasor(phasor_freq, noteOn, 0);

    table(j) = int(ma.modulo(j, S)) : sf(sound_number) : !, !, si.bus(C);
    idx = S*pct;
    f_idx = ma.frac(idx) + int(N / 2);
    i_idx = int(idx);

    volBus = par(i, C, ba.if((hold_timer>=S),0,_*gain));
};

gain = hslider("gain",0.1,0,1,0.01);     // note velocity
gate = button("gate");                   // note on/off
freq = hslider("freq", 20, 1, 20000, 1);

root_midi = hslider("center_note", 60., 1., 128., 0.01);

envVol = 0.5*gain*en.adsr(.002, 0.1, 0.9, .1, gate);

sound_number = 0;
process = oneshot_sf(soundfile("mySample",2), LAGRANGE_ORDER, sound_number, root_midi, freq, envVol, gate);
// polyphonic DSP code must declare a stereo effect
effect = _, _;
