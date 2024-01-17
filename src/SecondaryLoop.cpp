#include "SecondaryLoop.h"

LedController<1, 1> secondaryDisplay; // Secondary 7-segment LED display
unsigned long delaytime = 250;        // Delay time for scrolling animation

/*
 This method will continuously scroll the characters "GOLF 86"
 from left to right on the 8-digit display.
*/
void scrollGolf86On7Segment()
{
  const char golf86[] = "GOLF'86"; // Text to be scrolled
  const int numDigits = 8;          // Number of digits on the display

  for (int i = 0; i < strlen(golf86) + numDigits; i++)
  {
    // Display the "GOLF 86" text in a loop
    for (int j = 0; j < numDigits; j++)
    {
      // Calculate the character to be displayed
      char displayChar = (i - j >= 0 && i - j < strlen(golf86)) ? golf86[i - j] : ' '; // Display space before "GOLF 86"
      secondaryDisplay.setChar(0, j, displayChar, false);
    }

    // Delay before scrolling to the next position
    vTaskDelay(delaytime);
  }

  // Pause for a moment before starting the loop again
  vTaskDelay(1000);
}

/*
 Display the given text on the 7-segment LED display.
*/
void showText(const char *text)
{
  const int numDigits = 8; // Number of digits on the display
  int textLength = strlen(text);

  // Display each character of the text on the LED display
  for (int j = 0; j < numDigits; j++)
  {
    char displayChar = (j < textLength) ? text[j] : ' '; // Display space if the text is shorter than the number of digits
    secondaryDisplay.setChar(0, j, displayChar, false);
  }
}

// Secondary loop - Task to update the secondary display with MQTT received message
void secondaryDisplayLoop(void *parameter)
{
  volatile const char *tmpSecondaryScreenMode;

  while (1)
  {
    // Check the current secondary screen mode
    if (strcmp(const_cast<char *>(secondaryScreenMode), "WELCOME") == 0)
    {
      // Display the welcome message before showing MQTT messages
      scrollGolf86On7Segment();
    }
    else if (strcmp(const_cast<char *>(secondaryScreenMode), "MQTT") == 0)
    {
      // Show the MQTT message on the display if available
      if (newMessageAvailable2)
      {
        showText(const_cast<char *>(newMessage2));
        newMessageAvailable2 = false;
      }
    }

    // Add a delay to avoid a busy loop
    vTaskDelay(10);
  }
}
