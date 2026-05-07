# Speed Sensor ECU logic

- Raw Speed Sensor data is received through pins **PA0-PA3** on bluepill board.
- The pulse count is then calculated and stored to CAN buffer as this. (Note that the sample time also has to be stored and sent to later be able to calculate the actual velocity)
```bash
    encode_can_0x5dc_SpeedSensorLF_PulseCnt(&can_interface.can_storage_container,pulseCount_LeftFront);
    encode_can_0x5dc_SpeedSensorRF_PulseCnt(&can_interface.can_storage_container,pulseCount_RightFront);
    encode_can_0x5dc_SpeedSensorLR_PulseCnt(&can_interface.can_storage_container,pulseCount_LeftRear);
    encode_can_0x5dc_SpeedSensorRR_PulseCnt(&can_interface.can_storage_container,pulseCount_RightRear);
    encode_can_0x5dc_SpeedSensorSampleTime(&can_interface.can_storage_container, can_schedule_time);
```
- The data is then sent by this command that builds the CAN frame and transmits it through MCP2515
```bash
can_interface.SendSingleCanFrame(CAN_ID_GET_SPEED_SENSOR);
```
- Now it computes pulses every 100 ms which can be tuned to obtain better performance, but as for now the sample time is 100 ms.


# Flash Speed Sensor ECU code to STM32
- run these commands in the terminal on your local computer (no ssh in to PI)
```bash
cd Desktop/autonomous_platform/CAN_Nodes_Microcontroller_Node/Speed_Sensor_ECU
```
- Then make sure yopu have saved your `Speed_Sensor_ECU/main.cpp` file or other file that it is included in that file
- Connect ST-LINK with usb to your computer and the following pins on STM32 to the ST-LINK
* 3.3V --> VTref
* SWDIO --> SWDIO
* SWCLK --> SWCLK
* GND --> GND

Here is an overwiev of ST-LINK pins and what they refer to.


 ![ST-LINK pin_chart](../../Resources\extra_documentation_images\ST-LINK_pins.jpg)

Now when you have connected the ST-LINK properly and are located in `Speed_Sensor_ECU`-folder you write the follwoing command in the terminal.
```bash
pio run -t upload
```
Now the new and fresh updated code are flashed to the STM32 and hopefully everything will work as intended!

You could also build and flash using vscode extension `PlatformIO`

- Go to `PlatformIO` extension tab on the left bar at vscode.
- Locate the Speed_Sensor_ECU folder 
- In the bottom row of vscode there is buttons saying build and upload.

Press build first and let it build without errors, then press upload to load the built code to STM32.

# Debugging while flashing to STM32

The main.cpp file in `CAN_Nodes_Microcontroller_Code\Speed_Sensor_ECU\src` includes debugging loggers. The issue during 2026 work was that the Speed Sensors data was not transmitted over the CAN-network leading to gokart not able to run properly. This issue was dealth with and debugging loggers was added to easily keep track of the issue. If plugging in micro-usb to the STM32 to your computer you can run the following commands to see the status.

```bash
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
```
Then you whould expect something like

```bash
/dev/ttyACM0
```
Then use that output in the following command:

```bash
pio device monitor -p /dev/ttyACM0 -b 9600
```
the log of the Speed Sensor ECU will be printed and you can track the problem. You will se how many tx that was ok and how many was faulty as well as the ErrorFlag (EFLG) of them. You will also get a warning that 0x5dc could not get transmitted over CAN which means Speed Sensor data does not reach its destination and you will need to fix it.
The error flags is as follows:

- 0x01 = General warning state
- 0x02 = Receive Warning
- 0x04 = Transmit Warning
- 0x08 = Receive error-passive
- 0x10 = Transmit error-passive
- 0x20 = Transmit bus-off
- 0x40 = RX buffer 0 overflow
- 0x80 = RX buffer 1 overflow

So if you for example see EFLG = 0x15 you have got 0x10+0x04+0x01 which means Transmit error-passive + Transmit Warning + General warning state which means the CAN has problems transmitting which usually means missing ACKS but can bus is not fully off.
# TODO and Generic notes of Speed Sensor
Todo / Generic notes.

Considerations for design of speed sensor signaling in the AP4.

Sent the signal info as raw data to the HWI that will there calculate a speed.

Rationale: the actual measured speed will need to be derived by 2 parameters: speed sensor disk hole pattern and wheel circumference.  Those parameters are different for the front and rear wheel.

Defining the unity for raw data from the speed sensor on CAN:

- pulses/second.

With a design criteria of maximum 10 m/s, 35 of 60 hole speed disk and a circumference of appr 0.3m of a wheel, the max rotation
frequency of wheel can be around 30 rotations per second. With

speed = wheel circumference * rotations per second.

With Speed = 10 m/s
wheel cirumference = 0.69 (front wheel) and 0,78 (rear wheel)
gives
rps front = 10/0,78 =  12,8
rps rear = 10/0,69  = 14.5

with
holes per disk front = 35
holes per disk rear = 60

We get
pules/s front = 35 * 12,8 = 448\
pules/s rear = 60 * 14.5 = 870

The sensor is not able to see direction and will not be. Unless with using an analog input that could observe the edge flank of a signal and using a hole that would have an assymetic pattern. NOw a simpler approach was chosen, with digital I/O's that are only able to see a change from L/H or H/L. Pulses therefore do not get a direction and will only have zero or positive value.

So to code max 870 pulses 10 bits (2^10 = 1024 max) would be enough. For some additional space we can use 12 bits.

That will leave some space for status bits or additional info.
Diagnostics:
max speed overflow: a speed of max speed (0xFFF) should be considered as an overflow value (max speed in reality is equel to or higher than 4095 pulses/s).
Invalid: currently the speed sensor interpreter has no logic to handle invalid input data.

Blue Pill not initialied: LED will not light
CAN not initialized: LED will not light.
CAN be also seen in the terminal

As soon as CAN is initalized and the speed ECU can send CAN messages it will do so.

Send frequency: every 0.1 s.  Is now every second.

DBC file:

Speed sensor ECU:

assembling.

Mounting on the kart:

use

- m4 x10 screws
- m4 nut
  to fasten on the mount plate.
