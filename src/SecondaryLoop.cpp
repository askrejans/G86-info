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
 * @brief Scrolls the "GOLF 86" message on a 7-segment display.
 *
 * This function scrolls the "GOLF 86" message across an 8-digit 7-segment display.
 * It displays the message in a loop, with a delay between each scroll step.
 * After the message has scrolled completely, there is a pause before the loop starts again.
 *
 * The function uses the WELCOME_MSG2 constant to determine the message to be displayed.
 * It calculates the character to be displayed at each position and updates the display accordingly.
 *
 * @note The function assumes that the secondaryDisplay object and the WELCOME_MSG2 constant are defined elsewhere in the code.
 * @note The delaytime variable is used to control the delay between each scroll step.
 */
void scrollGolf86On7Segment()
{
  const int numDigits = 8; // Number of digits on the display
  const int messageLength = strlen(WELCOME_MSG2);

  for (int i = 0; i < messageLength + numDigits; i++)
  {
    for (int j = 0; j < numDigits; j++)
    {
      int charIndex = i - j;
      char displayChar = (charIndex >= 0 && charIndex < messageLength) ? WELCOME_MSG2[charIndex] : ' ';
      secondaryDisplay.setChar(0, j, displayChar, false);
    }

    vTaskDelay(delaytime);
  }

  vTaskDelay(1000);
}

/**
 * @brief Displays a given text on an LED display.
 *
 * This function takes a string of text and displays each character on an LED display.
 * If the text is shorter than the number of digits on the display, the remaining digits
 * will be filled with spaces.
 *
 * @param text The text to be displayed. It should be a null-terminated string.
 */
void showText(const char *text)
{
  const int numDigits = 8; // Number of digits on the display
  int textLength = strlen(text);

  for (int j = 0; j < numDigits; j++)
  {
    char displayChar = (j < textLength) ? text[j] : ' '; // Display space if the text is shorter than the number of digits
    secondaryDisplay.setChar(0, j, displayChar, false);
  }
}

/**
 * @brief Task function to handle the secondary display loop.
 *
 * This function runs in an infinite loop and updates the secondary display
 * based on the current mode specified by the `secondaryScreenMode` variable.
 *
 * Modes:
 * - "WELCOME": Scrolls "Golf86" on a 7-segment display.
 * - "MQTT": Displays a new MQTT message if available.
 * - "TIMER1": Displays the time for timer1.
 * - "TIMER2": Displays the time for timer2.
 *
 * The function uses FreeRTOS's `vTaskDelay` to delay the loop by 10 ticks.
 *
 * @param parameter Pointer to the parameters passed to the task (unused).
 */
void secondaryDisplayLoop(void *parameter)
{
  while (1)
  {
    const char *mode = const_cast<char *>(secondaryScreenMode);

    if (strcmp(mode, "WELCOME") == 0)
    {
      scrollGolf86On7Segment();
    }
    else if (strcmp(mode, "MQTT") == 0)
    {
      if (newMessageAvailable2)
      {
        showText(const_cast<char *>(newMessage2));
        newMessageAvailable2 = false;
      }
    }
    else if (strcmp(mode, "TIMER1") == 0)
    {
      displayTimer(timer1Value);
    }
    else if (strcmp(mode, "TIMER2") == 0)
    {
      displayTimer(timer2Value);
    }

    vTaskDelay(10);
  }
}

/**
 * @brief Displays the timer value in a formatted time.
 *
 * This function converts the given timer value into hours, minutes, seconds,
 * and hundredths of a second, and then displays the formatted time.
 *
 * @param timerValue The timer value to be displayed, typically in hundredths of a second.
 */
void displayTimer(int timerValue)
{
  int hours, minutes, seconds, hundredths;
  convertTimerToTime(timerValue, hours, minutes, seconds, hundredths);
  displayTime(hours, minutes, seconds, hundredths);
}

/**
 * @brief Common function to handle timer callbacks.
 *
 * This function increments the timer value by 10, converts the timer value to hours,
 * minutes, seconds, and hundredths, and displays the time if the secondary screen mode
 * matches the provided mode. Additionally, it sends the time to MQTT every 100 milliseconds.
 *
 * @param timerValue Reference to the timer value.
 * @param mode The mode string to compare with the secondary screen mode.
 * @param timerId The ID of the timer for MQTT.
 */
void handleTimerCallback(volatile unsigned long &timerValue, const char *mode, int timerId)
{
  timerValue += 10;
  int hours, minutes, seconds, hundredths;
  convertTimerToTime(timerValue, hours, minutes, seconds, hundredths);

  if (strcmp(const_cast<char *>(secondaryScreenMode), mode) == 0)
  {
    displayTime(hours, minutes, seconds, hundredths);
  }

  static int counter = 0;

  // Check if 100ms has elapsed
  if (counter == 10)
  {
    setTimeToMqtt(timerId, hours, minutes, seconds, hundredths);
    counter = 0; // Reset the counter
  }
  else
  {
    counter++;
  }
}

/**
 * @brief Callback function for Timer1.
 *
 * This function is called when Timer1 elapses.
 *
 * @param xTimer Handle to the timer that called this callback function.
 */
void timer1Callback(TimerHandle_t xTimer)
{
  handleTimerCallback(timer1Value, "TIMER1", 1);
}

/**
 * @brief Callback function for Timer2.
 *
 * This function is called when Timer2 elapses.
 *
 * @param xTimer Handle to the timer that called this callback function.
 */
void timer2Callback(TimerHandle_t xTimer)
{
  handleTimerCallback(timer2Value, "TIMER2", 2);
}

/**
 * @brief FreeRTOS task to handle timer chronometer functionality.
 * @param parameter Pointer to the timer ID (1 or 2).
 */
void timerChronometer(void *parameter)
{
  int timerId = *static_cast<int *>(parameter);
  const char *mode = (timerId == 1) ? "TIMER1" : "TIMER2";
  TimerHandle_t &timerHandle = (timerId == 1) ? timer1Handle : timer2Handle;
  TimerCallbackFunction_t timerCallback = (timerId == 1) ? timer1Callback : timer2Callback;

  while (strcmp(const_cast<char *>(secondaryScreenMode), mode) != 0)
  {
    vTaskDelay(10);
  }

  timerHandle = xTimerCreate(mode, pdMS_TO_TICKS(10), pdTRUE, nullptr, timerCallback);
  xTimerStart(timerHandle, 0);

  vTaskDelete(nullptr);
}

/**
 * @brief FreeRTOS task to handle timer 1 chronometer functionality.
 * @param parameter Unused parameter required by the FreeRTOS task signature.
 */
void timer1Chronometer(void *parameter)
{
  int timerId = 1;
  timerChronometer(&timerId);
}

/**
 * @brief FreeRTOS task to handle timer 2 chronometer functionality.
 * @param parameter Unused parameter required by the FreeRTOS task signature.
 */
void timer2Chronometer(void *parameter)
{
  int timerId = 2;
  timerChronometer(&timerId);
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
    const unsigned long totalSeconds = timerValue / 1000;

    hundredths = (timerValue / 10) % 100;
    seconds = totalSeconds % 60;
    minutes = (totalSeconds / 60) % 60;
    hours = (totalSeconds / 3600);
}

/**
 * @brief Displays the given time on the LED display.
 *
 * This function formats the given time components (hours, minutes, seconds, hundredths)
 * into a string and displays it on the LED display. If hours are greater than 0, the format
 * will be "HH-MM-SS". Otherwise, the format will be "MM-SS-HH".
 *
 * @param hours The hours component of the time.
 * @param minutes The minutes component of the time.
 * @param seconds The seconds component of the time.
 * @param hundredths The hundredths of a second component of the time.
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
 * @brief Publishes the formatted time to the specified MQTT topic based on the timer value.
 *
 * This function formats the provided time components (hours, minutes, seconds, hundredths)
 * into a string and publishes it to the corresponding MQTT topic for the given timer.
 *
 * @param timer An integer representing the timer (1 or 2) to determine the MQTT topic.
 * @param hours An integer representing the hours component of the time.
 * @param minutes An integer representing the minutes component of the time.
 * @param seconds An integer representing the seconds component of the time.
 * @param hundredths An integer representing the hundredths of a second component of the time.
 */
void setTimeToMqtt(int timer, int hours, int minutes, int seconds, int hundredths)
{
    char timeText[13]; // Adjusted size to accommodate thousandths

    // Format the time string with full thousandths of a second
    snprintf(timeText, sizeof(timeText), "%02d-%02d-%02d:%03d", hours, minutes, seconds, hundredths * 10);

    // Publish the time to the appropriate MQTT topic
    if (timer == 1)
    {
        mqttSetup.mqtt.publish(MQTT_TIMER1_TOPIC + String("value"), timeText);
    }
    else if (timer == 2)
    {
        mqttSetup.mqtt.publish(MQTT_TIMER2_TOPIC + String("value"), timeText);
    }
}