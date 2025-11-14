declare name "MyEffect";
myFilter= fi.lowpass(10, hslider("cutoff",  15000.,  20,  20000,  .01));
process = _, _, _, _ :> myFilter, myFilter;
