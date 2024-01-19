# Golf86-Info - PlatformIO/Arduino ESP32 LED Matrix Display Controller

This PlatformIO built C++/Arduino ESP32 program controls a retro-style LED-matrix display, presenting various values received from an MQTT server. The primary focus is on showcasing Speeduino ECU parameters, GPS data, lap times, etc. The display is managed using the Parola library.
Created as a personal project to drive an ESP32 based system in the glovebox of my project car (Golf mk2, hence the name). Currently only 2 core ESP32 is supported, as FreeRTOS task switching is used for display/mqtt control.

**Note** For GPS data, it currently is tested to work in pair with https://github.com/askrejans/gps-to-mqtt project.
**Note** For Speeduino ECU data, it currently is tested to work in pair with https://github.com/askrejans/speeduino-to-mqtt project.

!NOTE! Please note, this is a work in progress, made and tested only specifically for implementation in my car project. If required, modify as needed for any implementation.

## Features
- **Parola Library:** Utilizes the Parola library ([GitHub Link](https://github.com/MajicDesigns/MD_Parola)) for effective control of the LED-matrix display.
- **Settings Menu:** Implements a settings menu, allowing users to configure ECU data, brightness, and alignment interactively.
- **WiFi Connectivity:** Connects to WiFi using the WiFiManager library ([GitHub Link](https://github.com/tzapu/WiFiManager)) for hassle-free web-based configuration.
- **MQTT Integration:** Utilizes MQTT library ([GitHub Link](https://github.com/256dpi/arduino-mqtt)) for collecting data from an MQTT server. Note: MQTT is currently supported without a password.
- **OTA Updates:** Includes Over-The-Air (OTA) update functionality for easy firmware updates.
- **User Interface:** Menu controls are managed through a rotary encoder and an external switch.

## Dependencies
- [Preferences.h](https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences)
- [SPI.h](https://www.arduino.cc/en/reference/SPI)
- [MD_Parola](https://github.com/MajicDesigns/MD_Parola)
- [MD_MAX72xx](https://github.com/MajicDesigns/MD_MAX72XX)
- [MD_Menu](https://github.com/MajicDesigns/MD_Menu)
- [MD_REncoder](https://github.com/MajicDesigns/MD_REncoder)
- [MD_UISwitch](https://github.com/MajicDesigns/MD_UISwitch)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [MQTT](https://github.com/256dpi/arduino-mqtt)
- [WiFi](https://www.arduino.cc/en/reference/WiFi)

Dependency Graph
|-- googletest @ 1.12.1
|-- MQTT @ 2.5.1
|-- Preferences @ 2.0.0
|-- SPI @ 2.0.0
|-- WiFi @ 2.0.0
|-- WiFiManager @ 2.0.16-rc.2
|-- MD_MAX72XX @ 3.5.1
|-- MD_Menu @ 2.1.3
|-- MD_Parola @ 3.7.1
|-- MD_REncoder @ 1.0.2
|-- MD_UISwitch @ 2.2.2
|-- LedController @ 2.0.2

## Configuration
- **Menu Timeout:** 3000 milliseconds
- **MQTT Server Default:** localhost
- **MQTT Port Default:** 1883

## Hardware Configuration
- **LED Matrix Configuration:**
  - Hardware Type: MD_MAX72XX::FC16_HW
  - Max Devices: 4
  - CS Pin: 21
- **Rotary Switch and Button:**
  - Rotary A Pin: 25
  - Rotary B Pin: 26
  - Control (CTL) Pin: 27

## PlatformIO Build Instructions
1. Install PlatformIO IDE (if not already installed).
2. Open the project in PlatformIO IDE.
3. Configure the target board and other build settings in the `platformio.ini` file.
4. Build the project by clicking on the "Build" button in PlatformIO IDE.

## Setup
1. Setup hardware connections to defined pins.
2. Configure WiFi using the WiFiManager library for web-based configuration. AP default password: golf1986.
3. Save MQTT configuration parameters and establish a connection to the MQTT server.

**Note:** Ensure that the required libraries are installed using PlatformIO Library Manager.

Feel free to customize the program to suit your specific requirements and hardware setup.
