#include <Cobalt.h>
#include <Tbox.h>

Cobalt cobalt;
Tbox tbox;

void setup() {  
  
  cobalt.begin();
}

void loop() {
  while(Serial.available()){Serial.read();}
  Serial.println("Press enter to begin test");
  while(!Serial.available()){}
  
  cobalt.phasic_stim_insp(15, LASER_AMP, 10000, 10000)


}
