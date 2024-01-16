// Menu.h

#ifndef TIMER_H
#define TIMER_H

#include <MD_UISwitch.h>      // Physical switch lib https://github.com/MajicDesigns/MD_UISwitch
#include "LedController.hpp"  // 7 segment display driver https://github.com/noah1510/LedController


#define DIN 5
#define CS 4
#define CLK 15

/*
 Now we need a LedController Variable to work with.
 We have only a single MAX72XX so the Dimensions are 1,1.
 */
LedController<1, 1> timerDisplay;
unsigned long delaytime = 250;

/*
 This method will continuously scroll the characters "GOLF 86"
 from left to right on the 8-digit display.
*/
void scrollGolf86On7Segment() {
  const char golf86[] = "GOLF 86";
  const int numDigits = 8;

  while (true) {
    for (int i = 0; i < strlen(golf86) + numDigits; i++) {
      // Display the "GOLF 86" text in a loop
      for (int j = 0; j < numDigits; j++) {
        char displayChar = (i - j >= 0 && i - j < strlen(golf86)) ? golf86[i - j] : ' ';  // Display space before "GOLF 86"
        timerDisplay.setChar(0, j, displayChar, false);
      }

      vTaskDelay(delaytime);
    }

    // Pause for a moment before starting the loop again
    vTaskDelay(1000);
  }
}


#endif  // TIMER_H
