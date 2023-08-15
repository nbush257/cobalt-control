// This is a barebones script if you really need to debug the hardware
void setup() {
  Serial.begin(9600);
  pinMode(23,INPUT);
  pinMode(15,INPUT);
  pinMode(13,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  delayMicroseconds(2000);
  int ain = analogRead(23);
  int thresh = analogRead(15);
  Serial.print(ain);
  Serial.print(",");
  Serial.println(thresh);

  if (ain>thresh){
    digitalWrite(13,HIGH);
  }
  if (ain<thresh){
    digitalWrite(13,LOW);
  }
}
