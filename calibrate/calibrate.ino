#include <Cobalt.h>
#include <Tbox.h>

Cobalt cobalt;
Tbox tbox;

void setup() {  
  tbox.begin();
  cobalt.begin();
}

void loop() {
  tbox.user_wait();
  cobalt.calibrate_high_res();

}
