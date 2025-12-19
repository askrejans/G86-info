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
#include "SharedData.h"
#include "Constants.h"
#include <WebServer.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_task_wdt.h>

// Constants for configuration (using centralized constants)
const uint16_t MENU_TIMEOUT = MENU_TIMEOUT_MS;
const char *AP_NAME = WIFI_AP_NAME;
const char *WIFI_PASSWORD = WIFI_AP_PASSWORD;
const char *PRIMARY_MQTT_CLIENT_NAME = MQTT_CLIENT_PRIMARY;
const char *SECONDARY_MQTT_CLIENT_NAME = MQTT_CLIENT_SECONDARY;
const char *WELCOME_MSG = WELCOME_MSG_PRIMARY;
const char *WELCOME_MSG2 = WELCOME_MSG_SECONDARY;

// MQTT topic strings - using const char* to avoid heap fragmentation
const char MQTT_ECU_TOPIC[] = "/GOLF86/ECU/";
const char MQTT_GPS_TOPIC[] = "/GOLF86/GPS/";
const char MQTT_TIMER1_TOPIC[] = "/GOLF86/TM1/";
const char MQTT_TIMER2_TOPIC[] = "/GOLF86/TM2/";

// Global message buffers shared by Serial and Scrolling functions
char notAvailableMsg[] = "..N/A..";
bool firstRun = true;
bool newMessageAvailable = false;
char curMessage[MESSAGE_BUFFER_SIZE];
char newMessage[MESSAGE_BUFFER_SIZE];

// Volatile variables are now declared in SharedData.cpp
// These declarations kept for backward compatibility during transition

// Initialize WiFi and MQTT setup instances
WiFiSetup wifiSetup;
MqttSetup mqttSetup;

// Constants for the DOT matrix display setup
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// Initialize Parola library for DOT matrix display
MD_Parola mainDisplay = MD_Parola(HARDWARE_TYPE, DOT_MATRIX_CS_PIN, DOT_MATRIX_MAX_DEVICES);

WebServer server(WEB_SERVER_PORT);

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
  // Optimize by using char buffers instead of String concatenation
  static char buffer[64];
  
  if (var == "UPTIME") {
    snprintf(buffer, sizeof(buffer), "%lu", millis() / 1000);
    return String(buffer);
  } else if (var == "FREE_HEAP") {
    snprintf(buffer, sizeof(buffer), "%u", ESP.getFreeHeap());
    return String(buffer);
  } else if (var == "SIGNAL_STRENGTH") {
    snprintf(buffer, sizeof(buffer), "%d dBm", WiFi.RSSI());
    return String(buffer);
  } else if (var == "MQTT_SERVER") {
    return String(wifiSetup.config.mqtt_server);
  } else if (var == "MQTT_PORT") {
    return String(wifiSetup.config.mqtt_port);
  }
  return String();
}

void handleRoot() {
  // Check if client is actually connected
  if (!server.client() || !server.client().connected()) {
    return;
  }
  
  String htmlContent = htmlPage;
  htmlContent.replace("%UPTIME%", processor("UPTIME"));
  htmlContent.replace("%FREE_HEAP%", processor("FREE_HEAP"));
  htmlContent.replace("%SIGNAL_STRENGTH%", processor("SIGNAL_STRENGTH"));
  htmlContent.replace("%MQTT_SERVER%", processor("MQTT_SERVER"));
  htmlContent.replace("%MQTT_PORT%", processor("MQTT_PORT"));
  
  // Send with proper headers
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "0");
  server.send(200, "text/html; charset=utf-8", htmlContent);
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
  mainDisplay.setCharSpacing(DOT_MATRIX_CHAR_SPACING);

  // Display welcome MSG
  mainDisplay.displayText(WELCOME_MSG, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  mainDisplay.displayAnimate();

  // Initialize 7 Segment secondary display
  secondaryDisplay = LedController<1, 1>(SEVEN_SEG_DIN_PIN, SEVEN_SEG_CLK_PIN, SEVEN_SEG_CS_PIN);
  secondaryDisplay.setIntensity(SEVEN_SEG_DEFAULT_INTENSITY);
  secondaryDisplay.clearMatrix();
  Serial.println("7-Segment display initialized");



  // Initialize WiFi and MQTT setups
  wifiSetup.begin();
  mqttSetup.begin();
  setupNav();
  setupTimerSwitches();

  // Initialize Menu
  M.begin();
  M.setAutoStart(true);
  M.setTimeout(MENU_TIMEOUT);

  // Initialize shared data synchronization primitives
  initSharedData();

  // Configure watchdog timer for both cores
  esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true); // Enable panic on timeout
  esp_task_wdt_add(NULL); // Add current task (Core 1)

  // Create a secondary display task on a separate core
  TaskHandle_t secondaryTaskHandle = NULL;
  BaseType_t taskCreated = xTaskCreatePinnedToCore(
      secondaryDisplayLoop,
      "secondaryDisplayLoop",
      SECONDARY_DISPLAY_STACK_SIZE, // 16KB stack to prevent overflow
      NULL, // Task input parameter
      1,    // Priority of the task
      &secondaryTaskHandle, // Task handle for watchdog
      0     // Core where the task should run (different from the main loop)
  );
  
  if (taskCreated == pdPASS && secondaryTaskHandle != NULL) {
    // Add secondary task to watchdog
    esp_task_wdt_add(secondaryTaskHandle);
    Serial.println("Secondary display task created and added to watchdog");
  } else {
    Serial.println("ERROR: Failed to create secondary display task!");
  }

  // Initialize web server
  server.on("/", HTTP_GET, handleRoot);
  
  // 404 handler
  server.onNotFound([]() {
    server.send(404, "text/plain", "404: Not Found");
  });
  
  server.begin();
  Serial.println("HTTP server started on port " + String(WEB_SERVER_PORT));
}

/**
 * @brief Updates the main display based on the current state.
 */
void updateMainDisplay(bool &wasInMenu, bool &firstRun, char *curMessage)
{
  if (wasInMenu && !M.isInMenu())
  {
    // Properly reset display when exiting menu
    mainDisplay.displayClear();
    mainDisplay.displayReset();
    delay(50); // Increased delay to ensure display is fully cleared

    if (firstRun)
    {
      // Display welcome message and animation on first run
      mainDisplay.setSpriteData(pacman, W_PMAN, F_PMAN, pacman, W_PMAN, F_PMAN);
      mainDisplay.displayText(WELCOME_MSG, wifiSetup.config.align, 100, 3000, PA_OPENING_CURSOR, PA_SPRITE);
    }
    else
    {
      // Display current message on subsequent runs (print data, right aligned)
      mainDisplay.displayText(curMessage, PA_RIGHT, 0, 0, PA_PRINT, PA_NO_EFFECT);
    }

    wasInMenu = false;
  }

  wasInMenu = M.isInMenu();
  M.runMenu();

  if (!M.isInMenu())
  {
    if (mainDisplay.displayAnimate())
    {
      if (firstRun)
      {
        // After first animation completes, keep showing welcome animation until data arrives
        firstRun = false;
        mainDisplay.displayReset();
      }
      else if (newMessageAvailable)
      {
        // Update current message if new message is available
        strcpy(curMessage, newMessage);
        newMessageAvailable = false;
        
        // Clear and reset display before showing new message
        mainDisplay.displayClear();
        delay(10);
        mainDisplay.displayText(curMessage, PA_RIGHT, 0, 0, PA_PRINT, PA_NO_EFFECT);
      }
      else
      {
        // Keep looping welcome animation until we get MQTT data
        mainDisplay.displayReset();
      }
    }
  }
}

/**
 * @brief Main loop function.
 */
void loop()
{
  // Check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Attempting reconnection...");
    wifiSetup.begin(); // This will attempt to reconnect
  }

  // Connect to MQTT server with reconnection logic
  mqttSetup.connect();

  static bool wasInMenu = true;

  // Update the main display based on the current state
  updateMainDisplay(wasInMenu, firstRun, curMessage);
  monitorTimerSwitches();
  
  // Handle web server requests
  server.handleClient();
  
  // Small delay to prevent overwhelming the display controller
  delay(2);

  // Feed the watchdog timer
  esp_task_wdt_reset();
}