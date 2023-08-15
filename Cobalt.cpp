#include "Cobalt.h"
#include "Arduino.h"

Cobalt::Cobalt(){
}


void Cobalt::begin() {
  analogWriteResolution(DAC_RESOLUTION); 
  pinMode(_pin_OUT,OUTPUT);
  pinMode(_AIN_PIN,INPUT);
  pinMode(_POTPIN,INPUT);
  if (MODE =='S'){analogWrite(_pin_OUT,BASE_VAL);}
  Serial.begin(9600);
}

void Cobalt::_turn_on_binary(float amp){
  // Turn on the light instantaneously at a given amplitude. Scaled between 0 and 1 V
  int digAmp = map(amp,0,1,0,DAC_RANGE/V_REF);
  analogWrite(_pin_OUT,digAmp);
}

void Cobalt::_turn_off_binary(){
  // Turn off the light instataneously.
  analogWrite(_pin_OUT,0);
}

void Cobalt::_turn_on_sigm(float amp){
  // Turn on the light with a sigmoidal ramp Scales the ramp between a base amplitude (which is a voltage just below where the laser is on) to 1v.
  // The amplitude parameter scales maximum ramp.
  
  float sigmoidalValue;
  float t;
  uint startTime = micros();
  while ((micros() - startTime) < SIGM_RISETIME*1000) {
    t = float(micros() - startTime) / (SIGM_RISETIME *1000);
    sigmoidalValue = 1 / (1 + exp(-10 * (t - 0.5))); // Sigmoidal function
    sigmoidalValue = map(sigmoidalValue*amp,0,1,BASE_VAL,DAC_RANGE/V_REF);
    analogWrite(_pin_OUT, int(sigmoidalValue));
  }
}

void Cobalt::_turn_off_sigm(float amp){
  float sigmoidalValue;
  float t;
  uint startTime = micros();
  while ((micros() - startTime) < SIGM_RISETIME*1000) {
    t = float(micros() - startTime) / (SIGM_RISETIME *1000);
    sigmoidalValue = 1-(1 / (1 + exp(-10 * (t - 0.5)))); // Sigmoidal function
    sigmoidalValue = map(sigmoidalValue*amp,0,1,BASE_VAL,DAC_RANGE/V_REF);
    analogWrite(_pin_OUT, int(sigmoidalValue));
  }
  analogWrite(_pin_OUT, BASE_VAL);
}

void Cobalt::_turn_on(float amp){
  // Overload turn on function. Can either be in binary or sigmoidal mode
  switch (MODE){
    case 'B':
      _turn_on_binary(amp);
      break;
    case 'S':
      _turn_on_sigm(amp);
      break;
    default:
      _turn_on_sigm(amp);
  }
}

void Cobalt::_turn_off(float amp){
  // Overload turn off function. Can either be in binary or sigmoidal mode
  switch (MODE){
    case 'B':
      _turn_off_binary();
      break;
    case 'S':
      _turn_off_sigm(amp);
      break;
    default:
      _turn_off_sigm(amp);
  }
}

void Cobalt::pulse(float amp,uint dur_ms){
  // Run a single pulse with amplitude "amp"
  _turn_on(amp);
  int t_pulse = millis();
  while ((millis()-t_pulse)<dur_ms){};
  delay(dur_ms);
  _turn_off(amp);
}
//TODO: Use the difference in onset times to trigger each stimulation
void Cobalt::train(float amp,float freq_hz,uint dur_pulse,uint dur_train){
  // Run a sequence of pulses at a given amplitude and frequency
  // Also known as a pulse train
  // freq_hz - frequeny of stimulation
  // dur_pulse - duration of each pulse in the train 
  // dur_train- duration of the train
  // It is up to the user to make sure that the pulse duration is not too long for the frequency, and that the pulse duration is not 
  // longer than the train. 
  //The code will make it so that the worst that happens is the light is on for the whole duration of the train.


  if (dur_pulse>dur_train){
    dur_pulse=dur_train-10;
  }
  int _temp_pulse = (int(1000.0/freq_hz) - dur_pulse);

  // Catch case in which the pulse duration is too long for the frequency
  if (_temp_pulse<0){
    _temp_pulse=0;
    dur_pulse=(int(1000.0/freq_hz));
  }
  uint _dur_interpulse = _temp_pulse-SIGM_RISETIME*2;//Account for sigmoidal rise time
  uint t_start_train = millis();
  // Add a 5 millisecond buffer to not shut off the train too early
  while ((millis()-t_start_train)<dur_train+5){
    //TODO: Make this a sigmoid ramp
    pulse(amp,dur_pulse);
    // Pause for the interpulse interval
    uint t_interpulse = millis();
    while ((millis()-t_interpulse)<_dur_interpulse){};
  }
}

void Cobalt::train_duty(float amp,float freq_hz, float duty, uint dur_train){
  // Run a sequence of pulses at a given frequency and duty cycle.
  // Also known as a pulse train
  // freq_hz - frequeny of stimulation
  // duty - percent of the cycle that the light should be on
  // dur_train - length of the train in ms

  if (duty>1){duty=1;}
  uint _dur_pulse = (1000.0/freq_hz * duty);
  uint _dur_interpulse = (1000.0/freq_hz *(1-duty))-SIGM_RISETIME*2; //Account for sigmoidal rise time
  
    uint t_start_train = millis();
  // Add a 5 millisecond buffer to not shut off the train too early
  while ((millis()-t_start_train)<dur_train+5){
  
    //TODO: Make this a sigmoid ramp
    pulse(amp,_dur_pulse);
    // Pause for the interpulse interval
    uint t_interpulse = millis();
    while ((millis()-t_interpulse)<_dur_interpulse){};
  }
}

void Cobalt::run_10ms_tagging(int n){
  // Run a standard tagging of 10 ms pulses at full amplitude
  //n - number of pulses. Default is 75
  for (int ii=0; ii<n+1; ii++){
    Serial.print("Running 10ms tag ");
    Serial.print(ii+1);
    Serial.print(" of ");
    Serial.println(n);
    pulse(1,10);
    delay(5000);
  }
}

void Cobalt::phasic_stim_exp(float amp,uint dur_active) {
  // Run phasic stimulations during expiration
  // amp - amplitude to stimulate with
  // dur_active - duration to run the stimulation for
  Serial.print("Stimulating during expiratory time for: ");
  Serial.print(dur_active/1000);
  Serial.println(" seconds");
  uint t_start = millis();
  while ((millis()-t_start)<=dur_active) {
    int dbncr = 0;
    bool laser_on= false;
    int ain_val = analogRead(_AIN_PIN);
    int thresh_val =  analogRead(_POTPIN);

    delay(2); // Sets the sampling rate. Lower is less noisy. (Default - 5ms/200Hz)

    // dbncr must be high/low for n samples before triggering/ending the stim
    int n = 4;
    if (ain_val > thresh_val) {
      if (dbncr < n) {
        dbncr++;
      }
      else if (laser_on) {
        _turn_off(amp);
        laser_on = false;
        dbncr = 0;
      }
      else {
        dbncr = 0;
      }
    }
    else {
      if (dbncr < n/2) {
        dbncr++;
      }
      else if (!laser_on) {
        _turn_on(amp);
        laser_on = true;
        dbncr = 0;
      }
      else {
        dbncr = 0;
      }
    }
  }
}

void Cobalt::phasic_stim_insp(float amp, uint dur_active) {
  // Run phasic stimulations during inspiration
  // amp - amplitude to stimulate with
  // dur_active - duration to run the sstimulation for
  Serial.print("Stimulating during inspiratory time for: ");
  Serial.print(dur_active/1000);
  Serial.println(" seconds");
  uint t_start = millis();
  while ((millis()-t_start)<=dur_active){
    int dbncr = 0;
    bool laser_on= false;
    int ain_val = analogRead(_AIN_PIN);
    int thresh_val =  analogRead(_POTPIN);
    delay(2); // Sets the sampling rate. Lower is less noisy. (Default - 5ms/200Hz)

    // dbncr must be high/low for n samples before triggering/ending the stim
    int n = 4;
    if (ain_val > thresh_val) {
      if (dbncr < n) {
        dbncr++;
      }
      else if (!laser_on) {
        _turn_on(amp);

        laser_on = true;
        dbncr = 0;
      }
      else {
        dbncr = 0;
      }
    }
    else {
      if (dbncr < n/2) {
        dbncr++;
      }
      else if (laser_on) {
        _turn_off(amp);
        laser_on = false;
        dbncr = 0;
      }
      else {
        dbncr = 0;
      }
    }
  }
}
