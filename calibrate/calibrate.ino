#include <Cobalt.h>
#include <Tbox.h>

Cobalt cobalt;
Tbox tbox;
LASER_AMP = 0.75; //This should map to about 50-10mW at fiber tip.

void setup() {  
  
  cobalt.begin();
}

void loop() {
  while(Serial.available()){Serial.read();}
  Serial.println("Press enter to begin test");
  while(!Serial.available()){}
  
  cobalt.phasic_stim_insp(15, LASER_AMP, 10000, 10000)


}
