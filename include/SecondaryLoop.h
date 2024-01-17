// SECONDARY_LOOP_H

#ifndef SECONDARY_LOOP_H
#define SECONDARY_LOOP_H

#include <MD_UISwitch.h>
#include "LedController.hpp"

#define DIN 5
#define CS 4
#define CLK 15

extern LedController<1, 1> secondaryDisplay;
extern unsigned long delaytime;

#define TXT_BUF_SIZE 128

// Variables declared as extern for access in other source files
extern volatile char secondaryScreenMode[];
extern volatile bool newMessageAvailable2;
extern volatile char newMessage2[TXT_BUF_SIZE];

void scrollGolf86On7Segment();
void showText(const char *text);
void secondaryDisplayLoop(void *parameter);

#endif  // SECONDARY_LOOP_H
