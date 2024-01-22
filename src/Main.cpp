// This program uses the Parola library to show Speeduino ECU data on DOT matrix display
// Menu with settings and data selection is implemented
// Data is collected from MQTT server using WiFi, OTA update lib included
//
// ECU data, brightness, and align are controlled from the menu.
//
// The interface for menu controls
// a rotary encoder + external switch.
//

#include <Arduino.h>
#include "Menu.h"
#include "SecondaryLoop.h"
#include "PacmanSprites.h"
#include "WiFiSetup.h"
#include "MqttSetup.h"
#include "TimerButtons.h"

// Constants for configuration
const uint16_t MENU_TIMEOUT = 3000;
const char *AP_NAME = "G86-INFO-AP";
const char *WIFI_PASSWORD = "golf1986";
const char *PRIMARY_MQTT_CLIENT_NAME = "G86-INFO";
const char *SECONDARY_MQTT_CLIENT_NAME = "G86-INFO2";
const char *MQTT_TOPIC_BASE = "GOLF86";
const char *WELCOME_MSG = "Golf'86";
const char *WELCOME_MSG2 = "GOLF'86";

// Extract this constant as it's being used in multiple places
const String MQTT_ECU_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/ECU/";
const String MQTT_GPS_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/GPS/";
const String MQTT_TIMER1_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/TM1/";
const String MQTT_TIMER2_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/TM2/";

// Global message buffers shared by Serial and Scrolling functions
char notAvailableMsg[] = "..N/A..";
bool firstRun = true;
bool newMessageAvailable = true;
char curMessage[128];
char newMessage[128];
volatile char secondaryScreenMode[] = "WELCOME";
volatile bool newMessageAvailable2 = true;
volatile char newMessage2[128];

// Initialize WiFi and MQTT setup instances
WiFiSetup wifiSetup;
MqttSetup mqttSetup;

// Constants for the DOT matrix display setup
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 21

// Initialize Parola library for DOT matrix display
MD_Parola mainDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Setup function to initialize peripherals and tasks
void setup()
{
  // Initialize Serial communication
  Serial.begin(115200);

  // Initialize preferences for persistent storage
  wifiSetup.prefs.begin(PRIMARY_MQTT_CLIENT_NAME, false);

  // Initialize WiFi and MQTT setups
  wifiSetup.begin();
  mqttSetup.begin();
  setupNav();
  setupTimerSwitches();

  // Initialize Menu
  M.begin();
  M.setAutoStart(true);
  M.setTimeout(MENU_TIMEOUT);

  // Initialize DOT matrix display
  mainDisplay.begin();
  mainDisplay.setIntensity(wifiSetup.config.bright);
  mainDisplay.setCharSpacing(1);

  // Initialize 7 Segment secondary display
  secondaryDisplay = LedController<1, 1>(DIN, CLK, CS);
  secondaryDisplay.setIntensity(8);
  secondaryDisplay.clearMatrix();

  // Create a secondary display task on a separate core
  xTaskCreatePinnedToCore(
      secondaryDisplayLoop,
      "secondaryDisplayLoop",
      4096, // Stack size in bytes
      NULL, // Task input parameter
      1,    // Priority of the task
      NULL, // Task handle.
      0     // Core where the task should run (different from the main loop)
  );
}

/**
 * @brief Updates the main display based on the current state.
 */
void updateMainDisplay(bool &wasInMenu, bool &firstRun, char *curMessage)
{
  if (wasInMenu && !M.isInMenu())
  {
    mainDisplay.displayClear();

    if (firstRun)
    {
      // Display welcome message and animation on first run
      mainDisplay.setSpriteData(pacman, W_PMAN, F_PMAN, pacman, W_PMAN, F_PMAN);
      mainDisplay.displayText(WELCOME_MSG, wifiSetup.config.align, 100, 3000, PA_OPENING_CURSOR, PA_SPRITE);
    }
    else
    {
      // Display current message on subsequent runs
      mainDisplay.displayText(curMessage, wifiSetup.config.align, 1, 150, PA_PRINT, PA_PRINT);
    }

    wasInMenu = false;
  }

  wasInMenu = M.isInMenu();
  M.runMenu();

  if (!M.isInMenu())
  {
    if (mainDisplay.displayAnimate())
    {
      firstRun = false;

      if (newMessageAvailable)
      {
        // Update current message if new message is available
        strcpy(curMessage, newMessage);
        newMessageAvailable = false;
      }
      mainDisplay.displayReset();
    }
  }
}

/**
 * @brief Main loop function.
 */
void loop()
{
  // Connect to MQTT server
  mqttSetup.connect();

  static bool wasInMenu = true;

  // Update the main display based on the current state
  updateMainDisplay(wasInMenu, firstRun, curMessage);
  monitorTimerSwitches();
}