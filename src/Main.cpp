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
#include <WebServer.h>
#include <WiFi.h>
#include <esp_wifi.h>

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

WebServer server(80);

const char* htmlPage = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>G86 Stats</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f4f9;
      color: #333;
    }
    .container {
      max-width: 800px;
      margin: 50px auto;
      padding: 20px;
      background-color: #fff;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      border-radius: 8px;
    }
    h1 {
      text-align: center;
      color: #007BFF;
    }
    p {
      font-size: 1.2em;
      line-height: 1.6em;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>G86 Stats</h1>
    <p><strong>Uptime:</strong> %UPTIME%</p>
    <p><strong>Free Heap:</strong> %FREE_HEAP%</p>
    <p><strong>WiFi Signal Strength:</strong> %SIGNAL_STRENGTH%</p>
    <p><strong>MQTT Server:</strong> %MQTT_SERVER%</p>
    <p><strong>MQTT Port:</strong> %MQTT_PORT%</p>
  </div>
</body>
</html>)rawliteral";

String processor(const String& var) {
  if (var == "UPTIME") {
    return String(millis() / 1000);
  } else if (var == "FREE_HEAP") {
    return String(ESP.getFreeHeap());
  } else if (var == "SIGNAL_STRENGTH") {
    return String(WiFi.RSSI());
  } else if (var == "MQTT_SERVER") {
    return String(wifiSetup.config.mqtt_server);
  } else if (var == "MQTT_PORT") {
    return String(wifiSetup.config.mqtt_port);
  }
  return String();
}

void handleRoot() {
  String htmlContent = htmlPage;
  htmlContent.replace("%UPTIME%", processor("UPTIME"));
  htmlContent.replace("%FREE_HEAP%", processor("FREE_HEAP"));
  htmlContent.replace("%SIGNAL_STRENGTH%", processor("SIGNAL_STRENGTH"));
  htmlContent.replace("%MQTT_SERVER%", processor("MQTT_SERVER"));
  htmlContent.replace("%MQTT_PORT%", processor("MQTT_PORT"));
  server.send(200, "text/html", htmlContent);
}

// Setup function to initialize peripherals and tasks
void setup()
{
  // Initialize Serial communication
  Serial.begin(115200);

  // Initialize preferences for persistent storage
  wifiSetup.prefs.begin(PRIMARY_MQTT_CLIENT_NAME, false);

  // Initialize DOT matrix display
  mainDisplay.begin();
  mainDisplay.setIntensity(wifiSetup.config.bright);
  mainDisplay.setCharSpacing(1);

  // Display welcome MSG
  mainDisplay.displayText(WELCOME_MSG, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  mainDisplay.displayAnimate();

  // Initialize 7 Segment secondary display
  secondaryDisplay = LedController<1, 1>(DIN, CLK, CS);
  secondaryDisplay.setIntensity(8);
  secondaryDisplay.clearMatrix();



  // Initialize WiFi and MQTT setups
  wifiSetup.begin();
  mqttSetup.begin();
  setupNav();
  setupTimerSwitches();

  // Initialize Menu
  M.begin();
  M.setAutoStart(true);
  M.setTimeout(MENU_TIMEOUT);

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

    // Initialize web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
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
  server.handleClient();

}