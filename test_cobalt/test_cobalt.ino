#include <Cobalt.h>

Cobalt cobalt;

void setup() {  
  // cobalt.MODE='S';
  cobalt.begin();
}

void loop() {
  // delay(2000);
  // cobalt.calibrate();
  // delay(1000);

  // Serial.println("Testing one pulse");
  // cobalt.pulse(1,5000);
  // delay(5000);

  // Serial.println("Testing one train");
  // cobalt.train(0.6,10,50,2000);
  // delay(2000);

  // Serial.println("Testing one train, duty");
  // cobalt.train_duty(0.9,10,0.5,3000);
  // delay(2000);

  // Serial.println("Testing tagging");
  // cobalt.run_10ms_tagging(5);
  // delay(2000);

  Serial.println("Testing multiple pulses...");
  cobalt.run_multiple_pulses(5,0.8,100,500);
  delay(2000);

  Serial.println("Testing multiple trains...");
  cobalt.run_multiple_trains(5,1.0,15,10,2000,5000);
  delay(2000);
  
  // Serial.println("Testing phasic stims");
  // cobalt.phasic_stim_exp(0.8,10000);
  // cobalt.phasic_stim_insp(0.8,10000);
  // delay(10000);


}
