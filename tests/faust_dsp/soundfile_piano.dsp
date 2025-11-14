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
key = hslider("freq", 60, 1, 127, 1) : ba.hz2midikey : _ , -21 : +;
// note that A0 is midi note 21, so we subtract 21 to get to 0 (the first file is named 0.wav)
release = hslider("release",0.1,0.,2.,0.001);    // note release in seconds
envVol = en.adsr(0., 0., 1., release, gate);
safeKey = key : min(87) : max(0);
totalGain = gain * envVol * .5;

process = safeKey,my_phasor(1., gate):soundfile("mySound",2):!,!,_,_ : _*totalGain, _*totalGain;
// polyphonic DSP code must declare a stereo effect
effect = _, _;
