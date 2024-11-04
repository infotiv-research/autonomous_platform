/*
Autonomous Platform Gen 4 testing communication with kangaroo x2 controller board
20/2 - 2023

The kangaroo x2 documentation https://www.dimensionengineering.com/datasheets/KangarooManual.pdf

kangaroo x2 arduino library download https://www.dimensionengineering.com/info/arduino

Mentioned that one can use "Simplified Serial" to controll the controller board
One simply sends strings with simplified commands to the board and it will execute

Example commands
  1,p100  : Position motor 1 to positon = 100 units



Note: Run this sketch on an Arduino Mega to be able to use multiple Serial lines at the same time. I.e Serial AND Serial1 
*/

//alternative  solution for testing functionality, serial passthrough code : https://docs.arduino.cc/built-in-examples/communication/SerialPassthrough

//General functions to interface with kangaroo x2 board
void SendCommand(String inputCommand); //sends a serial command string on serial1
String ReadCommand(); //reads eventuall command from serial1
String SendCommandWaitForResponse(String inputCommand, int max_wait_time_ms); //Sends a serial command string and returns the response on serial1

//user input functions
void ReadSerialMonitorSendKangaroo(); //reads serial inputs from serial monitor (operator) and forwards to serial1 

//specific functions to interface with kangaroo x2 board
String ReadMotorPosition();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //to write to serial monitor on laptop
  Serial1.begin(9600); //to communicate with kangaroo x2, Serial1.listen(); ?

  //kangaroo x2 startup procedure (we are only controlling motor 1). Taken from example in documentation document
  delay(100);
  Serial.println("Starting kangaroo x2 startup procedure");
  SendCommand("1,start");
  //SendCommand("1,units 100 degs = 1024 lines"); //todo find a suitable unit conversion between units and steering wheel angle
  Serial.println("Homing motor axis! waiting 1 seconds");
  SendCommand("1,home");
  delay(5000);
  String homeposition = SendCommandWaitForResponse("1,getp", 1000);
  Serial.println("Motor is in home position = " + homeposition);
  Serial.println("Finnished kangaroo x2 startup procedure");
  SendCommand("1,p" + String(50.0));
}

void loop() {
  // put your main code here, to run repeatedly:

  int minpos = -80;
  int maxpos = 80;

  //find the motors programmed minimum and maximum value
  String motorMin = SendCommandWaitForResponse("1,getmin", 1000);
  String motorMax = SendCommandWaitForResponse("1,getmax", 1000);
  String temp = motorMin;
  if(motorMin != "-1" && motorMax != "-1")
  {
    motorMin = motorMin.substring(3, motorMin.length());
    motorMax = motorMax.substring(3, motorMax.length());
  }

  SendCommand("1,p" + String(minpos));
  delay(10);
  bool done = false;
  while(!done)
  {
    String currentPos = ReadMotorPosition();
    if(currentPos == String(minpos))
    {
      done = true;
    }
    delay(1);
    //write to serial monitor and plot
    Serial.println("Ref:" + String(minpos) + ",Current_pos:" + currentPos + ",motor_min:" + motorMin + ",motor_max:" + motorMax + ",tempvar:" + temp +"\n");
  }

  SendCommand("1,p" + String(maxpos));
  delay(100);
  done = false;
  while(!done)
  {
    String currentPos = ReadMotorPosition();
    if(currentPos == String(maxpos))
    {
      done = true;
    }
    delay(1);
    //write to serial monitor and plot
    Serial.println("Ref:" + String(maxpos) + ",Current_pos:" + currentPos + ",motor_min:" + motorMin + ",motor_max:" + motorMax + ",tempvar:" + temp +"\n");
  }
  
  /*
  delay(10);
  bool done2 = false;
  while(!done2)
  {
    SendCommand("1,p" + String(maxpos));

    String currentPos1 = ReadMotorPosition();
    if(currentPos1 == String(maxpos))
    {
      done2 = true;
    }
    delay(10);
    //write to serial monitor and plot
    Serial.println("Ref:" + String(maxpos) + ",Current_pos:" + currentPos1 + "\n");
  }
  */
  SendCommand("1,p" + String(maxpos));

  delay(3000); //10secs

  

  

}


void SendCommand(String inputCommand) //sends a serial command string on serial1
{
  int msg_length = inputCommand.length();

  if(inputCommand[msg_length-1] != '\n' && inputCommand[msg_length-2] != '\r')
  {
    //Serial.println("Error input command must end with '\r''\n' -> paddin output");
    inputCommand += '\r';
    inputCommand += '\n';
    msg_length = inputCommand.length();
  }
  //Serial.println("Sending kangaroo x2 command over Serial1: " + inputCommand);

  //could possibly be done faster, if speed is an issue later on https://forum.arduino.cc/t/serial-write-to-send-a-string/87226/10
  //send inputcommand string over serial1, character by character
  for(int i = 0; i < msg_length; i++)
  {
    Serial1.write(inputCommand[i]);
  }

}


String ReadCommand() //reads eventuall command from serial1
{
  String returnString = "-1";
  //there is something to be read from the Serial1 line
  if( Serial1.available() == 1)
  {
    //Serial.println("Reading Serial1 input!");
    // https://www.arduino.cc/reference/en/language/functions/communication/serial/readstring/
    //returnString = Serial1.readString(); //read until timeout
    returnString = Serial1.readStringUntil('\n'); //read until newline character
    returnString.trim(); // remove any \r and \n from the en of string

    //.println("Read >" + returnString + "< from Serial1 input");
    
  }
  return returnString;
}

String SendCommandWaitForResponse(String inputCommand, int max_wait_time_ms) //Sends a serial command string and returns the response on serial1
{
  String returnString = "-1"; //temp value, known return value if nothing useful was returned

  SendCommand(inputCommand); //sends command over Serial1

  //compare starttime - endtime
  long int startTime = millis(); 
  long int endTime = 0;
  bool waitloop = true;

  while(waitloop)
  {
    endTime = millis();

    if(endTime - startTime > max_wait_time_ms)
    {
      waitloop = false;
      break;
    }

    String readValueSerial1 = ReadCommand(); //Tries to read line from Serial1

    if(readValueSerial1 != "-1")
    {
      returnString = readValueSerial1;
      waitloop = false;
      break;
    }


  }



  return returnString;
}

String ReadMotorPosition()
{
  String cmd = "1,getp"; //simplified serial command to send to the kangaroo board, it will respond 
  int wait_time = 100; //waits 100 ms for response
  //the response should be 1,Pxxx\r\n or 1,pxxx\r\n. 
  //capital P means that the move is completed, lowercase p means that the goal position has not yet been reached
  String response = SendCommandWaitForResponse(cmd, wait_time ); 

  String returnString = "-1";
  if(response != "-1")
  {
    //extract the position from the msg, should be index 3 to index length - 4
    returnString = response.substring(3, response.length());
  }

  return returnString;
}
