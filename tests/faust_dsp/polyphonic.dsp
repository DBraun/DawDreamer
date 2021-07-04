declare name "MyEffect";

declare options "[nvoices:8]";
import("stdfaust.lib");

freq = hslider("freq",200,50,1000,0.01);
gain = hslider("gain",0.1,0,1,0.01);
gate = button("gate");

envFilter = en.adsr(.002, 0.1, 0.0, .05, gate);
envVol = 0.5*en.adsr(.002, 0.1, 0.9, .1, gate);

process = os.sawtooth(freq)*gain*envVol : fi.lowpass(10, 500. + 10000.*envFilter) <: _, _;
effect = _, _;