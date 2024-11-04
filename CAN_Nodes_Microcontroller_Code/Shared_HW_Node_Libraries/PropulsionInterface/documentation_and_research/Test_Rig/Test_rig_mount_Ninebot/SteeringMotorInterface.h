/*
Autonomous Platform Generation 4  interface with kangaroo x2 motor controller board
used for steering the gokart using simplified serial commands

Kangaroo x2 motion controller manual can be found here: https://www.dimensionengineering.com/datasheets/KangarooManual.pdf

*/
#ifndef __KANGAROO_X2_SIMPLIFIED_SERIAL_INTERFACE__
#define __KANGAROO_X2_SIMPLIFIED_SERIAL_INTERFACE__

#include "Stream.h"

class KangarooControllInterface {
public:
  //public variables


private:
  //private variables
  int wait_for_response_time;  //in ms
  Stream *command_serial_port;
  int port_nr;
  int motor_nr;
  int motorFixedSpeed;
  int motor_max_limit;
  int motor_min_limit;
  Stream *debug_serial_port;  // serial port to write any debug information on, for debugging purposes.

  String last_error;          //saves the last error code encountered
  bool recovered_from_error;  //indiciates weather or not the error could be recovered from.

public:
  //public functions


  //cpnstructor
  KangarooControllInterface(Stream &inputCmdPort, int inputPortNr, int inputMotorNr, int inputMotorFixedSpeed, int inputWaitForResponseTime);
  KangarooControllInterface(Stream &inputCmdPort, int inputPortNr, int inputMotorNr, int inputMotorFixedSpeed, int inputWaitForResponseTime, Stream &inputDebugPort);
  KangarooControllInterface();

  //explicit init func
  bool init(Stream &inputCmdPort, int inputPortNr, int inputMotorNr, int inputMotorFixedSpeed, int inputWaitForResponseTime);
  bool init(Stream &inputCmdPort, int inputPortNr, int inputMotorNr, int inputMotorFixedSpeed, int inputWaitForResponseTime, Stream &inputDebugPort);
  

  //set functions
  bool SetSerialPort(Stream &inputPort);
  bool SetWaitResponseTime(int inputTime);
  bool SetSerialPortNr(int inputSerialPortNr);
  bool SetMotorNr(int inputMotorNr);
  bool SetMotorFixedMovementSpeed(int inputMotorFixedSpeed);

  //get functions of internal private variables
  Stream* GetSerialPort();
  int GetWaitResponseTime();
  int GetSerialPortNr();
  int GetMotorNr();
  bool GetMotorFixedMovementSpeed();
  int GetSavedMotorMaxLimit();
  int GetSavedMotorMinLimit();

  //Read Motor Information Variables
  int ReadCurrentMotorEncoderPosition();
  int ReadMotorEncoderMaxLimit();
  int ReadMotorEncoderMinLimit();
  bool HasReachedTargetPosition();
  String GetLastErrorMsg();  //returns 1 to 6.

  //Set Motor Variables
  bool SetMotorAbsolutePosition(int inputMotorPosition);
  bool SetMotorIncrementPosition(int inputIncrementPosition);
  bool SetMotorAbsoluteSpeed(int inputMotorSpeed);
  bool SetMotorIncrementSpeed(int inputIncrementSpeed);
  bool SetMotorUnits(int unit1, int unit2);

private:
  //private functions
  bool SendSimplifiedSerialCommand(String inputCmd);
  String ReadSimplifiedSerialCommand();
  String SendSimplifiedSerialCommandWaitForResponse(String inputCmd);

  String ParseReadCommand(String inputCmd);
  bool HandleError(String inputErrorCmd);

  bool PerformKangarooStartupProcedure();

  void SendInformationDebugSerial(String inputDebugInformation);
};


#endif