#include "KangarooSimplifiedSerialInterface.h"
#include <Arduino.h>  //gives access to default arduino library functions, i.e millis()

//init func
KangarooControllInterface::KangarooControllInterface(Stream &inputCmdPort, int inputPortNr, int inputMotorNr, int inputMotorFixedSpeed, int inputWaitForResponseTime) {
  command_serial_port = &inputCmdPort;
  port_nr = inputPortNr;
  motor_nr = inputMotorNr;
  motorFixedSpeed = inputMotorFixedSpeed;
  wait_for_response_time = inputWaitForResponseTime;
  last_error = "no error saved";

  PerformKangarooStartupProcedure();

  motor_max_limit = ReadMotorEncoderMaxLimit();
  motor_min_limit = ReadMotorEncoderMinLimit();
  
}

KangarooControllInterface::KangarooControllInterface(Stream &inputCmdPort, int inputPortNr, int inputMotorNr, int inputMotorFixedSpeed, int inputWaitForResponseTime, Stream &inputDebugPort) {
  command_serial_port = &inputCmdPort;  //&Serial1; works
  port_nr = inputPortNr;
  motor_nr = inputMotorNr;
  motorFixedSpeed = inputMotorFixedSpeed;
  wait_for_response_time = inputWaitForResponseTime;
  debug_serial_port = &inputDebugPort; //&Serial; works  ;
  last_error = "no error saved";


  PerformKangarooStartupProcedure();
 
  motor_max_limit = ReadMotorEncoderMaxLimit();
  motor_min_limit = ReadMotorEncoderMinLimit();

}

//set functions
bool KangarooControllInterface::SetSerialPort(Stream &inputPort) {
  command_serial_port = &inputPort;
  return true;
}
bool KangarooControllInterface::SetWaitResponseTime(int inputTime) {
  wait_for_response_time = inputTime;
  return true;
}
bool KangarooControllInterface::SetSerialPortNr(int inputSerialPortNr) {
  port_nr = inputSerialPortNr;
  return true;
}
bool KangarooControllInterface::SetMotorNr(int inputMotorNr) {
  motor_nr = inputMotorNr;
  return true;
}
bool KangarooControllInterface::SetMotorFixedMovementSpeed(int inputMotorFixedSpeed) {
  motorFixedSpeed = inputMotorFixedSpeed;
  return true;
}

//get functions
Stream* KangarooControllInterface::GetSerialPort() {
  return command_serial_port;
 
}
int KangarooControllInterface::GetWaitResponseTime() {
  return wait_for_response_time;
}
int KangarooControllInterface::GetSerialPortNr() {
  return port_nr;
}
int KangarooControllInterface::GetMotorNr() {
  return motor_nr;
}
bool KangarooControllInterface::GetMotorFixedMovementSpeed() {
  return motorFixedSpeed;
}

/*
Returns the saved motor max limit
*/
int  KangarooControllInterface::GetSavedMotorMaxLimit() {
  return motor_max_limit;
}
/*
Returns the sasved motor min limit
*/
int  KangarooControllInterface::GetSavedMotorMinLimit() {
  return motor_min_limit;
}
//Read Motor Information Variables

/*
Reads the controller internal motor encoder position and returns it as an integer
*/
int KangarooControllInterface::ReadCurrentMotorEncoderPosition() {
  String cmd = String(motor_nr) + ",getp\r\n";

  String response = SendSimplifiedSerialCommandWaitForResponse(cmd);

  //if something valid has been read
  if (response != "\0")
  {
    //response should look something like 1,pxxx or 1,Pxxx
    int value = response.substring(3, response.length()).toInt();
    return value;
  }

  return -32768; //max val of integer default return value
}
/*
Function to retrieve the internal stored encoder value for maximum limit of motor
*/
int KangarooControllInterface::ReadMotorEncoderMaxLimit() {
  String cmd = String(motor_nr) + ",getmax\r\n";

  String response = SendSimplifiedSerialCommandWaitForResponse(cmd);

  //if something valid has been read
  if (response != "\0")
  {
    //response should look something like 1,pxxx or 1,Pxxx
    int value = response.substring(3, response.length()).toInt();
    return value;
  }

  return -32768; //max val of integer default return value
}
/*
Function to retrieve the internal stored encoder value for minimum limit of motor
*/
int KangarooControllInterface::ReadMotorEncoderMinLimit() {
  String cmd = String(motor_nr) + ",getmin\r\n";

  String response = SendSimplifiedSerialCommandWaitForResponse(cmd);

  //if something valid has been read
  if (response != "\0")
  {
    //response should look something like 1,pxxx or 1,Pxxx
    int value = response.substring(3, response.length()).toInt();
    return value;
  }

  return -32768; //max val of integer default return value
}
bool KangarooControllInterface::HasReachedTargetPosition() {
  String cmd = String(motor_nr) + ",getp\r\n";

  String response = SendSimplifiedSerialCommandWaitForResponse(cmd);

  //if something valid has been read
  if (response != "\0")
  {
    //response should look something like 1,pxxx or 1,Pxxx
    String cmd = response.substring(4,5);
    //A capital P means the motor has reached its goal position
    if(cmd == "P"){
      return true;
    }
    
  }

  return false;
}

/*
Function to retrieve any error messages that the kangaroo has stumbled upon.
It will only give the latest error message.

*/
String KangarooControllInterface::GetLastErrorMsg()  //returns 1 to 6.
{
  return last_error;
}

/*
=================================================================================================================================================
Set Motor Variables and commands
*/


/*
Sets the absolute motor position (not relative)
Example: 1,p50

From Manual:
Position command. The motor will go to the
specified position, in units

(Units can be specified manually as described in function down below)
*/
bool KangarooControllInterface::SetMotorAbsolutePosition(int inputMotorPosition) {
  String cmdString = String(motor_nr) + ",p" + String(inputMotorPosition) + "s" + String(motorFixedSpeed);
  SendSimplifiedSerialCommand(cmdString);
  return true;
}
/*
Sets the motor position incrementally according to the manual specificattions
example: 1,pi100

From Manual:
Incremental position command. The motor
will go the specified increment from its
current position.

*/
bool KangarooControllInterface::SetMotorIncrementPosition(int inputIncrementPosition) {
  String cmdString = String(motor_nr) + ",pi" + String(inputIncrementPosition);
  SendSimplifiedSerialCommand(cmdString);
  return true;
}
/*
Sets the motor absolute speed according to the manual specifications
Example: 2,s-400

From Manual:
Speed command. The motor will go at the
specified speed, in units per second

*/
bool KangarooControllInterface::SetMotorAbsoluteSpeed(int inputMotorSpeed) {
  String cmdString = String(motor_nr) + ",s" + String(inputMotorSpeed);
  SendSimplifiedSerialCommand(cmdString);
  return true;
}
/*
Sets the kangaroo x2 motor increment speeds according to specifications
example: 1,si50

From manual: 
Incremental speed command. The motor will
go faster or slower than its current speed by
the commanded increment. 

*/
bool KangarooControllInterface::SetMotorIncrementSpeed(int inputIncrementSpeed) {
  String cmdString = String(motor_nr) + ",si" + String(inputIncrementSpeed);
  SendSimplifiedSerialCommand(cmdString);
  return true;
}
/*
Sets the kangaroo x2 motor units accordding to specifications in kangaroo x2 manual
example: 1,units 1 rotaion = 100 lines

From manual:
This command is used to change the input
from machine units (millivolts or encoder lines)
to a user defined unit system. Please note that
commands use whole numbers only, and
design your program to use appropriate units.
For example, for a system with four inches of
travel, thousandths of an inch is more
appropriate than inches. The first value is the
user units, then an equals sign, followed by the
second value in the machine units, followed by
a newline. The system only acts on the
numbers, the text strings are optional and for
code clarity

*/
bool KangarooControllInterface::SetMotorUnits(int unit1, int unit2) {
  String cmdString = String(motor_nr) + ",units " + String(unit1) + " = " + String(unit2);
  SendSimplifiedSerialCommand(cmdString);
  return true;
}


//private functions

/*
Low level function to send a simplified serial command to the kangaroo x2 controller board. 
It assumes the inputCmd is a valid command according to the kangaroo x2 simplified serial standard (see internal documentations AP4 for a list).
*/
bool KangarooControllInterface::SendSimplifiedSerialCommand(String inputCmd) {
  int msg_length = inputCmd.length();

  //pad the input command with newline and return line if the inputCmd lacks those
  if (inputCmd[msg_length - 1] != '\n' && inputCmd[msg_length - 2] != '\r') {

    inputCmd += '\r';
    inputCmd += '\n';
    msg_length = inputCmd.length();
  }


  //could possibly be done faster, if speed is an issue later on https://forum.arduino.cc/t/serial-write-to-send-a-string/87226/10
  //send inputcommand string over serial port, character by character
  for (int i = 0; i < msg_length; i++) {
    command_serial_port->write(inputCmd[i]);
  }

  //returns true when it has finnished sending the simplified serial command
  return true;
}

/*
Low level function to read from the serial data stream if there are any characters aviblle to read
*/
String KangarooControllInterface::ReadSimplifiedSerialCommand() {
  String returnString = "\0";

  //there is something to be read from the Serial1 line
  if (command_serial_port->available() >= 1) {
    // https://www.arduino.cc/reference/en/language/functions/communication/serial/readstring/
    //returnString = Serial1.readString(); //read until timeout
    returnString = command_serial_port->readStringUntil('\n');  //read until newline character
    returnString.trim();                                       // remove any \r and \n from the en of string
  }
  return ParseReadCommand(returnString);
}

/*
Sends a simplified serial command, waits for a set amount of time and reads the response.
The function returns the simplified serial response
*/
String KangarooControllInterface::SendSimplifiedSerialCommandWaitForResponse(String inputCmd) {
  String returnString = "\0";  //temp value, known return value if nothing useful was returned

  SendSimplifiedSerialCommand(inputCmd);  //sends command over Serial1

  //compare starttime - endtime
  long int startTime = millis();
  long int endTime = 0;
  bool waitloop = true;

  while (waitloop) {
    endTime = millis();

    if (endTime - startTime >= wait_for_response_time) {
      waitloop = false;
      break;
    }

    String read_val_serial = ReadSimplifiedSerialCommand();  //Tries to read line from serial_port communication line

    if (read_val_serial != "\0") {
      returnString = read_val_serial;
      waitloop = false;
      break;
    }
  }

  return returnString;
}

/*
A function used to parse any command read from the kangaroo board

if inputCmd contains any error msgs, indicated by a capital E, this is bad and should be caught by this function

the cmd should be in the format: 1,Axxx\r\n or 1,axxx\r\n.
where A/a is replaced by a valid command response character.
Valid command response characters are: (according to the kangaroo x2 simplified serial manual )
P - Absolute motor position, motor has reached target position
p - Absolute motor position, motor has NOT reached target position
S - Absolute motor speed, motor has reached target speed
s - Absolute motor speed, motor has NOt reached target speed

E1 - Not started. The channel has not been started, or the kangaroo has lost power during operation

E2 - Not homed. The channel has been started
successfully, but has not homed in a mode
that requires homing, so absolute commands
are not meaningful

e2 - Homing in progress. The home command for
this axis has been sent, but has not completed
yet.

E3 - Control error, channel disabled. Check to
make sure your feedback sources are working
and the system matches how it was set up
during the tune.

E4 - System is in the wrong mode. You must tune
again to use this mode. 

E5 - Readback Command not recognized. The
kangaroo is unable to understand this
command.

E6 - Signal lost. The Kangaroo lost communication
with the PC or microcontroller since the last
command. 


*/
String KangarooControllInterface::ParseReadCommand(String inputCmd) {
  String command_character = inputCmd.substring(2, 3);  //start idx = 2, end idx = 3


  if (command_character == "P" || command_character == "p" || command_character == "S" || command_character == "s") {
    //if the command character is a standard position or speed response, do nothing and return the input as is
    return inputCmd;
  } else if (command_character == "E" || command_character == "e") {
    //if the command character is E or e, there was an error reported from the kangaroo x2 microcontroller

    last_error = inputCmd;
    recovered_from_error = HandleError(inputCmd);

    return "\0";
  }

  return "\0";
}
/*
Function takes an read command containing an error as input
i.e 1,E1 / 1,E2 / ...
and tries to recover from it automatically

if there is an error which can be fixed automatically in this function, it will return true
if it cannot be recovered in code and the kangaroo board needs to be restarted for example, it will return false.

*/
bool KangarooControllInterface::HandleError(String inputErrorCmd) {
  String error_char = inputErrorCmd.substring(2, 3);  //start idx, end idx
  String error_code = inputErrorCmd.substring(3, 4);

  if (error_code == "1") {
    SendInformationDebugSerial("Encountered error fromm kangaroo x2 controller: E1, COULD NOT RECOVER");
    SendInformationDebugSerial("E1: Not started. The channel has not been started, or the Kangaroo has lost power during operation");

    return false;

  } else if (error_code == "2") {

  } else if (error_code == "3") {
    SendInformationDebugSerial("Encountered error fromm kangaroo x2 controller: E3, COULD NOT RECOVER");
    SendInformationDebugSerial("E3: Control error, channel disabled. Check to make sure your feedback sources are working and the system matches how it was set up during the tune. ");

    return false;

  } else if (error_code == "4") {
    SendInformationDebugSerial("Encountered error fromm kangaroo x2 controller: E4, COULD NOT RECOVER");
    SendInformationDebugSerial("E4: System is in the wrong mode. You must tune again to use this mode.");

    return false;

  } else if (error_code == "5") {
    SendInformationDebugSerial("Encountered error fromm kangaroo x2 controller: E5, automatic recovery");
    SendInformationDebugSerial("E5: Readback Command not recognized. The kangaroo is unable to understand this command. => " + inputErrorCmd);

    return true;

  } else if (error_code == "6") {
    SendInformationDebugSerial("Encountered error fromm kangaroo x2 controller: E6, COULD NOT RECOVER");
    SendInformationDebugSerial("E6: Signal lost. The Kangaroo lost communication with the PC or microcontroller since the last command. ");

    //possible solution: try to reconnect? as of now, lay down and die

    return false;

  } else {
    // ?? should not be possible to end up here
    SendInformationDebugSerial("Unknown error encounted in HandlingError func?? = " + inputErrorCmd);
    return false;
  }

  return false;
}

/*
Performs the startup procedure according to the kangaroo x2 manual specifications
*/
bool KangarooControllInterface::PerformKangarooStartupProcedure() {
  SendSimplifiedSerialCommand(String(motor_nr) + ",start\r\n");
  SendSimplifiedSerialCommand(String(motor_nr) + ",home\r\n");
  delay(5000); //wait for motor to reach its position
  int pos = ReadCurrentMotorEncoderPosition();

  if(pos == 0)
  {
    return true;
  }
  SendInformationDebugSerial("ERROR STARTUP PROCEDURE: could not verify that motor is at position=0!");
  return false;
}

/*
Function used to print to a debug serial stream if it has been initialized
*/
void KangarooControllInterface::SendInformationDebugSerial(String inputDebugInformation) {
  if (debug_serial_port->availableForWrite()) {
    debug_serial_port->println(inputDebugInformation);
  }
}
