# Golf86-Info - Arduino ESP32 Dot Matrix Display Controller
 
This Arduino program controls a retro-style dot-matrix display, presenting various values received from an MQTT server. The primary focus is on showcasing Speeduino ECU parameters, GPS data, lap times, etc. The display is managed using the Parola library.
Created as a personal project to drive a ESP32 based system in the glovebox of my project car (Golf mk2, hence the name).

**Note** For GPS data it currently is tested to work in pair with https://github.com/askrejans/gps-to-mqtt project.

!NOTE!  Please note, this is raw work in progress, made and tested only specifically for implementation in my car project. If required, modify as needed to wor for any implementation.

## Features
- **Parola Library:** Utilizes the Parola library ([GitHub Link](https://github.com/MajicDesigns/MD_Parola)) for effective control of the dot-matrix display.
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

## Setup
1. Initialize serial communication with a baud rate of 115200.
2. Configure WiFi using the WiFiManager library for web-based configuration.
3. Load MQTT configuration parameters and establish a connection to the MQTT server.
4. Set up navigation controls using a rotary encoder and an external switch.
5. Initialize Parola for hardware SPI and set display intensity based on configuration.
6. Define menu headers, items, and inputs for user interaction.
7. Set up the menu system with navigation and display callbacks.
8. Implement callback functions for saving and loading custom configuration parameters.
9. Set up MQTT message reception and processing.
10. Run the main loop, handling MQTT communication, menu navigation, and display updates.

**Note:** Ensure that the required libraries are installed using the Arduino Library Manager.

Feel free to customize the program to suit your specific requirements and hardware setup.

