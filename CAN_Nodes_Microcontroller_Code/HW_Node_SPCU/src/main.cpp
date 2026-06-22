
/*
Here is a TLDR description of the HW node

HW_NODE name: Steering and Propulsion Control Unit (SPCU)
Hardware: STM32 Bluepill FC103xx

Purpose of this node:
example:
- Control of steering Motor
- Control of propulsion

Sensor Inputs:
- Encoder readings steering angle
- 

Actuator outputs:
- Serial commands for driver module
- Reference Value for Propulsions Digital to Analog Converters
*/

/*
              Changelog:
_____________________________________________
|   NAME    |   DATE    |       Changes Made        |
_________________________________________________________
|  Fredrik |   27/4-23  |   Base Code for AP4             |
|  Erik    |   25/8-23  |   Removed unnessary comments            |

*/


/*
================================================
Include Arduino libraries to use in project

HOW TO INCLUDE NEW LIBRARIES: Add library in platformio.ini file under lib_deps and they will be downloaded automatically during compiling
This can be done manually or through platformio's library manager.
*/
#include <Arduino.h> // Must Be Included since the PlattformIO uses Arduino Framework

/* Include config files */
#include "pin_config.h"
/*
=================================================
Include custom Controller Area Network (CAN) Database for Autonomous Platform Generation 4.
Make sure this platformio project is located in the CAN_Nodes_Microcontroller_Code directory in order for the relative path to work

Module Libraries are located in: SHARED_HW_Node_Libraries
adding paths to each one is done in platformio.ini
*/
#include "AP4_CAN.h"
#include "PropulsionInterface.h"
#include <SteeringMotorInterface.h>


/* Define and initialie global variables */
KangarooControllInterface steering_motor;
DAC_controller_Interface propulsion_controller;
bool set_SW_filter = false;
AP4_CAN can_interface(CAN_CS_PIN, set_SW_filter);

bool Heartbeat_frame_recieved; 
uint64_t heartbeat_val;
uint64_t heartbeat_val_new;

uint16_t ThrottleVoltage;
uint16_t BreakVoltage;

int8_t steer_pos;
int8_t current_pos;

uint32_t t; 
uint32_t t_i1;
uint32_t t_i2;
int32_t current_encoder_ticks;
bool interupt_flag = false;

bool recieved_interrupt = false;
/* Include node specific library dependencies */
// From the lib folder within each project
#include <SPCU_CAN_SETUP.h>


// Interuptfunction declaration 
void AP4CANInterrupt();


// ---------------------------- CODE: ------------------------------

/*
Arduino setup function or also called init
*/
void setup() {
  // note that the void loop() does not have scope to read variables declared within the setup()
  
  //Serial is used for debugging purposes, USB-micro connection cable of the STM32 Bluepill
  //if (SerialUSB){
    Serial.begin(9600);
    //uint32_t t = getCurrentMillis();
    bool break_while=true;
    uint32_t t = getCurrentMillis();
    while(!Serial && break_while)
    {
      uint32_t dt=getCurrentMillis()-t;
      if(dt > (u_int32_t)3000)
      {
        break_while=false;
      }
    }
  Serial.println("Start of Kangaroo setup");
  //Serial3 is used for sending commands to the KANGAROO X2 (drive module for steering modtor)
  // Serial3 is coded in the arduino object file and is there hardcoded to I/O pin 10 and 11 of the BluePill HW.
  // The definitions can be found in the arduino.h file.
  Serial3.begin(9600);
  while(!Serial3);
  
  if(propulsion_controller.DAC_init(DAC_Vin,DAC_offset,Serial))
  {
    Serial.println("Propulsion Controll sucessfully initialized");

    encode_can_0x3e8_Act_ThrottleVoltage(&can_interface.can_storage_container,DAC_MIN_OUT_VOLTAGE*CAN_Propulsion_Scale_Up);
    decode_can_0x3e8_Act_ThrottleVoltage(&can_interface.can_storage_container,&ThrottleVoltage);
    propulsion_controller.send_voltage("dac_acc",(float)ThrottleVoltage*(float)CAN_Propulsion_Scale_Down);


    encode_can_0x3e8_Act_BreakVoltage(&can_interface.can_storage_container,DAC_MIN_OUT_VOLTAGE*CAN_Propulsion_Scale_Up);
    decode_can_0x3e8_Act_BreakVoltage(&can_interface.can_storage_container,&BreakVoltage);
    propulsion_controller.send_voltage("dac_break",(float)BreakVoltage*(float)CAN_Propulsion_Scale_Down);
    }
  else{
    Serial.println("Propulsion Controll failed!!");
    }
   Serial.println("Passed Propulsion setup");
   
   int iteration = 0;
   bool try_steering_init = true;
   // kangarro might encounter synchornisation problem during initialization, thereby try until establish serial communiation
   while(try_steering_init && iteration < 5)
   {
    if(steering_motor.init(Serial3, InputPortNr, InputMotorNr, InputMotorFixedSpeed, InputWaitForResponseTime, Serial)){
       Serial.println("Steering motor successfully initialized!");
       try_steering_init = false;
       steering_motor.GetLastErrorMsg() ="";
     }
     else {
       Serial.println("Steering motor initialization ERROR!");
       Serial.println("Internal error msg=" + steering_motor.GetLastErrorMsg());
     }
     iteration = iteration+1;
     Serial.println("steering try: " + String(iteration) + "\n");
   }

  // WARNING!!! This should only be necessary when there are two nodes in the system. remove later
  Serial.println("waiting 1 minute for raspberry pi to start!!!");
  //delay(60000);

  // Set up of CAN buss
  can_interface.InitMCP2515();
  int nr_can_bus_init_attempts = 0;
  int max_attempts = 500;

  while(can_interface.InitMCP2515() == false && nr_can_bus_init_attempts < max_attempts)
  {
    Serial.println("  INIT OF CANBUSS FAILED! Waiting 1 second and retrying");
    nr_can_bus_init_attempts++;
    delay(1000);
  }


  attachInterrupt(digitalPinToInterrupt(CAN_INTERRUPT_PIN), AP4CANInterrupt, LOW);

  Serial.println("Set up of CAN interface complete!");

  /*
  Configure what CAN signals should be sent (by ID) and in what Periodic intervall (ms) 
  //can_interface.AddPeriodicCanSend(CAN_ID_FRAMENAME, 5000); See CAN_DB.h for frame names
  */
  can_interface.AddPeriodicCanSend(CAN_ID_GET_SPCU,100);
  
  /*
  Configure what CAN signals should be read from the CAN bus (by Id), any other message ID will be ignored!!
  */

  //can_interface.AddCANSWFilter(CAN_ID_FRAMENAME);


  pinMode(LED_BUILTIN, OUTPUT); //internal LED will light up!
  Serial.println("Init of ECU is Complete!");
  Serial.println("");
  t = getCurrentMillis();
  // Engage reverse
  propulsion_controller.send_voltage("dac_break",4);  
  delay(100);
  propulsion_controller.send_voltage("dac_break",DAC_MIN_OUT_VOLTAGE);
  delay(200);
  propulsion_controller.send_voltage("dac_break",4);  
  delay(300);
  propulsion_controller.send_voltage("dac_break",DAC_MIN_OUT_VOLTAGE);
  delay(1000);
  // disengage reverse
  propulsion_controller.send_voltage("dac_break",4);  
  delay(100);
  propulsion_controller.send_voltage("dac_break",DAC_MIN_OUT_VOLTAGE);
  delay(100);
  propulsion_controller.send_voltage("dac_break",4);  
  delay(100);
  propulsion_controller.send_voltage("dac_break",DAC_MIN_OUT_VOLTAGE);
}


/*
"WHILE True-loop"
*/
void loop() {
  //Serial.println("Start of main while loop");
  
  uint32_t t_time = getCurrentMillis();

  // the loop function runs over and over again forever
  if(interupt_flag){
  //  Serial.println("hello interrupt?");
    interupt_flag=false;
  }
  //Serial.println("After interrupt flag if");
  can_interface.PeriodicCanSend();
  
  if(recieved_interrupt)
  {
    //decode_can_0x064_Sig_Req_Heartbeat(&can_interface.can_storage_container,&heartbeat_val);
  
  // GET CAN MESSAGE
  can_interface.InterruptReadCAN();

  }

  // Check if error has occured!
  String error_steer = steering_motor.GetLastErrorMsg();
  //String error_prop = propulsion_controller.get_last_error();

  if(!error_steer.equals("no error saved")){
    Serial.println("Error in steering");
    encode_can_0x1f4_Steering_Error(&can_interface.can_storage_container,1);
    can_interface.SendSingleCanFrame(CAN_ID_ERROR_SPCU);
    }  
  if(!propulsion_controller.get_error_in_dac())
    {
    encode_can_0x1f4_Propulsion_Error(&can_interface.can_storage_container,1);
    can_interface.SendSingleCanFrame(CAN_ID_ERROR_SPCU);
    }

  // Steering Control: 23.9 is the calculated avg maximum steering angle for the approximated bicycle model of the ackermann steering
  decode_can_0x3e8_Act_SteeringPosition(&can_interface.can_storage_container,&steer_pos); //gets stored val
  steering_motor.SetMotorAbsolutePosition(int(map(steer_pos,-22,22,steering_motor.GetSavedMotorMaxLimit(),steering_motor.GetSavedMotorMinLimit())));
  
  // Steering feedback 
  current_encoder_ticks = steering_motor.ReadCurrentMotorEncoderPosition();
  current_pos = map(current_encoder_ticks, steering_motor.GetSavedMotorMinLimit(), steering_motor.GetSavedMotorMaxLimit(), 22,-22);
  encode_can_0x7d0_Get_SteeringAngle(&can_interface.can_storage_container,current_pos);

  // Propulsion Control
  decode_can_0x3e8_Act_ThrottleVoltage(&can_interface.can_storage_container,&ThrottleVoltage);
  float temp = (float) ThrottleVoltage*(float)CAN_Propulsion_Scale_Down;
  propulsion_controller.send_voltage("dac_acc",temp);

  decode_can_0x3e8_Act_BreakVoltage(&can_interface.can_storage_container,&BreakVoltage);
  float tempb = (float) BreakVoltage*(float)CAN_Propulsion_Scale_Down;
  propulsion_controller.send_voltage("dac_break",tempb);
  
  uint32_t time_print = getCurrentMillis()-t_time;
  //Serial.println("ABsolute time" + String(getCurrentMillis()) +"Loop time: " + String(time_print));

  
  /* DEMO CODE BELOW
  Dont forget to comment out steering control and propulsion control above!!!
  propulsion_controller.send_voltage("dac_acc",3);
  
  if (getCurrentMillis()-t> (uint32_t) 5000 && getCurrentMillis()-t < (uint32_t) 5500 ){
    steering_motor.SetMotorAbsolutePosition(int(map(20,-24,24,steering_motor.GetSavedMotorMaxLimit(),steering_motor.GetSavedMotorMinLimit())));
  }
  if (getCurrentMillis()-t> (uint32_t) 10000)
  {
    steering_motor.SetMotorAbsolutePosition(int(map(-20,-24,24,steering_motor.GetSavedMotorMaxLimit(),steering_motor.GetSavedMotorMinLimit())));
    t=getCurrentMillis();
    Serial.println(String(t));
  }
  
  */

  //Serial.println("Last line of while loop");
  
} //END OF WHILE

void AP4CANInterrupt() 
{
  recieved_interrupt = true;
  Serial.println("in CAn int loop");
   
}
