
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include "SteeringMotorInterface.h"

void reverse_engaged();
void set_speed(float set_speed);
void set_break();
String ReadCommand();
void ramp_down_speed();
void ramp_up_speed();


Adafruit_MCP4725 dac_acc;
Adafruit_MCP4725 dac_break;
KangarooControllInterface steering_motor;



int sensorPinBrake = A1;    // Brake pedal analog input PIN
int sensorPinAcc = A0;    // Acc analog input PIN

float curr_start = millis();

const float DAC_res = 4096;
const float pin_res = 1023;

const float dac_scaling_5v = DAC_res/5.26; // might be some offset in the 5V output pin
const float dac_scaling_3v = DAC_res/3.3; // ACC
const float analog_pin_scaling = 5/pin_res;

const float max_volt = 4.35;
const float min_volt = 0.85;
boolean toggle1 = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial3.begin(9600); //for steering motor
  while(!Serial3);
  dac_acc.begin(0x62);
  dac_break.begin(0x63);
  //pinMode(pwm_output,OUTPUT);
/*
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = ((16*10^6) / (1*1024) - 1);// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
*/
  /*
  initializse steering motor
  Parameters <motor command serial port> <port nr> <motor nr> <motor speed> <debug serial port>
  */
  Serial.println("Starting to init motor");
  if(steering_motor.init(Serial3, 1, 1, 150, 200, Serial)){
    Serial.println("Steering motor successfully initialized!");
  }
  else {
    Serial.println("Steering motor initialization ERROR!");
    Serial.println("ERROR=" + steering_motor.GetLastErrorMsg());
    //Todo blink error LED
    //Send error MSG on CAN buss
    //Enter endless loop
  }
  dac_break.setVoltage(min_volt*dac_scaling_5v, false);
  dac_acc.setVoltage(min_volt*dac_scaling_5v, false);
  Serial.println(steering_motor.GetSavedMotorMaxLimit());
  Serial.println("Steering max=" + String(steering_motor.GetSavedMotorMaxLimit()) + " min=" + String(steering_motor.GetSavedMotorMinLimit()));
  steering_motor.SetMotorAbsolutePosition(50);
  set_break();
  delay(3000);
  Serial.println("\n");
  Serial.println("init complete");

}

void loop() {
  // put your main code here, to run repeatedly:
  
  // Set Voltages
  //set_speed(2);
  String command = ReadCommand();
  //Serial.println(command);
  /*
  String command = "-1";
  int steer_val = 0;
  if (! (command_read==String(-1)))
  {
    int start_index = command_read.indexOf(" ");
    
    Serial.println(start_index);
    command = command_read.substring(0,start_index);
    Serial.println(command);
    String steer_str_val = command_read.substring(start_index,command_read.length());
    steer_val = steer_str_val.toInt();
    Serial.println(steer_val);
         
  }
 */
  if (command == "-1")
  {
    // nothing changed, continue
  } 
    else if (command == "speed")
      {
       // Serial.println("\n");
       Serial.println("ACCELERATE");
       // Serial.println("\n");
        set_speed(2);
      }
    else if (command == "break")
      {
       // Serial.println("\n");
       Serial.println("Break");
       // Serial.println("\n");
        set_break();
      }
    else if (command == "reverse")
      {
       // Serial.println("\n");
      Serial.println("reversing");
       // Serial.println("\n");
        reverse_engaged();
      }
      else if (command == "ramp_up")
      {
       // Serial.println("\n");
       Serial.println("ramp_up");
       // Serial.println("\n");
        ramp_up_speed();
      }
      else if (command == "ramp_down")
      {
       // Serial.println("\n");
       Serial.println("ramping_down");
       // Serial.println("\n");
        ramp_down_speed();
      }
      else if (command == "steer_left")
      {
        Serial.println("steer left");
        steering_motor.SetMotorAbsolutePosition(10);
      }
      else if (command == "steer_right")
      {
        Serial.println("steer rigth");
        steering_motor.SetMotorAbsolutePosition(90);
      }
    else
      {
        //Serial.println("\n");
       // Serial.println("Pls enter correct");
       // Serial.println("\n");
      }      
  //Serial.println("d13 Pinout: " + String(volt_set*pin_scaling));
  delay(200);
}

void reverse_engaged()
{

  set_break();
  //Serial.println(5*dac_scaling);
  delay(1000);
  

  // gokart stopped
  dac_break.setVoltage(max_volt*dac_scaling_5v, false);
  delay(100);

  dac_break.setVoltage(min_volt*dac_scaling_5v, false);
  delay(200);
  
  dac_break.setVoltage(max_volt*dac_scaling_5v, false);
  delay(100);

  dac_break.setVoltage(min_volt*dac_scaling_5v, false);

}

void set_speed(float set_speed)
{
  //Serial.println("HEJ");
  dac_break.setVoltage(min_volt*dac_scaling_5v, false);
  dac_acc.setVoltage(set_speed*dac_scaling_5v, false); 
}

void ramp_up_speed()
{    
  set_break();
for (int i = 0; i <= DAC_res ; i=i+100)
  {
    //Serial.println(i);
    dac_acc.setVoltage(i, false);
    delay(200);
  }  
}

void ramp_down_speed()
{    
  
for (int i = DAC_res; i >= 0.75/dac_scaling_5v*DAC_res ; i=i-100)
  {
    //Serial.println(i);
    dac_acc.setVoltage(i, false);
    delay(200);
  }  
}


void set_break()
{
  dac_break.setVoltage(max_volt*dac_scaling_5v, false);
  dac_acc.setVoltage(min_volt*dac_scaling_5v, false);
  delay(500);
  dac_break.setVoltage(min_volt*dac_scaling_5v, false);
}


String ReadCommand()  //reads eventuall command from serial1
{
String returnString = "-1";
//there is something to be read from the Serial1 line
 if (Serial.available() > 0) {
   
    //Serial.println("Reading Serial1 input!");   
    // https://www.arduino.cc/reference/en/language/functions/communication/serial/readstring/   
    //returnString = Serial1.readString(); //read until timeout 
    returnString = Serial.readStringUntil('\n'); 
    //read until newline character    returnString.trim(); // remove any \r and \n from the en of string  
    //.println("Read >" + returnString + "< from Serial1 input");    
     }
return returnString;}



/*

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
 //Serial.println("Voltage Readings at 1Hz"); 
 // Read Voltages
float brakeVal = analogRead(sensorPinBrake);
float brake_voltage = brakeVal *analog_pin_scaling;
float accVal = analogRead(sensorPinAcc);
float acc_voltage = accVal *analog_pin_scaling;
float curr_time = millis();
Serial.println("Current Pos: "+ String(steering_motor.ReadCurrentMotorEncoderPosition())  + "Steering motor Max=" + String(steering_motor.GetSavedMotorMaxLimit()) + ", Min=" + String(steering_motor.GetSavedMotorMinLimit()));  
Serial.println("Time (ms): "+String(curr_time-curr_start) + "\t\t Acc Volt (V): " +String(acc_voltage) + "\t\t Break volt (V): " + String(brake_voltage));

}
*/
