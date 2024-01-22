// Menu.h

#ifndef MENU_H
#define MENU_H

#include <MD_Menu.h>      // Settings menu https://github.com/MajicDesigns/MD_Menu
#include <MD_REncoder.h>  // Rotary encoder lib https://github.com/MajicDesigns/MD_REncoder
#include <MD_UISwitch.h>  // Physical switch lib https://github.com/MajicDesigns/MD_UISwitch
#include <MD_Parola.h>    // Display library https://github.com/MajicDesigns/MD_Parola
#include <MD_MAX72xx.h>   // Display driver https://github.com/MajicDesigns/MD_MAX72XX

#include "WiFiSetup.h"
#include "MqttSetup.h"

/**
 * @file Menu.h
 * @brief Header file for the menu system with rotary encoder, switch, and display functionality.
 */

extern MD_Menu M;  ///< Global instance of MD_Menu for menu navigation.

/**
 * @brief Navigation function for the menu system.
 *
 * This function handles the navigation actions for the menu system based on the
 * rotary encoder input.
 *
 * @param incDelta The change in the encoder value.
 * @return MD_Menu::userNavAction_t The user navigation action.
 */
MD_Menu::userNavAction_t navigation(uint16_t& incDelta);

/**
 * @brief Display function for the menu system.
 *
 * This function handles the display actions for the menu system.
 *
 * @param action The user display action.
 * @param msg The message to display.
 * @return bool True if the display action is successful, false otherwise.
 */
bool display(MD_Menu::userDisplayAction_t action, char* msg);

/**
 * @brief Request menu value function for the menu system.
 *
 * This function is used to request the value of a menu item.
 *
 * @param id The menu ID.
 * @param bGet Boolean indicating whether to get or set the value.
 * @return MD_Menu::value_t* Pointer to the menu value.
 */
MD_Menu::value_t* mnuValueRqst(MD_Menu::mnuId_t id, bool bGet);

void setupNav(void);
extern MD_Parola mainDisplay;
extern WiFiSetup wifiSetup;
extern MqttSetup mqttSetup;
extern const String MQTT_ECU_TOPIC;
extern const String MQTT_GPS_TOPIC;
extern volatile char secondaryScreenMode[];

#endif  // MENU_H
