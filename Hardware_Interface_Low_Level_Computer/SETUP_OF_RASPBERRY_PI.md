## Setup Hardware Interface Low Level Code (Raspberry Pi)

This document aims to describe how to assemble the hardware of this unit and configure a fresh Raspberry Pi to run the low level software on the autonomous platform. This document contains information about all the individual parts of this unit followed by the assembly of the hardware. It will further on describe how one can install a fresh PI with the required software. Lastly, some reference images and circuit diagram can be found.

In theory this unit should already be assembled and the PI should have all of its required software. But, in the cases where one need to replace a piece of hardware or re-install a fresh PI, this document should be helpful.

## List of parts

This is the required hardware to get the hardware interface low level computer up and running from scratch. Items can be found in the [REQ_SPEC_BOOM](../REQ_SPEC_BOOM.xlsx)

| Name                        | Quantity | Description and Notes                                                                 |
| --------------------------- | :------: | :------------------------------------------------------------------------------------ |
| **Essential parts**         |          |                                                                                       |
| Raspberry Pi 4b             |    1     | Recommended 4 or 8 GB RAM version.                                                    |
| LM2596 DC-DC converter      |    1     | Convert 12v to 5v                                                                     |
| RS485 CAN hat               |    1     | To send CAN signals                                                                   |
| 40x40x10 12v fan            |    1     | Cooling                                                                               |
| Raspberry PI 4 heatsink     |    1     | Cooling                                                                               |
| XT60 female connector       |    1     | connect to Power supply                                                               |
| DB9 Female connector        |    1     | Transpond signals                                                                     |
| Biltema fuse holder         |    1     |                                                                                       |
| 3 A mini fuse               |    1     |                                                                                       |
| USB C cable to slice        |    1     | To power the PI                                                                       |
| Micro SD card               |    1     | Micro SD card from reputable brand. Minimum recommended size 32GB. 64 is recommended. |
| **Non-Essential parts**     |          |                                                                                       |
| Xbox controller             |    1     | Controlling GoKart                                                                    |
| Touch screen                |    1     | Interface while debugging                                                             |
| Keboard and mouse           |    1     | interfacing while debugging                                                           |
| Wifi router                 |    1     | To connect laptop wirelessly                                                          |
| Mini-HDMI to HDMI M-M cable |    1     | Connect a screen to PI                                                                |
| **Components to 3D print**  |          |                                                                                       |
| Top                         |    1     | [link](../CAD/RaspberryPi_interface_node/RaspberryPi_AP4_Interface_Top.png)           |
| Bottom                      |    1     | [link](../CAD/RaspberryPi_interface_node/RaspberryPi_AP4_Interface_Bottom.png)        |
| XT60 holder                 |    1     | [link](../CAD/Battery_BOX/x60_clamp.stl-thumbnail.png)                                |
| **Screws and bolts**        |          |                                                                                       |
| 1,5 mm copper wire red      |   N/A    | wiring. For positive charge                                                           |
| 1,5 mm copper wire black    |   N/A    | wiring. For negative charge                                                           |
| Jumper wires                |    2     | for can hat to DB9                                                                    |
| M2 nut and bolts            |    4     | For PI box                                                                            |
| M3 nut and bolts            |    8     | For DC-DC converter, fan, and DB9                                                     |
| M4 nut and bolts            |    5     | For case                                                                              |

## Assembling the hardware parts.

You need access to some workshop in order to solder, crimp wires, and 3D print the three components. You can find images showcasing how it should look, in the last section of this document [Link to images](#reference-images) & [Link to diagram](./resources/AP4_HWI_interface.png).

Some guidelines to assembling the ahrdware according to the diagram is:

- Make sure the DC-DC converter is set to output 5V or else the PI will be fried.
  - On the LM2596 DC-DC converter there is a blue tower with a screw that you can adjust to change the voltage output.
- We used 3A fuses.
- Double check with multimeter that the wiring is correctly setup before connecting PI.
- The PI should receive a voltage of ~5V +- 5% (4.75 - 5.25)

A computer that can flash a micro sd card is required.

## Installing Base Software on Fresh Raspberry Pi 4b <a name="Installing-Base-Software-on-Fresh-Raspberry-Pi-4b"></a>

- Raspberry Pi 4b, 32 GB micro SD card
- Download Raspberry pi imager v1.7.3 and select Ubuntu 22.04
- Insert micro SD card (recommended minimum size 32gb)
- Connect display cable, mouse, keyboard
- Plug in power - Raspberry Pi should boot up
- Follow the standard linux installation process presented on screen
- Raspberry pi system configuration
- Name: ap4-hardware-interface
- Computer Name ap4-hw-interface-rpi4
- Username: find it in the internal [README.md](../INTERNAL/INFOTIV_INTERNAL.md)
- Password: find it in the internal [README.md](../INTERNAL/INFOTIV_INTERNAL.md)

Now you can install any new software updates

```bash
sudo apt-get update && sudo apt-get upgrade
```

Restart Raspberry Pi.

Set up folders and install git

```bash
sudo apt install git
cd Desktop
mkdir GIT
cd GIT
```

Clone repository

```bash
git clone --recursive https://GITLAB_USERNAME:PERSONAL_ACCESS_TOKEN@gitlab.infotivlab.se/internal-development/autonomous_platform.git
```

The canbus communication socket speed can be set using these commands. They need to be runned after every boot but they are automatically done so using the [startup script](./ini_system.bash). **You do not need to run these two commands**.

```bash
sudo apt install can-utils
```

```bash
sudo ip link set can0 up type can bitrate 1000000
```

Follow Docker [installation guide](https://www.ricmedia.com/tutorials/install-docker-on-raspberry-pi-with-ubuntu). Install docker-compose afterwards.

```bash
sudo apt install docker-compose
```

```bash
sudo apt-get update && sudo apt-get upgrade
```

### Install GPIO library on raspberry pi running ubuntu

If running a ubuntu distribution on raspberry pi, make sure libraries for controlling the gpio are installed.

```bash
sudo apt install python3-lgpio
```

### Install can-utils library

In order for linux to be able to handle can messages, install can utils

```bash
sudo apt-get install can-utils
```

### Raspberry pi 4 configuration to integrate with CAN hat.

There are some software configurations to do in order to enable a can interface using the mcp2515 board. Once configured, the can0 should show up as an available interface when using the command 'ifconfig'

Here is a guide we follow in order to do so: https://www.youtube.com/watch?v=fXiOIUZtV10 and here is a different guide in written format https://harrisonsand.com/posts/can-on-the-raspberry-pi/. Below is a step by step guide how we did it.

The MCP2515 module we have is has a clock of 8 Mhz. This is important because it may vary depending on supplier. (The clock frequency should be printed on the clock module on the mcp2515.)

Here is the procedure as described more in detail in the above resources:

1. Update the system

```bash
sudo apt-get update
sudo apt-get upgrade
```

2. Enable SPI bus and load kernel modules:

Using nano editor

With a raspberry pi 4 running on ubuntu 22.04 this file was located at /boot/firmware/config.txt

hence the command becomes

```bash
sudo nano /boot/firmware/config.txt
```

In this file, append, where 12000000 is the clock frequency of the RS485 hat

```bash
dtparam=spi=on
dtoverlay=mcp2515-can0,oscillator=12000000,interrupt=25 
dtoverlay=spi-bcm2835-overlay
```

3. Reboot raspberry pi

If everything is setup as it should, running the following command should not throw an error. If it asks, then install net-tools with:
`sudo apt install net-tools`

```bash
ifconfig can0
```

### Setting the CAN speed

The canbus communication speed can be set using

```bash
sudo ip link set can0 up type can bitrate 125000
```

Where 125000 is the speed of the connected canbuss

### Fixing problems with GPIO pins on raspberry Pi

The MCP2515 CAN controller and the TJCxxx tranciever can operate at different voltages. (But are mounted on the same board refered to as MCP2515)

Raspberry Pi GPIO pins are ONLY 3.3V tolerant, meaning the MCP2515 board had to be modified in order for the logic signals from the card to be understood by the raspberry pi. The CAN tranciever MUST be powered from 5V since the CAN is a 5V network.

### Setting up automatic start of software upon boot-up

It is very useful for the software running on the raspberry pi 4b to start when booting up.

This can be done using linux services. The service will call a bash script "init_system.bash" located in this directory.

The full procedure can be read [here](https://www.shubhamdipt.com/blog/how-to-create-a-systemd-service-in-linux/).

The steps are summarized below.

in /etc/systemd/system add a new service file. Name it:

```bash
startup_lowlevel.service
```

With the following contents:

```bash
[Unit]
Description=description about this service
After=network.target
StartLimitIntervalSec=0

 
[Service]
Type=oneshot
User=root
WorkingDirectory=/home/ap4/Desktop/GIT/autonomous_platform_generation_4/Hardware_Interface_Low_Level_Computer
ExecStart=/home/ap4/Desktop/GIT/autonomous_platform_generation_4/Hardware_Interface_Low_Level_Computer/init_system.bash

[Install]
WantedBy=multi-user.target
```

Reload linux service file to include the newly created service.

```bash
sudo systemctl daemon-reload
```

Start the newly created service

```bash
sudo systemctl start startup_lowlevel.service
```

To enable the service to run on boot, enter

```bash
sudo systemctl enable startup_lowlevel.service
```

The status of the service can be observed using

```bash
sudo systemctl status startup_lowlevel.service
```

After this the bash script will execute upon boot-up. And the docker container will always run on startup.

### Configuring a joystick (Optional).

Although not necessary, if you want to configure the xbox controller you can follow [this guide](./ap4_hwi_code/ap4hwi_ws/src/xbox_controller_feed_forward_ctrl_pkg/documentation_and_research/README.md):

### Testing and debugging (system level)

Here is a list of things to test and verify

- Does it receive CAN messages?
- Can the docker container be started
- Does the docker container start automatically on boot-up?
- Can raspberry pi 4 be pinged from a computer on the same network?

verify that CAN works expected

### Testing and verifying basic functionality

The Raspberry Pi was set up as a CAN sniffer onto the Infotiv CAN lab kit (CAN Education course - Excersise 2). First the kit was setup to run exercise 2 and send temperature over the can bus. The two nodes could send and eceive. Thereafter the raspberry pi was connected onto the CAN bus.\
Some errors occurred such as:

- Make sure the CAN speed is set up right
- The oscillator frequency is set to 8Mhz (NOT 20!!)
- Common ground over the complete CAN system, (GND was connected from lab kit to rasberry pi)

## Reference images

The end result of following this document should look something like:

![Rendered CAD illustration of outside](../Resources/Pictures/CAD/RB4.PNG)

![Outside view](../Resources/extra_documentation_images/raspberry_pi_hwi_outside.jpg)

![Inside view](../Resources/extra_documentation_images/raspberry_pi_hwi_inside.jpg)

![Diagram](./resources/AP4_HWI_interface.png)
