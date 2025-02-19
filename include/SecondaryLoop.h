// SECONDARY_LOOP_H

#ifndef SECONDARY_LOOP_H
#define SECONDARY_LOOP_H

#include <MD_UISwitch.h>
#include "LedController.hpp"
#include "MqttSetup.h"

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

/**
 * @brief FreeRTOS task to handle timer 1 chronometer functionality.
 * @param parameter Unused parameter required by the FreeRTOS task signature.
 */
void timer1Chronometer(void *parameter);

/**
 * @brief FreeRTOS task to handle timer 2 chronometer functionality.
 * @param parameter Unused parameter required by the FreeRTOS task signature.
 */
void timer2Chronometer(void *parameter);

extern TimerHandle_t timer1Handle;
extern TimerHandle_t timer2Handle;

extern bool timer1Started;
extern bool timer2Started;

extern bool timer1Paused;
extern bool timer2Paused;

extern volatile unsigned long timer1Value;
extern volatile unsigned long timer2Value;

extern const String MQTT_TIMER1_TOPIC;
extern const String MQTT_TIMER2_TOPIC;
extern MqttSetup mqttSetup;

/**
 * @brief Callback function for timer 1.
 * @param xTimer The timer handle.
 */
void timer1Callback(TimerHandle_t xTimer);

/**
 * @brief Callback function for timer 2.
 * @param xTimer The timer handle.
 */
void timer2Callback(TimerHandle_t xTimer);

/**
 * @brief Convert timer value to hours, minutes, seconds, and hundredths of seconds.
 * @param timerValue The timer value in milliseconds.
 * @param hours Reference to store the calculated hours.
 * @param minutes Reference to store the calculated minutes.
 * @param seconds Reference to store the calculated seconds.
 * @param hundredths Reference to store the calculated hundredths of seconds.
 */
void convertTimerToTime(unsigned long timerValue, int &hours, int &minutes, int &seconds, int &hundredths);

/**
 * @brief Display time on the 7-segment LED display.
 * @param hours Hours to display.
 * @param minutes Minutes to display.
 * @param seconds Seconds to display.
 * @param hundredths Hundredths of seconds to display.
 */
void displayTime(int hours, int minutes, int seconds, int hundredths);

/**
 * @brief Pushes time to MQTT
 * @param timer Timer nr.
 * @param hours Hours to display.
 * @param minutes Minutes to display.
 * @param seconds Seconds to display.
 * @param hundredths Hundredths of seconds to display.
 */
void setTimeToMqtt(int timer, int hours, int minutes, int seconds, int hundredths);

/**
 * @brief Displays the timer value in a formatted time.
 * @param timerValue The timer value in milliseconds.
 */
void displayTimer(int timerValue);

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
void handleTimerCallback(volatile unsigned long &timerValue, const char *mode, int timerId);

/**
 * @brief FreeRTOS task to handle timer chronometer functionality.
 * @param parameter Pointer to the timer ID (1 or 2).
 */
void timerChronometer(void *parameter);

#endif // SECONDARY_LOOP_H