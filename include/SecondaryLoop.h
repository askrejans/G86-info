// SECONDARY_LOOP_H

#ifndef SECONDARY_LOOP_H
#define SECONDARY_LOOP_H

#include <MD_UISwitch.h>
#include "LedController.hpp"

#define DIN 5
#define CS 4
#define CLK 15

// External declarations for variables
extern LedController<1, 1> secondaryDisplay;
extern unsigned long delaytime;
extern volatile char secondaryScreenMode[];
extern volatile bool newMessageAvailable2;
extern volatile char newMessage2[128];
extern const char *WELCOME_MSG2;
/**
 * @brief Scrolls the characters "GOLF 86" from left to right on the 8-digit display.
 */
void scrollGolf86On7Segment();

/**
 * @brief Displays the given text on the 7-segment LED display.
 * @param text The text to be displayed.
 */
void showText(const char *text);

/**
 * @brief Task to update the secondary display with an MQTT received message.
 * @param parameter Unused parameter required by the FreeRTOS task signature.
 */
void secondaryDisplayLoop(void *parameter);

#endif // SECONDARY_LOOP_H
