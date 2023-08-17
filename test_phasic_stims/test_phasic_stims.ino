// Run this to calibrate the phasic amplitudes
#include <Cobalt.h>
#include <Tbox.h>
Cobalt cobalt;
Tbox tbox;
void setup() {
  cobalt.begin();
  tbox.begin();
  tbox.attachDefaults();
}

void loop() {
  cobalt.phasic_stim_exp(1,0.8,10000,10000);
  cobalt.phasic_stim_insp(1,0.8,10000,10000);
  
}
