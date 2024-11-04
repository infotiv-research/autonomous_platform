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


void SendCommand(String inputCommand); //sends a serial command string on serial1
String ReadCommand(); //reads eventuall command from serial1
String SendCommandWaitForResponse(String inputCommand, int max_wait_time_ms); //Sends a serial command string and returns the response on serial1
void ReadSerialMonitorSendKangaroo(); //reads serial inputs from serial monitor (operator) and forwards to serial1 


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //to write to serial monitor on laptop
  Serial1.begin(9600); //to communicate with kangaroo x2, Serial1.listen(); ?

  //kangaroo x2 startup procedure (we are only controlling motor 1). Taken from example in documentation document
  delay(100);
  Serial.println("Starting kangaroo x2 startup procedure");
  sendCommand("1,start");
  sendCommand("1,units 100 degs = 1024 lines"); //todo find a suitable unit conversion between units and steering wheel angle
  Serial.println("Homing motor axis! waiting 5 seconds");
  sendCommand("1,home");
  delay(5000);
  String homeposition = SendCommandWaitForResponse("1,getp", 1000);
  Serial.println("Motor is in home position = " + homeposition);
  Serial.println("Finnished kangaroo x2 startup procedure");

}

void loop() {
  // put your main code here, to run repeatedly:



  delay(500);

}


void SendCommand(String inputCommand) //sends a serial command string on serial1
{
  int msg_length = inputCommand.length();

  if(inputCommand[length-1] != '\n' && inputCommand[length-2] != '\r')
  {
    Serial.println("Error input command must end with '\r''\n' -> paddin output");
    inputCommand += '\r';
    inputCommand += '\n';
    msg_length = inputCommand.length();
  }
  Serial.println("Sending kangaroo x2 command over Serial1: " + inputcommand);

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
    Serial.println("Reading Serial1 input!")
    // https://www.arduino.cc/reference/en/language/functions/communication/serial/readstring/
    //returnString = Serial1.readString(); //read until timeout
    returnString = Serial1.readStringUntil('\n'); //read until newline character
    returnString.trim(); // remove any \r and \n from the en of string

    Serial.println("Read >" + returnString + "< from Serial1 input");
    
  }
  return returnString;
}

String SendCommandWaitForResponse(String inputCommand, int max_wait_time_ms) //Sends a serial command string and returns the response on serial1
{
  String returnString = "-1"; //temp value, known return value if nothing useful was returned

  SendCommand(inputCommand); //sends command over Serial1

  long int startTime = millis(); 
  long int endtime = 0;
  bool waitloop = true;

  while(waitloop)
  {
    String readValueSerial1 = ReadCommand(); //Tries to read line from Serial1

    if(readValueSeral1 != "-1")
    {
      returnString = readValueSerial1;
      waitloop = false;
      break;
    }
  }



  return returnString;
}
