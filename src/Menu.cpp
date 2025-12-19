// Menu.cpp
#include "Menu.h"
#include "SharedData.h"
#include "Constants.h"

// Rotary switch and button initialization (using centralized constants)
const uint8_t RE_A_PIN = MENU_ROTARY_A_PIN; ///< Port for rotary switch A channel
const uint8_t RE_B_PIN = MENU_ROTARY_B_PIN; ///< Port for rotary switch B channel
const uint8_t CTL_PIN = MENU_BUTTON_PIN;     ///< Port for the main input button

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
    {1, "Menu>>>", 1, 7, 1},
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
    // If it's not clockwise or counterclockwise, return increment
    if (re != DIR_CW && re != DIR_CCW)
      return MD_Menu::NAV_INC;

    if (M.isInEdit())
    {
      // Adjust incDelta based on speed only in edit mode
      int speed = RE.speed();
      if (speed < 0)
        speed = -speed;
      incDelta = 1 << (speed >> 3);
    }
    else
    {
      // Outside edit mode, just move one item
      incDelta = 1;
    }
    return (re == DIR_CW) ? MD_Menu::NAV_INC : MD_Menu::NAV_DEC;
  }

  MD_UISwitch::keyResult_t keyResult = swCtl.read();
  switch (keyResult)
  {
  case MD_UISwitch::KEY_PRESS:
    return MD_Menu::NAV_SEL;
  case MD_UISwitch::KEY_LONGPRESS:
    return MD_Menu::NAV_ESC;
  default:
    break;
  }

  return MD_Menu::NAV_NULL;
}

/**
 * @brief Initializes the navigation system for the menu.
 *
 * This function sets up the rotary encoder and switch controller, configuring
 * various parameters such as repeat, long press, and double press timings. It
 * also enables menu wrapping.
 *
 * @note The following configurations are applied:
 * - Repeat is disabled.
 * - Long press is enabled with a press time of 500 ms and a long press time of 1000 ms.
 * - Double press is disabled.
 * - Menu wrapping is enabled.
 */
void setupNav(void)
{
  RE.begin();
  swCtl.begin();
  swCtl.enableRepeat(false);
  swCtl.enableLongPress(true);
  swCtl.setPressTime(500);
  swCtl.setLongPressTime(1000);
  swCtl.enableDoublePress(false);

  // Enable menu wrapping
  M.setMenuWrap(true);
}

// String array for ECU and GPS Data parameters
// Using const char* arrays instead of String to avoid heap allocations
const char* const ecuDataStrings[] = {"RPM", "TPS", "VE1", "O2P", "AFT", "MAT", "CAD", "MAP", "BAT", "ADV", "PW1", "SPK", "DWL", "ILL", "BAR", "TAE", "NER", "ENG"};
const char* const gpsDataStrings[] = {"SPD", "TME", "DTE", "LAT", "LNG", "ALT", "CRS", "QTY"};

// Helper to unsubscribe from both ECU and GPS topics
void unsubscribeAll(MQTTClient &client, const char *idx)
{
  // Build topic strings without String allocation
  char topic[64];
  
  snprintf(topic, sizeof(topic), "%s%s", MQTT_ECU_TOPIC, idx);
  client.unsubscribe(topic);
  
  snprintf(topic, sizeof(topic), "%s%s", MQTT_GPS_TOPIC, idx);
  client.unsubscribe(topic);
}

// Helper to update index from array
int findArrayIndex(const char* const arr[], size_t size, const char *val)
{
  if (val == nullptr) return 0;
  
  for (size_t i = 0; i < size; i++)
    if (strcmp(arr[i], val) == 0)
      return i;
  return 0;
}

/**
 * @brief Handles menu value requests for various menu IDs.
 *
 * This function processes menu value requests for different menu IDs, handling both
 * primary and secondary displays, as well as text alignment and brightness settings.
 *
 * @param id The menu ID to process.
 * @param bGet If true, the function retrieves the current value; if false, it sets the value.
 * @return MD_Menu::value_t* Pointer to the value structure if bGet is true, otherwise nullptr.
 *
 * The function uses two lambda functions to handle primary and secondary displays:
 * - handlePrimary: Handles non-volatile variables for the primary display.
 * - handleSecondary: Handles volatile variables for the secondary display.
 *
 * The function processes the following menu IDs:
 * - 2: ECU primary display
 * - 3: GPS primary display
 * - 4: ECU secondary display
 * - 5: GPS secondary display
 * - 6: Text alignment
 * - 7: Brightness
 *
 * For text alignment and brightness, the function directly modifies the display settings.
 * For other IDs, it subscribes to the appropriate MQTT topics and updates message availability.
 */
MD_Menu::value_t *mnuValueRqst(MD_Menu::mnuId_t id, bool bGet)
{
  static MD_Menu::value_t v;

  // For primary display (non-volatile vars) - optimized to avoid String allocations
  auto handlePrimary = [&](int arraySize,
                           const char* const arr[],
                           char *indexRef,
                           MQTTClient &client,
                           char *messageRef,
                           bool *msgAvail,
                           const char *topicBase)
  {
    if (!bGet)
      unsubscribeAll(client, indexRef);

    if (bGet)
    {
      v.value = findArrayIndex(arr, arraySize, indexRef);
      if (v.value < 0) v.value = 0; // Fallback to first item if not found
    }
    else
    {
      // Validate array index
      if (v.value < 0 || v.value >= arraySize) {
        Serial.printf("ERROR: Invalid array index %d (max: %d)\n", v.value, arraySize - 1);
        return;
      }
      
      strncpy(indexRef, arr[v.value], sizeof(dataIndex));
      indexRef[sizeof(dataIndex) - 1] = '\0';
      
      // Build topic without String allocation
      char fullTopic[64];
      snprintf(fullTopic, sizeof(fullTopic), "%s%s", topicBase, indexRef);
      client.subscribe(fullTopic);
      
      Serial.print("\nSubscribed to topic: ");
      Serial.println(fullTopic);
      
      strncpy(messageRef, "---", MESSAGE_BUFFER_SIZE - 1);
      messageRef[MESSAGE_BUFFER_SIZE - 1] = '\0';
      *msgAvail = true;
    }
  };

  // For secondary display (volatile vars) - optimized to avoid String allocations
  auto handleSecondary = [&](int arraySize,
                             const char* const arr[],
                             char *indexRef,
                             MQTTClient &client,
                             volatile char *messageRef,
                             volatile bool *msgAvail,
                             const char *topicBase)
  {
    if (!bGet)
      unsubscribeAll(client, indexRef);

    if (bGet)
    {
      v.value = findArrayIndex(arr, arraySize, indexRef);
      if (v.value < 0) v.value = 0; // Fallback to first item if not found
    }
    else
    {
      // Validate array index
      if (v.value < 0 || v.value >= arraySize) {
        Serial.printf("ERROR: Invalid secondary array index %d (max: %d)\n", v.value, arraySize - 1);
        return;
      }
      
      strncpy(indexRef, arr[v.value], sizeof(dataIndex2));
      indexRef[sizeof(dataIndex2) - 1] = '\0';
      
      // Build topic without String allocation
      char fullTopic[64];
      snprintf(fullTopic, sizeof(fullTopic), "%s%s", topicBase, indexRef);
      client.subscribe(fullTopic);
      
      // Thread-safe mode setting
      if (!g_secondaryMode.set("MQTT")) {
        Serial.println("WARNING: Failed to set secondary mode to MQTT");
        // Fallback to legacy volatile
        strncpy((char*)secondaryScreenMode, "MQTT", MODE_BUFFER_SIZE - 1);
        ((char*)secondaryScreenMode)[MODE_BUFFER_SIZE - 1] = '\0';
      }
      
      Serial.print("\nSubscribed to secondary: ");
      Serial.println(fullTopic);
      
      strncpy((char *)messageRef, "---", MESSAGE_BUFFER_SIZE - 1);
      ((char*)messageRef)[MESSAGE_BUFFER_SIZE - 1] = '\0';
      *msgAvail = true;
    }
  };

  switch (id)
  {
  case 2: // ECU primary
    handlePrimary(
        ARRAY_SIZE(ecuDataStrings),
        ecuDataStrings,
        dataIndex,
        mqttSetup.mqtt,
        newMessage,
        &newMessageAvailable,
        MQTT_ECU_TOPIC);
    break;

  case 3: // GPS primary
    handlePrimary(
        ARRAY_SIZE(gpsDataStrings),
        gpsDataStrings,
        dataIndex,
        mqttSetup.mqtt,
        newMessage,
        &newMessageAvailable,
        MQTT_GPS_TOPIC);
    break;

  case 4: // ECU secondary
    handleSecondary(
        ARRAY_SIZE(ecuDataStrings),
        ecuDataStrings,
        dataIndex2,
        mqttSetup.mqtt2,
        newMessage2,
        &newMessageAvailable2,
        MQTT_ECU_TOPIC);
    break;

  case 5: // GPS secondary
    handleSecondary(
        ARRAY_SIZE(gpsDataStrings),
        gpsDataStrings,
        dataIndex2,
        mqttSetup.mqtt2,
        newMessage2,
        &newMessageAvailable2,
        MQTT_GPS_TOPIC);
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

  case 7: // Brightness
    if (bGet)
    {
      v.value = wifiSetup.config.bright;
    }
    else
    {
      wifiSetup.config.bright = v.value;
      mainDisplay.setIntensity(wifiSetup.config.bright);
    }
    break;

  default:
    Serial.printf("ERROR: Unknown menu ID: %d\n", id);
    return nullptr;
  }

  if (bGet)
    return &v;
  wifiSetup.paramSave();
  return nullptr;
}

/**
 * @brief Handles the display actions for the menu.
 *
 * This function processes different display actions such as initialization,
 * clearing the display, and printing messages to specific lines.
 *
 * @param action The display action to be performed. It can be one of the following:
 *               - MD_Menu::DISP_INIT: Initialize the display (no action taken in this case).
 *               - MD_Menu::DISP_CLEAR: Clear the display.
 *               - MD_Menu::DISP_L0: Print a message on line 0.
 *               - MD_Menu::DISP_L1: Print a message on line 1.
 * @param msg The message to be displayed when the action is DISP_L0 or DISP_L1.
 * @return true Always returns true.
 */
bool display(MD_Menu::userDisplayAction_t action, char *msg)
{
  switch (action)
  {
  case MD_Menu::DISP_INIT:
    // nothing
    break;

  case MD_Menu::DISP_CLEAR:
    mainDisplay.displayClear();
    break;

  case MD_Menu::DISP_L0:
  case MD_Menu::DISP_L1:
    mainDisplay.print(msg);
    break;

  default:
    // optional default handling
    break;
  }
  return true;
}