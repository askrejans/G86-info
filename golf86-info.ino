// This program uses the Parola library to show Speeduino ECU data on DOT matrix display
// Menu with settings and data selection is implemented
// Data is collected from MQTT server using wifi, OTA update lib included
//
// ECU data, brightness and aligh are controlled from the menu.
//
// The interface for menu controls
// a rotary encoder + external switch.
//
//

#include <Preferences.h>    // Preferences library for ESP32
#include <SPI.h>            //SPI interface for display control
#include <WiFiManager.h>    //Wifi Web manager/config + custom parameters https://github.com/tzapu/WiFiManager
#include <MQTT.h>           //MQTT lib https://github.com/256dpi/arduino-mqtt
#include <WiFi.h>           //Wifi lib for MQTT
#include "Menu.h"           //Device menu and navigation
#include "PacmanSprites.h"  //Sprites for startup animation

// Various constants
const uint16_t MENU_TIMEOUT = 3000;         // in milliseconds
const char* AP_NAME = "G86-INFO-AP";        //WIFI access point name
const char* WIFI_PASSWORD = "golf1986";     //WIFI access point password
const char* MQTT_CLIENT_NAME = "G86-INFO";  // MQTT client name
const char* MQTT_TOPIC_BASE = "GOLF86";     // MQTT base topic for ECU and GPS data
const char* WELCOME_MSG = "Golf'86";        //Welcome message

// Extract this constant as it's being used in multiple places
const String MQTT_ECU_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/ECU/";
const String MQTT_GPS_TOPIC = "/" + String(MQTT_TOPIC_BASE) + "/GPS/";

Preferences prefs;
WiFiClient net;
MQTTClient mqtt;

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 21

// Display object for the main Parola display
MD_Parola mainDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Configuration structure for parameters
struct cfgParameter_t {
  uint8_t bright;        // display intensity
  textPosition_t align;  // text alignment
  char mqtt_server[40];  // mqtt server
  char mqtt_port[6];     // mqtt port
} Config;

//flag for saving data
bool shouldSaveConfig = false;

//Transform helper variables
unsigned long lastBlinkMillis = 0;  // Variable to store the last time the colon was blinked for time
bool colonVisible = true;           // Flag to track the visibility of the colon for time
char dataIndex[] = "RPM";

void setupWifi(void) {
  WiFiManagerParameter custom_mqtt_server("server", "MQTT server", Config.mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT port", Config.mqtt_port, 6);

  //WiFiManager, Local intialization.
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setPreSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);

  wifiManager.setTimeout(180);
  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "G86-INFO"),
  // then goes into a blocking loop awaiting configuration and will return success result

  if (!wifiManager.autoConnect(AP_NAME, WIFI_PASSWORD)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    strncpy(Config.mqtt_server, custom_mqtt_server.getValue(), sizeof(Config.mqtt_server) - 1);
    strncpy(Config.mqtt_port, custom_mqtt_port.getValue(), sizeof(Config.mqtt_port) - 1);
    paramSave();

    Serial.println("MQTT config saved: ");
    Serial.println("\tmqtt_server : " + String(Config.mqtt_server));
    Serial.println("\tmqtt_port : " + String(Config.mqtt_port));
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  Serial.println("Connected to wifi..");
}

void setupMqtt(void) {
  mqtt.begin(Config.mqtt_server, net);
  mqtt.onMessage(messageReceived);

  Serial.print("\nconnecting to MQTT to " + String(Config.mqtt_server) + ":" + String(Config.mqtt_port));
  while (!mqtt.connect(MQTT_CLIENT_NAME, "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nMQTT connected!");
}

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE 128
char notAvailableMsg[] = "..N/A..";
bool firstRun = true;
bool newMessageAvailable = true;
char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
char dataValue[BUF_SIZE];

// String array for ECU and GPS Data parameters
const String ecuDataStrings[] = { "RPM", "TPS", "VE1", "O2P", "AFT", "MAT", "CAD", "MAP", "BAT", "ADV", "PW1", "SPK", "DWL", "ILL", "BAR", "TAE", "NER", "ENG" };
const String gpsDataStrings[] = { "SPD", "TME", "DTE", "LAT", "LNG", "ALT", "CRS", "QTY" };

// Menu system callback functions
MD_Menu::value_t* mnuValueRqst(MD_Menu::mnuId_t id, bool bGet) {
  static MD_Menu::value_t v;

  switch (id) {

    case 2:  // ECU Values
      if (!bGet) {
        mqtt.unsubscribe(MQTT_ECU_TOPIC + String(dataIndex));
        mqtt.unsubscribe(MQTT_GPS_TOPIC + String(dataIndex));
      }

      if (bGet) {
        // Convert dataIndex to index and set the value
        for (int i = 0; i < sizeof(ecuDataStrings) / sizeof(ecuDataStrings[0]); ++i) {
          if (String(dataIndex) == ecuDataStrings[i]) {
            v.value = i;
            break;
          }
        }
      } else {
        strncpy(dataIndex, ecuDataStrings[v.value].c_str(), sizeof(dataIndex));
        dataIndex[sizeof(dataIndex) - 1] = '\0';  // Ensure null-termination

        // subscribe to chosen MQTT topic
        mqtt.subscribe(MQTT_ECU_TOPIC + String(dataIndex));
        Serial.println("\nSubscribed to topic: " + MQTT_ECU_TOPIC + String(dataIndex));
        // set initial display value
        strcpy(newMessage, "---");
        newMessageAvailable = true;
      }
      break;
    case 3:  // GPS Values
      if (!bGet) {
        mqtt.unsubscribe(MQTT_ECU_TOPIC + String(dataIndex));
        mqtt.unsubscribe(MQTT_GPS_TOPIC + String(dataIndex));
      }
      if (bGet) {
        for (int i = 0; i < sizeof(gpsDataStrings) / sizeof(gpsDataStrings[0]); ++i) {
          if (String(dataIndex) == gpsDataStrings[i]) {
            v.value = i;
            break;
          }
        }
      } else {
        strncpy(dataIndex, gpsDataStrings[v.value].c_str(), sizeof(dataIndex));
        dataIndex[sizeof(dataIndex) - 1] = '\0';  // Ensure null-termination

        // subscribe to chosen MQTT topic
        mqtt.subscribe(MQTT_GPS_TOPIC + String(dataIndex));

        Serial.println("\nSubscribed to topic: " + MQTT_GPS_TOPIC + String(dataIndex));
        // set initial display value
        strcpy(newMessage, "---");
        newMessageAvailable = true;
      }
      break;
    case 4:  // Text align
      if (bGet) {
        switch (Config.align) {
          case PA_LEFT: v.value = 0; break;
          case PA_CENTER: v.value = 1; break;
          case PA_RIGHT: v.value = 2; break;
        }
      } else {
        switch (v.value) {
          case 0: Config.align = PA_LEFT; break;
          case 1: Config.align = PA_CENTER; break;
          case 2: Config.align = PA_RIGHT; break;
        }
        mainDisplay.setTextAlignment(Config.align);
      }
      break;

    case 5:  // Screen brightness
      if (bGet)
        v.value = Config.bright;
      else {
        Config.bright = v.value;
        mainDisplay.setIntensity(Config.bright);
      }
      break;
  }

  // if things were requested, return the buffer
  if (bGet)
    return &v;
  else  // save the parameters
    paramSave();

  return nullptr;
}

//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save custom config..");
  shouldSaveConfig = true;
}

// Configuration Save to Preferences
void paramSave(void) {
  prefs.putBytes("config", &Config, sizeof(Config));
}

//Load config data from Preferences
void paramLoad(void) {
  // Reading the stored configuration from preferences
  prefs.getBytes("config", &Config, sizeof(Config));

  // Optional: Add validation or set default values for individual fields
  if (strlen(Config.mqtt_port) == 0) {
    strncpy(Config.mqtt_port, "1883", sizeof(Config.mqtt_port));
  }
  if (strlen(Config.mqtt_server) == 0) {
    strncpy(Config.mqtt_port, "localhost", sizeof(Config.mqtt_port));
  }
}

void messageReceived(String& topic, String& payload) {
  // Extract the last two segments from the MQTT topic
  int lastSlashIndex = topic.lastIndexOf('/');
  if (lastSlashIndex != -1) {
    String lastSegment = topic.substring(lastSlashIndex + 1);

    int secondLastSlashIndex = topic.lastIndexOf('/', lastSlashIndex - 1);
    if (secondLastSlashIndex != -1) {
      String secondLastSegment = topic.substring(secondLastSlashIndex + 1, lastSlashIndex);

      // Switch between the last two segments
      if (secondLastSegment == "GPS" && lastSegment == "TME") {
        transformTime(payload);
      } else if (secondLastSegment == "GPS" && lastSegment == "DTE") {
        // Remove dots between values, add slash, remove year. Incoming format dd.mm.yyyy
        payload.replace(".", "");
        String day = payload.substring(0, 2);
        String month = payload.substring(2, 4);
        payload = day + "/" + month;
      } else if (secondLastSegment == "GPS" && lastSegment == "SPD") {
        // Round the speed to the closest 1kmh + add kmh string
        float speed = payload.toFloat();
        payload = String(int(speed)) + "kmh";
      } else if (secondLastSegment == "GPS" && lastSegment == "ALT") {
        payload = payload + "m";
      } else if (secondLastSegment == "ECU" && (lastSegment == "TPS" || lastSegment == "VE1" || lastSegment == "TAE")) {
        // Add '%' sign to the end of the numeric value
        payload = payload + "%";
      } else if (secondLastSegment == "ECU" && (lastSegment == "MAT" || lastSegment == "CAD")) {
        // Add 'C'
        payload = payload + "C";
      } else if (secondLastSegment == "ECU" && lastSegment == "BAT") {
        // Add 'V'
        payload = payload + "V";
      } else if (secondLastSegment == "ECU" && lastSegment == "DWL") {
        // Add 'ms'
        payload = payload + "ms";
      }
    }
  }
  // Convert String to char array
  strncpy(newMessage, payload.c_str(), sizeof(newMessage) - 1);
  newMessage[sizeof(newMessage) - 1] = '\0';  // Ensure null-termination
  newMessageAvailable = true;
}

void transformTime(String& timeString) {
  // Check if the timeString has the expected format "HH:MM:SS"
  if (timeString.length() == 8 && timeString[2] == ':' && timeString[5] == ':') {
    // Extract the "HH:MM" part
    String transformedTime = timeString.substring(0, 5);

    // Replace the original timeString with the transformed one
    timeString = transformedTime;


    if (!colonVisible) {
      std::replace(timeString.begin(), timeString.end(), ':', ' ');
    }

    // Blink the colon every second
    if (millis() - lastBlinkMillis >= 1000) {
      lastBlinkMillis = millis();
      colonVisible = !colonVisible;  // Toggle colon visibility
    }

  } else {
    Serial.println("Invalid time format: " + timeString);
  }
}

// Display callback function
bool display(MD_Menu::userDisplayAction_t action, char* msg) {
  switch (action) {
    case MD_Menu::DISP_INIT:
      // nothing to do
      break;

    case MD_Menu::DISP_CLEAR:
      mainDisplay.displayClear();
      break;

    case MD_Menu::DISP_L0:
      // Only one zone, no line 0
      mainDisplay.print(msg);
      break;

    case MD_Menu::DISP_L1:
      mainDisplay.print(msg);
      break;
  }

  return true;
}

void setup() {

  Serial.begin(115200);
  prefs.begin(MQTT_CLIENT_NAME, false);

  setupWifi();
  paramLoad();
  setupMqtt();
  setupNav();

  //Menu
  M.begin();
  M.setAutoStart(true);
  M.setTimeout(MENU_TIMEOUT);

  //Main display
  mainDisplay.begin();
  mainDisplay.setIntensity(Config.bright);
  mainDisplay.setCharSpacing(1);
}

void loop() {

  mqtt.loop();

  static bool wasInMenu = true;  // ensure we initialize the display first

  if (wasInMenu && !M.isInMenu())  // was running but not anymore
  {
    // Reset the main display to show the message
    mainDisplay.displayClear();

    if (firstRun) {
      mainDisplay.setSpriteData(pacman, W_PMAN, F_PMAN, pacman, W_PMAN, F_PMAN);
      mainDisplay.displayText(WELCOME_MSG, Config.align, 100, 3000, PA_OPENING_CURSOR, PA_SPRITE);
    } else {
      mainDisplay.displayText(curMessage, Config.align, 1, 150, PA_PRINT, PA_PRINT);
    }

    wasInMenu = false;
  }

  wasInMenu = M.isInMenu();  // save the current status
  M.runMenu();               // run or autostart the menu

  if (!M.isInMenu())  // not running the menu? do something else
  {
    // animate the main display and check for a new message if ended
    if (mainDisplay.displayAnimate()) {
      firstRun = false;

      if (newMessageAvailable) {
        strcpy(curMessage, newMessage);
        newMessageAvailable = false;
      }
      mainDisplay.displayReset();
    }
  }
}
