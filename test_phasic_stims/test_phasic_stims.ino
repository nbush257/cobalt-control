// Run this to calibrate the phasic amplitudes
#include <Cobalt.h>

Cobalt cobalt;
void setup() {
  cobalt.begin();
}

void loop() {
  cobalt.phasic_stim_exp(0.8,10000);
  delay(5000);
  cobalt.phasic_stim_insp(0.8,10000);
  delay(5000);
}
