#ifndef TIMER_BUTTON_H
#define TIMER_BUTTON_H

#include <MD_UISwitch.h> // Physical switch lib https://github.com/MajicDesigns/MD_UISwitch
#include "MqttSetup.h"

// External declarations for global variables used in TimerButtons.cpp
extern int activeTimer;
extern bool timer1Started;
extern bool timer2Started;
extern TimerHandle_t timer1Handle;
extern TimerHandle_t timer2Handle;
extern volatile unsigned long timer1Value;
extern volatile unsigned long timer2Value;
extern const String MQTT_TIMER1_TOPIC;
extern const String MQTT_TIMER2_TOPIC;
extern MqttSetup mqttSetup;

// Pin assignments for timer buttons
const uint8_t SW1_TME_PIN = 14; // Port for the first timer button
const uint8_t SW2_TME_PIN = 12; // Port for the second timer button
const uint8_t TG1_TME_PIN = 33; // Port for the first timer toggle switch position
const uint8_t TG2_TME_PIN = 13; // Port for the second timer toggle switch position

// Function declarations
void setupTimerSwitches(void);
void configureTimerSwitch(void);
void monitorTimerSwitches(void);
void startTimer1(void);
void startTimer2(void);
void processTg1Pos(void);
void processTg2Pos(void);
void resetTimer(int timerNr);
void pauseTimer(int timerNr);

// External declaration for secondary screen mode
extern volatile char secondaryScreenMode[];

#endif // TIMER_BUTTON_H
