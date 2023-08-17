#include "Cobalt.h"
#include "Arduino.h"

Cobalt::Cobalt(){
}

void Cobalt::begin() {
  analogWriteResolution(DAC_RESOLUTION); 
  pinMode(LASER_PIN,OUTPUT);
  pinMode(AIN_PIN,INPUT);
  pinMode(POT_PIN,INPUT);
  if (MODE =='S'){analogWrite(LASER_PIN,BASE_VAL);}
  Serial.begin(9600);
}

void Cobalt::_turn_on_binary(float amp){
  // Turn on the light instantaneously at a given amplitude. Scaled between 0 and 1 V
  int digAmp = map(amp,0,1,0,DAC_RANGE/V_REF);
  analogWrite(LASER_PIN,digAmp);
}

void Cobalt::_turn_off_binary(){
  // Turn off the light instataneously.
  analogWrite(LASER_PIN,0);
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
    sigmoidalValue = map(sigmoidalValue,0,1,BASE_VAL,DAC_RANGE/V_REF*amp);
    analogWrite(LASER_PIN, int(sigmoidalValue));
  }
}

void Cobalt::_turn_off_sigm(float amp){
  float sigmoidalValue;
  float t;
  uint startTime = micros();
  while ((micros() - startTime) < SIGM_RISETIME*1000) {
    t = float(micros() - startTime) / (SIGM_RISETIME *1000);
    sigmoidalValue = 1-(1 / (1 + exp(-10 * (t - 0.5)))); // Sigmoidal function
    sigmoidalValue = map(sigmoidalValue,0,1,BASE_VAL,DAC_RANGE/V_REF*amp);
    analogWrite(LASER_PIN, int(sigmoidalValue));
  }
  analogWrite(LASER_PIN, BASE_VAL);
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
  int t_pulse_on = micros();
  while ((micros()-t_pulse_on)<(dur_ms*1000)){};
  _turn_off(amp);
}

void Cobalt::train(float amp,float freq_hz,uint dur_pulse,uint dur_train){
  // Run a sequence of pulses at a given amplitude and frequency
  // Also known as a pulse train
  // freq_hz - frequeny of stimulation
  // dur_pulse - duration of each pulse in the train 
  // dur_train- duration of the train
  // It is up to the user to make sure that the pulse duration is not too long for the frequency, and that the pulse duration is not 
  // longer than the train. 

  if (dur_pulse>dur_train){
    dur_pulse=dur_train-5;
  }
  uint full_duty_time = (1000.0/freq_hz)*1000; //in microseconds

  uint t_start_train = micros();
  Serial.print('\t');
  Serial.print(freq_hz);
  Serial.print(" Hz,");
  Serial.print(dur_pulse);
  Serial.print(" ms pulses, ");
  Serial.print(amp);
  Serial.print(" volts, ");
  Serial.print(dur_train/1000.);
  Serial.println("s train.");
  while ((micros()-t_start_train)<dur_train*1000){
    uint t_start_pulse = micros();
    pulse(amp,dur_pulse);
    while((micros()-t_start_pulse)<full_duty_time){}
  }
}

void Cobalt::train_duty(float amp,float freq_hz, float duty, uint dur_train){
  // Run a sequence of pulses at a given frequency and duty cycle.
  // Also known as a pulse train
  // freq_hz - frequeny of stimulation
  // duty - percent of the cycle that the light should be on
  // dur_train - length of the train in ms

  if (duty>1){duty=1;}
  uint dur_pulse = (1000.0/freq_hz * duty);
  uint full_duty_time = (1000.0/freq_hz)*1000; //in microseconds
  Serial.print('\t');
  Serial.print(freq_hz);
  Serial.print(" Hz,");
  Serial.print(duty*100);
  Serial.print("% duty cycle, ");
  Serial.print(dur_pulse);
  Serial.print(" ms pulses, ");
  Serial.print(amp);
  Serial.print(" volts, ");
  Serial.print(dur_train/1000.);
  Serial.println("s train.");
  uint t_start_train = micros();
  while ((micros()-t_start_train)<(dur_train*1000)){
    uint t_start_pulse = micros();
    pulse(amp,dur_pulse);
    while((micros()-t_start_pulse)<full_duty_time){}
  }
}

void Cobalt::run_10ms_tagging(int n){
  // Run a standard tagging of 10 ms pulses at full amplitude
  //n - number of pulses. Default is 75
  for (int ii=0; ii<n; ii++){
    Serial.print("Running 10ms tag ");
    Serial.print(ii+1);
    Serial.print(" of ");
    Serial.println(n);
    pulse(1,10);
    delay(5000);
  }
}

void Cobalt::run_multiple_pulses(int n, float amp, uint dur_pulse, uint IPI){
  // Run a sequence of pulses seperated by a fixed interval
  // Equivalent to a train, but easier to program for a lot of single pulses
  for (int ii=0; ii<n; ii++){
    Serial.print("Running ");
    Serial.print(dur_pulse);
    Serial.print(" ms pulse at ");
    Serial.print(amp);
    Serial.print(" volts: pulse ");
    Serial.print(ii+1);
    Serial.print(" of ");
    Serial.println(n);
    pulse(amp,dur_pulse);
    delay(IPI);
  }
}

void Cobalt::run_multiple_trains(int n, float amp, float freq_hz, uint dur_pulse, uint dur_train,uint intertrain_interval){
  for (int ii=0; ii<n; ii++){
    Serial.print("Running train ");
    Serial.print(ii+1);
    Serial.print(" of ");
    Serial.println(n);
    train(amp,freq_hz, dur_pulse, dur_train);
    Serial.print("\tWaiting ");
    Serial.print(float(intertrain_interval)/1000.0,1);
    Serial.println(" seconds...");
    delay(intertrain_interval);
  }
  Serial.println("Done with trains");

}


void Cobalt::phasic_stim_insp(uint n, float amp, uint dur_active,uint intertrial_interval){
    for (uint ii=0;ii<n;ii++){

  Serial.print("Stimulating during inspiratory time for: ");
  Serial.print(dur_active/1000);
  Serial.print(" seconds. ");
  Serial.print("Rep ");
  Serial.print(ii+1);
  Serial.print(" of ");
  Serial.println(n);

  bool laser_on=false;
  _turn_off(NULL_VOLTAGE);
  int ain_val = analogRead(AIN_PIN);
  int thresh_val =  analogRead(POT_PIN);
  int thresh_down = int(float(thresh_val)*0.9);
  
  uint t_start = millis();
  while ((millis()-t_start)<=dur_active){
    ain_val = analogRead(AIN_PIN);
    thresh_val =  analogRead(POT_PIN);
    thresh_down = int(float(thresh_val)*0.9);
    if ((ain_val>thresh_val) & !laser_on){
      _turn_on(amp);
      laser_on=true;
    }
    if ((ain_val<thresh_down) & laser_on){
      _turn_off(amp);
      laser_on=false;
    }
  }

  if (laser_on){_turn_off(amp);}
  delay(intertrial_interval);

}
}

void Cobalt::phasic_stim_exp(uint n, float amp, uint dur_active,uint intertrial_interval){
    for (uint ii=0;ii<n;ii++){

  Serial.print("Stimulating during expiratory time for: ");
  Serial.print(dur_active/1000);
  Serial.print(" seconds. ");
  Serial.print("Rep ");
  Serial.print(ii+1);
  Serial.print(" of ");
  Serial.println(n);
  _turn_off(NULL_VOLTAGE);
  bool laser_on=false;
    
  int ain_val = analogRead(AIN_PIN);
  int thresh_val =  analogRead(POT_PIN);
  int thresh_down = int(float(thresh_val)*0.9);
  
  uint t_start = millis();
  while ((millis()-t_start)<=dur_active){
    ain_val = analogRead(AIN_PIN);
    thresh_val =  analogRead(POT_PIN);
    thresh_down = int(float(thresh_val)*0.9);
    if ((ain_val>thresh_val) & laser_on){
      _turn_off(amp);
      laser_on=false;
    }
    if ((ain_val<thresh_down) & !laser_on){
      _turn_on(amp);
      laser_on=true;
    }
  }
  if (laser_on){_turn_off(amp);}
  delay(intertrial_interval);
}
}



void Cobalt::calibrate(){
  // Calibrate the laser power by running 5 second pulses 
  //with command voltages increasing from BASE_VAL to 1 in 0.1V steps

  //Serial input to start calibration or not
  uint timeout = 10000;
  while (Serial.available()>0){Serial.read();}
  Serial.print("Do you want to run calibration? (n) to skip. Timeout=");
  Serial.print(float(timeout)/1000.0,2);
  Serial.print("s ");
  uint timeout_counter = millis();
  char user_in='n';
  uint temp = millis();
  while((millis()-timeout_counter)<timeout){
    if ((millis()-temp)>1000){Serial.print(".");temp=millis();} //Little timer in serial
    if (Serial.available()){
      user_in = Serial.read();
      break;
    }
  }
  Serial.print('\n');
  
  // Skip calibration if n is input
  if (user_in=='n'){
    Serial.println("Not Calibrating.");
    return;
  }

  Serial.println("Calibrating");
  delay(1000);
  float amp = 0.1;
  while (amp<=1.05){
    Serial.print("\tRunning ");
    Serial.print(amp,4);
    Serial.print(" volts...");
    pulse(amp,5000);
    Serial.println("done");
    delay(5000);
    amp+=0.1;
  }
  Serial.println("Done calibrating");
}


void Cobalt::calibrate_high_res(){
  // Calibrate the laser power by running 5 second pulses 
  //with command voltages increasing from BASE_VAL to 1 in 0.1V steps

  Serial.println("Calibrating");
  delay(1000);
  float amp = 0.45;
  while (amp<=1.05){
    Serial.print("\tRunning ");
    Serial.print(amp);
    Serial.print(" volts...");
    pulse(amp,5000);
    Serial.println("done");
    delay(2000);
    amp+=0.025;
  }
  Serial.println("Done calibrating");
}


//TODO: Add phasic stimulation trains

void Cobalt::phasic_stim_exp_train(uint n, float amp, float freq_hz, uint dur_ms, uint dur_active,uint intertrial_interval){
  for (uint ii=0;ii<n;ii++){
  Serial.print("Stimulating during expiratory time train for: ");
  Serial.print(dur_active/1000);
  Serial.print(" seconds. Freq ");
  Serial.print(freq_hz);
  Serial.print("Rep ");
  Serial.print(ii+1);
  Serial.print(" of ");
  Serial.println(n);
  _turn_off(NULL_VOLTAGE);
  bool is_insp=false;
    
  int ain_val = analogRead(AIN_PIN);
  int thresh_val =  analogRead(POT_PIN);
  int thresh_down = int(float(thresh_val)*0.9);
  uint full_duty_time = (1000.0/freq_hz)*1000; //in microseconds

  uint last_stim_on = micros();
  uint t_start = millis();
  while ((millis()-t_start)<=dur_active){
    ain_val = analogRead(AIN_PIN);
    thresh_val =  analogRead(POT_PIN);
    thresh_down = int(float(thresh_val)*0.9);

    if ((ain_val>thresh_val)){

      is_insp = true;
    }
    if ((ain_val<thresh_down)){

      is_insp = false;
    }
    if (!is_insp){
      if ((micros() - last_stim_on)>full_duty_time){
        last_stim_on = micros();
        pulse(amp,dur_ms);
        
      }
    }

  }
  delay(intertrial_interval);
  }
}

// TODO: add phasic calibration time
//TODO: add standard battery of tags, amplitude sweeps?