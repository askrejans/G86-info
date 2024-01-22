// Menu.cpp
#include "Menu.h"

// Rotary switch and button initialization
const uint8_t RE_A_PIN = 26; ///< Port for rotary switch A channel
const uint8_t RE_B_PIN = 25; ///< Port for rotary switch B channel
const uint8_t CTL_PIN = 27;  ///< Port for the main input button

// Initialize rotary encoder
MD_REncoder RE(RE_A_PIN, RE_B_PIN);
 
// Initialize switch library for the main input button
MD_UISwitch_Digital swCtl(CTL_PIN);

// Transform helper variables
char dataIndex[] = "RPM";
char dataIndex2[] = "RPM";

/**
 * @file MyMenu.cpp
 * @brief Implementation file for the menu system with rotary encoder, switch, and display functionality.
 */

// Menu definition

// Header data for the menu
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] = {
    {1, "Menu>>>", 1, 7, 0},
};

// Menu item data for ECU, GPS, POS, and BRT options
const PROGMEM MD_Menu::mnuItem_t mnuItm[] = {
    {2, "P:ECU", MD_Menu::MNU_INPUT, 2},
    {3, "P:GPS", MD_Menu::MNU_INPUT, 3},
    {4, "S:ECU", MD_Menu::MNU_INPUT, 4},
    {5, "S:GPS", MD_Menu::MNU_INPUT, 5},
    {6, "POS", MD_Menu::MNU_INPUT, 6},
    {7, "BRT", MD_Menu::MNU_INPUT, 7},
};

// Mapping of 3-letter values to their corresponding parameters in the Speeduino ECU data
// Detailed explanation provided in comments:

/*
   Mapping of 3-letter values to their corresponding parameters in the Speeduino ECU data.

RPM: Engine revolutions per minute
TPS: Throttle Position Sensor reading (0% to 100%)
VE1: Volumetric Efficiency (%)
O2P: Primary O2 sensor reading
AFT: Air-Fuel Ratio Target
MAT: Manifold Air Temperature sensor reading
CAD: Coolant Analog-to-Digital Conversion value
MAP: Manifold Absolute Pressure sensor reading
BAT: Battery voltage (scaled by 10)
ADV: Ignition Advance
PW1: Pulse Width 1
SPK: Spark
DWL: Dwell time
ILL: Idle Load
BAR: Barometric Pressure
TAE: Warm-Up Enrichment Correction (%)
NER: Next Error code
ENG: Engine status
*/

const PROGMEM char listECU[] = "RPM|TPS|VE1|O2P|AFT|MAT|CAD|MAP|BAT|ADV|PW1|SPK|DWL|ILL|BAR|TAE|NER|ENG";

// GPS parameters menu
const PROGMEM char listGPS[] = "SPD|TME|DTE|LAT|LNG|ALT|CRS|QTY";

// Text alignment options
const PROGMEM char listAlign[] = "L|C|R";

// Menu input data for ECU, GPS, and alignment options
const PROGMEM MD_Menu::mnuInput_t mnuInp[] = {
    {2, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listECU},
    {3, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listGPS},
    {4, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listECU},
    {5, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listGPS},
    {6, "P", MD_Menu::INP_LIST, mnuValueRqst, 1, 0, 0, 0, 0, 0, listAlign},
    {7, "B", MD_Menu::INP_INT, mnuValueRqst, 2, 0, 0, 15, 0, 10, nullptr},
};

// Menu global object
MD_Menu M(navigation, display,         // user navigation and display
          mnuHdr, ARRAY_SIZE(mnuHdr),  // menu header data
          mnuItm, ARRAY_SIZE(mnuItm),  // menu item data
          mnuInp, ARRAY_SIZE(mnuInp)); // menu input data

// Navigation callback function
MD_Menu::userNavAction_t navigation(uint16_t &incDelta)
{
  uint8_t re = RE.read();

  if (re != DIR_NONE)
  {
    if (M.isInEdit())
      incDelta = 1 << abs(RE.speed() >> 3);
    return re == DIR_CCW ? MD_Menu::NAV_DEC : MD_Menu::NAV_INC;
  }

  switch (swCtl.read())
  {
  case MD_UISwitch::KEY_PRESS:
    return MD_Menu::NAV_SEL;
  case MD_UISwitch::KEY_LONGPRESS:
    return MD_Menu::NAV_ESC;
  }

  return MD_Menu::NAV_NULL;
}

// Setup navigation with rotary encoder and switch
void setupNav(void)
{
  RE.begin();
  swCtl.begin();
  swCtl.enableRepeat(false);
  swCtl.enableLongPress(true);
  swCtl.setPressTime(500);
  swCtl.setLongPressTime(1000);
  swCtl.enableDoublePress(false);
}

// String array for ECU and GPS Data parameters
const String ecuDataStrings[] = {"RPM", "TPS", "VE1", "O2P", "AFT", "MAT", "CAD", "MAP", "BAT", "ADV", "PW1", "SPK", "DWL", "ILL", "BAR", "TAE", "NER", "ENG"};
const String gpsDataStrings[] = {"SPD", "TME", "DTE", "LAT", "LNG", "ALT", "CRS", "QTY"};

// Menu system callback functions
MD_Menu::value_t *mnuValueRqst(MD_Menu::mnuId_t id, bool bGet)
{
  static MD_Menu::value_t v;

  switch (id)
  {

  case 2: // ECU Values
    if (!bGet)
    {
      mqttSetup.mqtt.unsubscribe(MQTT_ECU_TOPIC + String(dataIndex));
      mqttSetup.mqtt.unsubscribe(MQTT_GPS_TOPIC + String(dataIndex));
    }

    if (bGet)
    {
      // Convert dataIndex to index and set the value
      for (int i = 0; i < sizeof(ecuDataStrings) / sizeof(ecuDataStrings[0]); ++i)
      {
        if (String(dataIndex) == ecuDataStrings[i])
        {
          v.value = i;
          break;
        }
      }
    }
    else
    {
      strncpy(dataIndex, ecuDataStrings[v.value].c_str(), sizeof(dataIndex));
      dataIndex[sizeof(dataIndex) - 1] = '\0'; // Ensure null-termination

      // subscribe to the chosen MQTT topic
      mqttSetup.mqtt.subscribe(MQTT_ECU_TOPIC + String(dataIndex));
      Serial.println("\nSubscribed to primary topic: " + MQTT_ECU_TOPIC + String(dataIndex));
      // set the initial display value
      strcpy(newMessage, "---");
      newMessageAvailable = true;
    }
    break;
  case 3: // GPS Values
    if (!bGet)
    {
      mqttSetup.mqtt.unsubscribe(MQTT_ECU_TOPIC + String(dataIndex));
      mqttSetup.mqtt.unsubscribe(MQTT_GPS_TOPIC + String(dataIndex));
    }
    if (bGet)
    {
      for (int i = 0; i < sizeof(gpsDataStrings) / sizeof(gpsDataStrings[0]); ++i)
      {
        if (String(dataIndex) == gpsDataStrings[i])
        {
          v.value = i;
          break;
        }
      }
    }
    else
    {
      strncpy(dataIndex, gpsDataStrings[v.value].c_str(), sizeof(dataIndex));
      dataIndex[sizeof(dataIndex) - 1] = '\0'; // Ensure null-termination

      // subscribe to the chosen MQTT topic
      mqttSetup.mqtt.subscribe(MQTT_GPS_TOPIC + String(dataIndex));

      Serial.println("\nSubscribed to primary topic: " + MQTT_GPS_TOPIC + String(dataIndex));
      // set the initial display value
      strcpy(newMessage, "---");
      newMessageAvailable = true;
    }
    break;
  case 4: // ECU Values
    if (!bGet)
    {
      mqttSetup.mqtt2.unsubscribe(MQTT_ECU_TOPIC + String(dataIndex2));
      mqttSetup.mqtt2.unsubscribe(MQTT_GPS_TOPIC + String(dataIndex2));
    }

    if (bGet)
    {
      // Convert dataIndex to index and set the value
      for (int i = 0; i < sizeof(ecuDataStrings) / sizeof(ecuDataStrings[0]); ++i)
      {
        if (String(dataIndex2) == ecuDataStrings[i])
        {
          v.value = i;
          break;
        }
      }
    }
    else
    {
      strncpy(dataIndex2, ecuDataStrings[v.value].c_str(), sizeof(dataIndex2));
      dataIndex2[sizeof(dataIndex2) - 1] = '\0'; // Ensure null-termination

      // subscribe to the chosen MQTT topic
      mqttSetup.mqtt2.subscribe(MQTT_ECU_TOPIC + String(dataIndex2));
      strcpy(const_cast<char *>(secondaryScreenMode), "MQTT");
      Serial.println("\nSubscribed to secondary topic: " + MQTT_ECU_TOPIC + String(dataIndex2));
      // set the initial display value
      strcpy(const_cast<char *>(newMessage2), "---");
      newMessageAvailable2 = true;
    }
    break;
  case 5: // GPS Values
    if (!bGet)
    {
      mqttSetup.mqtt2.unsubscribe(MQTT_ECU_TOPIC + String(dataIndex2));
      mqttSetup.mqtt2.unsubscribe(MQTT_GPS_TOPIC + String(dataIndex2));
    }
    if (bGet)
    {
      for (int i = 0; i < sizeof(gpsDataStrings) / sizeof(gpsDataStrings[0]); ++i)
      {
        if (String(dataIndex2) == gpsDataStrings[i])
        {
          v.value = i;
          break;
        }
      }
    }
    else
    {
      strncpy(dataIndex2, gpsDataStrings[v.value].c_str(), sizeof(dataIndex2));
      dataIndex2[sizeof(dataIndex2) - 1] = '\0'; // Ensure null-termination

      // subscribe to the chosen MQTT topic
      mqttSetup.mqtt2.subscribe(MQTT_GPS_TOPIC + String(dataIndex2));
      strcpy(const_cast<char *>(secondaryScreenMode), "MQTT");
      Serial.println("\nSubscribed to secondary topic: " + MQTT_GPS_TOPIC + String(dataIndex2));
      // set the initial display value
      strcpy(const_cast<char *>(newMessage2), "---");
      newMessageAvailable2 = true;
    }
    break;
  case 6: // Text align
    if (bGet)
    {
      switch (wifiSetup.config.align)
      {
      case PA_LEFT:
        v.value = 0;
        break;
      case PA_CENTER:
        v.value = 1;
        break;
      case PA_RIGHT:
        v.value = 2;
        break;
      }
    }
    else
    {
      switch (v.value)
      {
      case 0:
        wifiSetup.config.align = PA_LEFT;
        break;
      case 1:
        wifiSetup.config.align = PA_CENTER;
        break;
      case 2:
        wifiSetup.config.align = PA_RIGHT;
        break;
      }
      mainDisplay.setTextAlignment(wifiSetup.config.align);
    }
    break;

  case 7: // Screen brightness
    if (bGet)
      v.value = wifiSetup.config.bright;
    else
    {
      wifiSetup.config.bright = v.value;
      mainDisplay.setIntensity(wifiSetup.config.bright);
    }
    break;
  }

  // if things were requested, return the buffer
  if (bGet)
    return &v;
  else // save the parameters
    wifiSetup.paramSave();

  return nullptr;
}

// Display callback function
bool display(MD_Menu::userDisplayAction_t action, char *msg)
{
  switch (action)
  {
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