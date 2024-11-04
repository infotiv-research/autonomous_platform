
#include <Wire.h>
#include <Adafruit_MCP4725.h>


Adafruit_MCP4725 dac;
Adafruit_MCP4725 dac1;

#define  pwm_output  13

const double DAC_res = 4096;
const double pin_res = 255;

const double dac_scaling = DAC_res/5;
const double dac_scaling1 = DAC_res/3.3;
const double pin_scaling = pin_res/5;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dac.begin(0x62);
  dac1.begin(0x63);
  //pinMode(pwm_output,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  double volt_set = 3.5; 
  double volt_set1 = 2;
  dac.setVoltage(volt_set*dac_scaling, false);
  dac1.setVoltage(volt_set1*dac_scaling1, false);
  //analogWrite(pwm_output,volt_set*pin_scaling);

  //Serial.println("d13 Pinout: " + String(volt_set*pin_scaling));
  delay(100);
}
