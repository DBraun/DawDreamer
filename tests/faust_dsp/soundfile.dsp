declare name "MyInstrument";

declare options "[nvoices:8]"; // FaustProcessor has a property which will override this.
import("stdfaust.lib");

// variation of hs_phasor that doesn't loop. It's like a one-shot trigger.
my_phasor(inc,c) = inc*on_memory : + ~ (_*(1-start_pulse))
with {
    is_on = c>0;
    
    start_pulse = is_on & (1-is_on');
    on_memory = is_on : max ~ (_*(1-start_pulse));
};

gain = hslider("gain",0.1,0,1,0.01);     // note velocity
gate = button("gate");                   // note on/off
key = hslider("freq", 60, 1, 127, 1) : ba.hz2midikey;
soundChoice = nentry("soundChoice", 0, 0, 2, 1);

root_midi = hslider("center_note", 60., 1., 128., 0.01);
semitones = key - root_midi;
ratio = semitones : ba.semi2ratio;

envVol = en.adsr(.002, 0.1, 0.9, .1, gate);

totalGain = gain * envVol * .5;

process = soundChoice,my_phasor(ratio, gate):soundfile("mySound",2):!,!,_,_ : _*totalGain, _*totalGain;
// polyphonic DSP code must declare a stereo effect
effect = _, _;