# Simple OBD2 for Alfa Romeo Giulia with ESP32-C3
 
 A simple Arduino project to show how to communicate with your car's OBD2 port using an ESP32-C3 and SN65HVD230 CAN bus transceiver, using ESP32 TWAI (Two-Wire Automotive Interface)

NOTE: It's fun to tinker with your car, but there is always a chance to mess things up. I won't be liable if for some reason you damage your car.

NOTE: The CAN IDs and PIDs used in this app specifically works with a 2019 Alfa Romeo Giulia 2.0L (Petrol). It's highly unlikely that the same PIDs will work with another car, you'll have to research what PIDs work with your own car.

A big thank you to the Alfisti community for reverse enginering some of these PIDs!

Some tips:

1) Connect your car to a battery charger while experimenting. It's highly likely that you'll spend several hours in your car while the battery is being drained.
2) Diagrams of OBD2 pins are normally shown for the female connector in your car. Don't forget that those pins are in swapped/mirrored positions on the male connector.
3) The OBD2 connector has an "always on" 12V pin. Make sure the wire connecting to that pin on your male connector isn't exposed so that it cannot touch other wires!
4) I tried multiple pins on the ESP32-C3 to connect to the SN65HVD230, but only D4/D5 worked for me. Coincidentally these are also the SDA/SCL pins.
5) Check if your car has an OBD2 Security Gateway (SGW). If so, you need to install a SGW Bypass module before you to send/receive OBD2 frames to your car.

Hardware:
 - XIAO ESP32-C3
 - SN65HVD230 CAN bus tranceiver

Arduino library used:
 - ESP32-TWAI-CAN found here: https://github.com/handmade0octopus/ESP32-TWAI-CAN

The following information is very helpful to understand how OBD2 uses the CAN bus for communication:
   https://www.csselectronics.com/pages/obd2-explained-simple-intro

![Hardware](https://github.com/ClaudeMarais/Simple_OBD2_for_AlfaRomeoGiulia/blob/main/Images/Hardware.jpg?raw=true)

![Setup](https://github.com/ClaudeMarais/Simple_OBD2_for_AlfaRomeoGiulia/blob/main/Images/Setup.jpg?raw=true)
