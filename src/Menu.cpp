// Menu.cpp
#include "Menu.h"

// Rotary switch and button initialization
const uint8_t RE_A_PIN = 25;  ///< Port for rotary switch A channel
const uint8_t RE_B_PIN = 26;  ///< Port for rotary switch B channel
const uint8_t CTL_PIN = 27;   ///< Port for the main input button

// Initialize rotary encoder
MD_REncoder RE(RE_A_PIN, RE_B_PIN);

// Initialize switch library for the main input button
MD_UISwitch_Digital swCtl(CTL_PIN);

/**
 * @file MyMenu.cpp
 * @brief Implementation file for the menu system with rotary encoder, switch, and display functionality.
 */

// Menu definition

// Header data for the menu
const PROGMEM MD_Menu::mnuHeader_t mnuHdr[] = {
  { 1, "Menu>>>", 1, 7, 0 },
};

// Menu item data for ECU, GPS, POS, and BRT options
const PROGMEM MD_Menu::mnuItem_t mnuItm[] = {
  { 2, "P:ECU", MD_Menu::MNU_INPUT, 2 },
  { 3, "P:GPS", MD_Menu::MNU_INPUT, 3 },
  { 4, "S:ECU", MD_Menu::MNU_INPUT, 4 },
  { 5, "S:GPS", MD_Menu::MNU_INPUT, 5 },
  { 6, "POS", MD_Menu::MNU_INPUT, 6 },
  { 7, "BRT", MD_Menu::MNU_INPUT, 7 },
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
  { 2, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listECU },
  { 3, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listGPS },
  { 4, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listECU },
  { 5, "", MD_Menu::INP_LIST, mnuValueRqst, 3, 0, 0, 0, 0, 0, listGPS },
  { 6, "P", MD_Menu::INP_LIST, mnuValueRqst, 1, 0, 0, 0, 0, 0, listAlign },
  { 7, "B", MD_Menu::INP_INT, mnuValueRqst, 2, 0, 0, 15, 0, 10, nullptr },
};

// Menu global object
MD_Menu M(navigation, display,          // user navigation and display
          mnuHdr, ARRAY_SIZE(mnuHdr),   // menu header data
          mnuItm, ARRAY_SIZE(mnuItm),   // menu item data
          mnuInp, ARRAY_SIZE(mnuInp));  // menu input data

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
