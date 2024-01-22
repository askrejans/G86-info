#include "SecondaryLoop.h"

LedController<1, 1> secondaryDisplay; // Secondary 7-segment LED display
unsigned long delaytime = 250;        // Delay time for scrolling animation

// Define variables to store timer values
volatile unsigned long timer1Value = 0;
volatile unsigned long timer2Value = 0;
bool timer1Started = false; // Flag to indicate if Timer 1 is started
bool timer2Started = false; // Flag to indicate if Timer 2 is started
TimerHandle_t timer1Handle;
TimerHandle_t timer2Handle;

/**
 * @brief Scrolls the characters "GOLF 86" from left to right on the 8-digit display.
 */
void scrollGolf86On7Segment()
{
  const int numDigits = 8; // Number of digits on the display

  for (int i = 0; i < strlen(WELCOME_MSG2) + numDigits; i++)
  {
    // Display the "GOLF 86" text in a loop
    for (int j = 0; j < numDigits; j++)
    {
      // Calculate the character to be displayed
      char displayChar = (i - j >= 0 && i - j < strlen(WELCOME_MSG2)) ? WELCOME_MSG2[i - j] : ' '; // Display space before "GOLF 86"
      secondaryDisplay.setChar(0, j, displayChar, false);
    }

    // Delay before scrolling to the next position
    vTaskDelay(delaytime);
  }

  // Pause for a moment before starting the loop again
  vTaskDelay(1000);
}

/**
 * @brief Displays the given text on the 7-segment LED display.
 * @param text The text to be displayed.
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

/**
 * @brief Task to update the secondary display with an MQTT received message.
 * @param parameter Unused parameter required by the FreeRTOS task signature.
 */
void secondaryDisplayLoop(void *parameter)
{
  volatile const char *tmpSecondaryScreenMode;

  while (1)
  {
    if (strcmp(const_cast<char *>(secondaryScreenMode), "WELCOME") == 0)
    {
      scrollGolf86On7Segment();
    }
    else if (strcmp(const_cast<char *>(secondaryScreenMode), "MQTT") == 0)
    {
      if (newMessageAvailable2)
      {
        showText(const_cast<char *>(newMessage2));
        newMessageAvailable2 = false;
      }
    }
    else if (strcmp(const_cast<char *>(secondaryScreenMode), "TIMER1") == 0)
    {
      int hours, minutes, seconds, hundredths;
      convertTimerToTime(timer1Value, hours, minutes, seconds, hundredths);
      displayTime(hours, minutes, seconds, hundredths);
    }
    else if (strcmp(const_cast<char *>(secondaryScreenMode), "TIMER2") == 0)
    {
      int hours, minutes, seconds, hundredths;
      convertTimerToTime(timer2Value, hours, minutes, seconds, hundredths);
      displayTime(hours, minutes, seconds, hundredths);
    }

    vTaskDelay(10);
  }
}

/**
 * @brief Callback function for timer 1.
 * @param xTimer The timer handle.
 */
void timer1Callback(TimerHandle_t xTimer)
{
  timer1Value += 10;
  int hours, minutes, seconds, hundredths;
  convertTimerToTime(timer1Value, hours, minutes, seconds, hundredths);

  if (strcmp(const_cast<char *>(secondaryScreenMode), "TIMER1") == 0)
  {
    displayTime(hours, minutes, seconds, hundredths);
  }

  static int counter = 0;

  // Check if 100ms has elapsed
  if (counter == 10)
  {
    setTimeToMqtt(1, hours, minutes, seconds, hundredths);
    counter = 0; // Reset the counter
  }
  else
  {
    counter++;
  }
}

/**
 * @brief Callback function for timer 2.
 * @param xTimer The timer handle.
 */
void timer2Callback(TimerHandle_t xTimer)
{
  timer2Value += 10;

  int hours, minutes, seconds, hundredths;
  convertTimerToTime(timer2Value, hours, minutes, seconds, hundredths);

  if (strcmp(const_cast<char *>(secondaryScreenMode), "TIMER2") == 0)
  {
    displayTime(hours, minutes, seconds, hundredths);
  }

  static int counter = 0;

  // Check if 100ms has elapsed
  if (counter == 10)
  {
    setTimeToMqtt(2, hours, minutes, seconds, hundredths);
    counter = 0; // Reset the counter
  }
  else
  {
    counter++;
  }
}

/**
 * @brief FreeRTOS task to handle timer 1 chronometer functionality.
 * @param parameter Unused parameter required by the FreeRTOS task signature.
 */
void timer1Chronometer(void *parameter)
{
  while (strcmp(const_cast<char *>(secondaryScreenMode), "TIMER1") != 0)
  {
    vTaskDelay(10);
  }

  timer1Handle = xTimerCreate("Timer1", pdMS_TO_TICKS(10), pdTRUE, nullptr, timer1Callback);
  xTimerStart(timer1Handle, 0);

  vTaskDelete(nullptr);
}

/**
 * @brief FreeRTOS task to handle timer 2 chronometer functionality.
 * @param parameter Unused parameter required by the FreeRTOS task signature.
 */
void timer2Chronometer(void *parameter)
{
  while (strcmp(const_cast<char *>(secondaryScreenMode), "TIMER2") != 0)
  {
    vTaskDelay(10);
  }

  timer2Handle = xTimerCreate("Timer2", pdMS_TO_TICKS(10), pdTRUE, nullptr, timer2Callback);
  xTimerStart(timer2Handle, 0);

  vTaskDelete(nullptr);
}

/**
 * @brief Convert timer value to hours, minutes, seconds, and hundredths of seconds.
 * @param timerValue The timer value in milliseconds.
 * @param hours Reference to store the calculated hours.
 * @param minutes Reference to store the calculated minutes.
 * @param seconds Reference to store the calculated seconds.
 * @param hundredths Reference to store the calculated hundredths of seconds.
 */
void convertTimerToTime(unsigned long timerValue, int &hours, int &minutes, int &seconds, int &hundredths)
{
  unsigned long totalSeconds = timerValue / 1000;

  hundredths = (timerValue / 10) % 100;
  seconds = totalSeconds % 60;
  minutes = (totalSeconds / 60) % 60;
  hours = (totalSeconds / 3600) % 24;
}

/**
 * @brief Display time on the 7-segment LED display.
 * @param hours Hours to display.
 * @param minutes Minutes to display.
 * @param seconds Seconds to display.
 * @param hundredths Hundredths of seconds to display.
 */
void displayTime(int hours, int minutes, int seconds, int hundredths)
{
  const int numDigits = 8;
  char timeText[9];

  if (hours > 0)
  {
    snprintf(timeText, sizeof(timeText), "%02d-%02d-%02d", hours, minutes, seconds);
  }
  else
  {
    snprintf(timeText, sizeof(timeText), "%02d-%02d-%02d", minutes, seconds, hundredths);
  }

  // Ensure null-termination
  timeText[sizeof(timeText) - 1] = '\0';

  // Display each character of the time on the LED display
  for (int j = 0; j < numDigits; j++)
  {
    char displayChar = (j < strlen(timeText)) ? timeText[strlen(timeText) - j - 1] : ' ';
    secondaryDisplay.setChar(0, j, displayChar, false);
  }
}

/**
 * @brief Pushes time to MQTT
 * @param timer Timer nr.
 * @param hours Hours to display.
 * @param minutes Minutes to display.
 * @param seconds Seconds to display.
 * @param hundredths Hundredths of seconds to display.
 */
void setTimeToMqtt(int timer, int hours, int minutes, int seconds, int hundredths)
{
  char timeText[12];

  snprintf(timeText, sizeof(timeText), "%02d-%02d-%02d:%02d", hours, minutes, seconds, hundredths);

  if (timer == 1)
  {
    mqttSetup.mqtt.publish(MQTT_TIMER1_TOPIC + String("value"), timeText);
  }
  else if (timer == 2)
  {
    mqttSetup.mqtt.publish(MQTT_TIMER2_TOPIC + String("value"), timeText);
  }
}