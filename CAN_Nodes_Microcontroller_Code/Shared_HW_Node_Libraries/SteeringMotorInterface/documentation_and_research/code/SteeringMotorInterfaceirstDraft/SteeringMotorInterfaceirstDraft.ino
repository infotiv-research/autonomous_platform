/*
Autonomous Platform Generation 4 creating first iteration of Kangaroo x2 controll board Steering Interface
23/2-2023

The goal is to create ready to use functions to easilly access kangaroo x2 functionality

*/

#include "KangarooSimplifiedSerialInterface.h"
//KangarooControllInterface *steeringmotor; 



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(1000);
  

  KangarooControllInterface steeringmotor(Serial1, 1, 1, 300, 50, Serial);
  int minLimit = steeringmotor.GetSavedMotorMinLimit();
  int maxLimit = steeringmotor.GetSavedMotorMaxLimit();
  Serial.println("Min Limit:" + String(minLimit) +", Max Limit:" + String(maxLimit));
  steeringmotor.SetMotorAbsolutePosition(minLimit);
  delay(5000);
  steeringmotor.SetMotorAbsolutePosition(maxLimit);
  delay(5000);
  steeringmotor.SetMotorAbsolutePosition(0);

  String startuperror = steeringmotor.GetLastErrorMsg();
  Serial.println("encountered error:" + startuperror);



  Serial.println("Finnished setup");
}

void loop() {
  // put your main code here, to run repeatedly:

  

}
