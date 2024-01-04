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

#include <Preferences.h>  // Preferences library for ESP32
#include <SPI.h>          //SPI interface for display control
#include <MD_Parola.h>    //Display library https://github.com/MajicDesigns/MD_Parola
#include <MD_MAX72xx.h>   //Display driver https://github.com/MajicDesigns/MD_MAX72XX
#include <MD_Menu.h>      //Settings menu https://github.com/MajicDesigns/MD_Menu
#include <MD_REncoder.h>  //Rotary encoder lib https://github.com/MajicDesigns/MD_REncoder
#include <MD_UISwitch.h>  //Physical swith lib https://github.com/MajicDesigns/MD_UISwitch
#include <WiFiManager.h>  //Wifi Web manager/config + custom parameters https://github.com/tzapu/WiFiManager
#include <MQTT.h>         //MQTT lib https://github.com/256dpi/arduino-mqtt
#include <WiFi.h>         //Wifi lib for MQTT

// Constants for menu and EEPROM
const uint16_t MENU_TIMEOUT = 3000;  // in milliseconds
Preferences prefs;
WiFiClient net;
MQTTClient mqtt;
extern MD_Menu M;

// Configuration structure for parameters
struct cfgParameter_t {
  uint8_t bright;        // display intensity
  textPosition_t align;  // text alignment
  char mqtt_server[40];  // mqtt server
  char mqtt_port[6];     // mqtt port
} Config;

//flag for saving data
bool shouldSaveConfig = false;

// LED matrix configuration
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 21

//Rotary switch and button init
const uint8_t RE_A_PIN = 25;
const uint8_t RE_B_PIN = 26;
const uint8_t CTL_PIN = 27;

MD_REncoder RE(RE_A_PIN, RE_B_PIN);
MD_UISwitch_Digital swCtl(CTL_PIN);

// Setup navigation with rotary encoder and switch
void setupNav(void) {
  RE.begin();
  swCtl.begin();
  swCtl.enableRepeat(false);
  swCtl.enableLongPress(true);
  swCtl.setPressTime(500);
  swCtl.setLongPressTime(1000);
  swCtl.enableDoublePress(false);
}

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

  if (!wifiManager.autoConnect("G86-INFO-AP", "golf1986")) {
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
  while (!mqtt.connect("GOLF86INFO", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nMQTT connected!");
}

// Navigation callback function
MD_Menu::userNavAction_t navigation(uint16_t& incDelta) {
  uint8_t re = RE.read();

  if (re != DIR_NONE) {
    if (M.isInEdit()) incDelta = 1 << abs(RE.speed() >> 3);
    return re == DIR_CCW ? MD_Menu::NAV_DEC : MD_Menu::NAV_INC;
  }

  switch (swCtl.read()) {
    case MD_UISwitch::KEY_PRESS:
      return MD_Menu::NAV_SEL;
    case MD_UISwitch::KEY_LONGPRESS:
      return MD_Menu::NAV_ESC;
  }

  return MD_Menu::NAV_NULL;
}

// MD_Parola initialization for hardware SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Global message buffers shared by Serial and Scrolling functions
#define BUF_SIZE 128
bool firstRun = true;
char curMessage[BUF_SIZE] = { "..N/A.." };
char newMessage[BUF_SIZE] = { "..N/A.." };
bool newMessageAvailable = true;
char dataValue[BUF_SIZE] = { "..N/A.." };
char dataIndex[4] = { "RPM" };

//Pacman sprite for startup screen (from MD_Parola lib defaults)
const uint8_t F_PMAN = 6;
const uint8_t W_PMAN = 18;
static const uint8_t PROGMEM pacman[F_PMAN * W_PMAN] =  // ghost pursued by a pacman
  {
    0x00,
    0x81,
    0xc3,
    0xe7,
    0xff,
    0x7e,
    0x7e,
    0x3c,
    0x00,
    0x00,
    0x00,
    0xfe,
    0x7b,
    0xf3,
    0x7f,
    0xfb,
    0x73,
    0xfe,
    0x00,
    0x42,
    0xe7,
    0xe7,
    0xff,
    0xff,
    0x7e,
    0x3c,
    0x00,
    0x00,
    0x00,
    0xfe,
    0x7b,
    0xf3,
    0x7f,
    0xfb,
    0x73,
    0xfe,
    0x24,
    0x66,
    0xe7,
    0xff,
    0xff,
    0xff,
    0x7e,
    0x3c,
    0x00,
    0x00,
    0x00,
    0xfe,
    0x7b,
    0xf3,
    0x7f,
    0xfb,
    0x73,
    0xfe,
    0x3c,
    0x7e,
    0xff,
    0xff,
    0xff,
    0xff,
    0x7e,
    0x3c,
    0x00,
    0x00,
    0x00,
    0xfe,
    0x73,
    0xfb,
    0x7f,
    0xf3,
    0x7b,
    0xfe,
    0x24,
    0x66,
    0xe7,
    0xff,
    0xff,
    0xff,
    0x7e,
    0x3c,
    0x00,
    0x00,
    0x00,
    0xfe,
    0x73,
    0xfb,
    0x7f,
    0xf3,
    0x7b,
    0xfe,
    0x00,
    0x42,
    0xe7,
    0xe7,
    0xff,
    0xff,
    0x7e,
    0x3c,
    0x00,
    0x00,
    0x00,
    0xfe,
    0x73,
    0xfb,
    0x7f,
    0xf3,
    0x7b,
    0xfe,
  };

// Function prototypes for Navigation/Display
bool display(MD_Menu::userDisplayAction_t, char*);
MD_Menu::userNavAction_t navigation(uint16_t& incDelta);
MD_Menu::value_t* mnuValueRqst(MD_Menu::mnuId_t id, bool bGet);

// Menu definition
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] = {
  { 8, "Menu>>>", 8, 11, 0 },
};

const PROGMEM MD_Menu::mnuItem_t mnuItm[] = {
  { 9, "Value", MD_Menu::MNU_INPUT, 9 },
  { 10, "Pos", MD_Menu::MNU_INPUT, 10 },
  { 11, "Brt", MD_Menu::MNU_INPUT, 11 },
};

//Car params menu
const PROGMEM char listI[] = "RPM|TMP|LMD|SPD|LCT|TM1|TM2|DST|VE1|ENR|IET|FAN";
//Text align options
const PROGMEM char listAlign[] = "L|C|R";

const PROGMEM MD_Menu::mnuInput_t mnuInp[] = {
  { 9, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listI },
  { 10, "P", MD_Menu::INP_LIST, mnuValueRqst, 1, 0, 0, 0, 0, 0, listAlign },
  { 11, "B", MD_Menu::INP_INT, mnuValueRqst, 2, 0, 0, 15, 0, 10, nullptr },
};

// Menu global object
MD_Menu M(navigation, display,          // user navigation and display
          mnuHdr, ARRAY_SIZE(mnuHdr),   // menu header data
          mnuItm, ARRAY_SIZE(mnuItm),   // menu item data
          mnuInp, ARRAY_SIZE(mnuInp));  // menu input data


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

//Process MQTT message receive
void messageReceived(String& topic, String& payload) {
  // Convert String to char array
  strncpy(newMessage, payload.c_str(), sizeof(newMessage) - 1);
  newMessage[sizeof(newMessage) - 1] = '\0';  // Ensure null-termination
  newMessageAvailable = true;
}

// Menu system callback functions
MD_Menu::value_t* mnuValueRqst(MD_Menu::mnuId_t id, bool bGet) {
  static MD_Menu::value_t v;

  switch (id) {

    case 9:  // Values
      if (bGet) {
        if (dataIndex == "RPM") v.value = 0;
        else if (dataIndex == "TMP") v.value = 1;
        else if (dataIndex == "LMD") v.value = 2;
        else if (dataIndex == "SPD") v.value = 3;
        else if (dataIndex == "LCT") v.value = 4;
        else if (dataIndex == "TM1") v.value = 5;
        else if (dataIndex == "TM2") v.value = 6;
        else if (dataIndex == "DST") v.value = 7;
        else if (dataIndex == "VE1") v.value = 8;
        else if (dataIndex == "ENR") v.value = 9;
        else if (dataIndex == "IET") v.value = 10;
        else if (dataIndex == "FAN") v.value = 11;
      } else {
        if (v.value == 0) {
          strcpy(dataIndex, "RPM");
        } else if (v.value == 1) {
          strcpy(dataIndex, "TMP");
        } else if (v.value == 2) {
          strcpy(dataIndex, "LMD");
        } else if (v.value == 3) {
          strcpy(dataIndex, "SPD");
        } else if (v.value == 4) {
          strcpy(dataIndex, "LCT");
        } else if (v.value == 5) {
          strcpy(dataIndex, "TM1");
        } else if (v.value == 6) {
          strcpy(dataIndex, "TM2");
        } else if (v.value == 7) {
          strcpy(dataIndex, "DST");
        } else if (v.value == 8) {
          strcpy(dataIndex, "VE1");
        } else if (v.value == 9) {
          strcpy(dataIndex, "ENR");
        } else if (v.value == 10) {
          strcpy(dataIndex, "IET");
        } else if (v.value == 11) {
          strcpy(dataIndex, "FAN");
        }
        //subscribe to chosen MQTT topic
        mqtt.subscribe("/GOLF86/ECU/" + String(dataIndex));
        Serial.println("\nSubscribed to topic /GOLF86/ECU/" + String(dataIndex));
        //set initial display valu
        strcpy(newMessage, "---");
        newMessageAvailable = true;
      }
      break;
      newMessageAvailable = true;
    case 10:  // Align
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
        P.setTextAlignment(Config.align);
      }
      break;

    case 11:  // Bright
      if (bGet)
        v.value = Config.bright;
      else {
        Config.bright = v.value;
        P.setIntensity(Config.bright);
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

// Display callback function
bool display(MD_Menu::userDisplayAction_t action, char* msg) {
  switch (action) {
    case MD_Menu::DISP_INIT:
      // nothing to do
      break;

    case MD_Menu::DISP_CLEAR:
      P.displayClear();
      break;

    case MD_Menu::DISP_L0:
      // Only one zone, no line 0
      P.print(msg);
      break;

    case MD_Menu::DISP_L1:
      P.print(msg);
      break;
  }

  return true;
}

void setup() {

  Serial.begin(115200);
  prefs.begin("G86-INFO", false);

  setupWifi();
  paramLoad();
  setupMqtt();
  setupNav();

  //Menu
  M.begin();
  M.setAutoStart(true);
  M.setTimeout(MENU_TIMEOUT);

  //Display
  P.begin();
  P.setIntensity(Config.bright);

  // Seed for random number generation
  srand(time(NULL));
}

//DEBUG
unsigned long previousMillis = 0;
const int rpmGenerationInterval = 200;  // Set the interval in milliseconds
//END DEBUG

void loop() {

  mqtt.loop();

  static bool wasInMenu = true;  // ensure we initialize the display first

  if (wasInMenu && !M.isInMenu())  // was running but not anymore
  {
    // Reset the display to show the message
    P.displayClear();

    if (firstRun) {
      P.setSpriteData(pacman, W_PMAN, F_PMAN, pacman, W_PMAN, F_PMAN);
      P.displayText("Golf'86", Config.align, 100, 3000, PA_OPENING_CURSOR, PA_SPRITE);
    } else {
      P.displayText(curMessage, Config.align, 1, 150, PA_PRINT, PA_PRINT);
    }

    wasInMenu = false;
  }

  wasInMenu = M.isInMenu();  // save the current status
  M.runMenu();               // run or autostart the menu

  if (!M.isInMenu())  // not running the menu? do something else
  {
    unsigned long currentMillis = millis();

    // Check if the interval has passed
    if (currentMillis - previousMillis >= rpmGenerationInterval) {
      previousMillis = currentMillis;  // Save the current time for the next interval

      // Generate a random RPM value between 500 and 2500
      int randomRPM = rand() % (1000 - 1150 + 1) + 1000;

      // Push to MQTT for tests
      mqtt.publish("/GOLF86/ECU/RPM", String(randomRPM));
    }


    // animate the display and check for a new message if ended
    if (P.displayAnimate()) {
      firstRun = false;

      if (newMessageAvailable) {
        strcpy(curMessage, newMessage);
        newMessageAvailable = false;
      }
      P.displayReset();
    }
  }
}
