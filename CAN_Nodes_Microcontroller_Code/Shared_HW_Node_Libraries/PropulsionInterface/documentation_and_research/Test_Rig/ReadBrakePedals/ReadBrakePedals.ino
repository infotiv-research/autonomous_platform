/*
  Analog Input

  Demonstrates analog input by reading an analog sensor on analog pin 0 and
  turning on and off a light emitting diode(LED) connected to digital pin 13.
  The amount of time the LED will be on and off depends on the value obtained
  by analogRead().

  The circuit:
  - potentiometer
    center pin of the potentiometer to the analog input 0
    one side pin (either one) to ground
    the other side pin to +5V
  - LED
    anode (long leg) attached to digital output 13 through 220 ohm resistor
    cathode (short leg) attached to ground

  - Note: because most Arduinos have a built-in LED attached to pin 13 on the
    board, the LED is optional.

  created by David Cuartielles
  modified 30 Aug 2011
  By Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/AnalogInput
*/

int sensorPinBrake = A0;    // Acc pedal analog input PIN
int sensorPinAcc = A1;    // Brake analog input PIN
int ledPin = 13;      // select the pin for the LED
float curr_start = millis();
void setup() {
  Serial.println("");
  
  Serial.println("");
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  //Serial.println("Delay 3 secs");
  //delay(3000);
  Serial.println("Starting Measurement");
  Serial.println("Acc,Brake,Time (ms)");
}

void loop() {

  //Read voltage from brake pedals (when wired in parellell)

  // read the value from the sensor:
  int brakeVal = analogRead(sensorPinBrake);
  int accVal = analogRead(sensorPinAcc);
  float curr_time = millis();
  float brakeVoltage = brakeVal * (5.0 / 1023.0);  // store current brake value
  float accVoltage = accVal * (5.0 / 1023.0); //store current acc value

  //print to serial window
  
  //Serial.print("Accelator_voltage:");
  //Serial.print(accVoltage);
  
  Serial.println(String(curr_time-curr_start) + "," +String(accVoltage) + "," + String(brakeVoltage));
  //Serial.print(",Brake_voltage:");
  //Serial.println(brakeVoltage);
  delay(200);

 //write analog voltage
 /*float accVal = 4; //value between 0.8 and 4.35 v
 //map(brakeVal, 0.8, 4.3, 0, 255); //mappar 0.8 - 4.3 -> 0 - 255
 analogWrite(sensorPinBrake, accVal * 255 / 4.35 );

 float brakeVal = 2; //value between 0.8 and 4.35 v
 //map(brakeVal, 0.8, 4.3, 0, 255); //mappar 0.8 - 4.3 -> 0 - 255
 analogWrite(sensorPinBrake, brakeVal * 255 / 4.35);

 Serial.print("Writing_acc_voltage:");
 Serial.print(accVal* 255 / 4.35);
 Serial.print(",");
 Serial.print("Writing_brake_voltage:");
 Serial.println(brakeVal* 255 / 4.35);
 
 
  
  // turn the ledPin on
  //digitalWrite(ledPin, HIGH);
  // stop the program for <sensorValue> milliseconds:
  //delay(300);
  // turn the ledPin off:
  //digitalWrite(ledPin, LOW);
  // stop the program for for <sensorValue> milliseconds:
  delay(100);
  */
}
