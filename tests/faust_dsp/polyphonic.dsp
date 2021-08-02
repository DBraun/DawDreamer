declare name "MyInstrument";

declare options "[nvoices:8]"; // FaustProcessor has a property which will override this.
import("stdfaust.lib");

freq = hslider("freq",200,50,1000,0.01); // note pitch
gain = hslider("gain",0.1,0,1,0.01);     // note velocity
gate = button("gate");                   // note on/off

myFilter = fi.lowpass(10, hslider("cutoff", 20000., 30., 20000., 0.1));
decay = hslider("decay", .1, 0.001, 10., 0.001);

envFilter = en.adsr(.002, decay, 0.0, .05, gate);
envVol = 0.5*en.adsr(.002, 0.1, 0.9, .1, gate);

process = os.sawtooth(freq)*gain*envVol : fi.lowpass(10, 500. + 10000.*envFilter) <: _, _;

// polyphonic DSP code must declare a stereo effect, even it is simply
// effect = _, _;
effect = myFilter, myFilter;