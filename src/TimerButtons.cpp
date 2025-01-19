/**
 * TimerButtons.cpp
 * This file contains the implementation of functions related to timer switches and controls.
 */

#include "TimerButtons.h"
#include "SecondaryLoop.h"

// Global variable to store the active timer
int activeTimer;

// Initialize switch library for the timer input buttons
MD_UISwitch_Digital sw1Timer(SW1_TME_PIN);
MD_UISwitch_Digital sw2Timer(SW2_TME_PIN);
MD_UISwitch_Digital tg1PosTimer(TG1_TME_PIN);
MD_UISwitch_Digital tg2PosTimer(TG2_TME_PIN);

/**
 * @brief Configure the settings for a timer switch.
 * @param uiSwitch The timer switch to configure.
 * @param enableLongPress Whether to enable long press.
 * @param enableDoublePress Whether to enable double press.
 */
void configureTimerSwitch(MD_UISwitch_Digital &uiSwitch, bool enableLongPress, bool enableDoublePress)
{
    uiSwitch.begin();
    uiSwitch.enableRepeat(false);
    uiSwitch.enableLongPress(enableLongPress);
    uiSwitch.setPressTime(500);
    uiSwitch.setLongPressTime(1000);
    uiSwitch.enableDoublePress(enableDoublePress);
}

/**
 * @brief Setup navigation with rotary encoder and switch.
 * This function initializes the settings for the timer input buttons, including press, long press, and double press configurations.
 */
void setupTimerSwitches(void)
{
    // Configure settings for sw1Timer
    configureTimerSwitch(sw1Timer, true, false);

    // Configure settings for sw2Timer
    configureTimerSwitch(sw2Timer, true, false);

    // Configure settings for tg1PosTimer
    configureTimerSwitch(tg1PosTimer, false, false);

    // Configure settings for tg2PosTimer
    configureTimerSwitch(tg2PosTimer, false, false);
}

/**
 * @brief Monitors the state of various timer switches and handles their actions accordingly.
 * 
 * This function checks the state of multiple timer switches and performs actions such as 
 * starting, pausing, and resetting timers, as well as processing positional timer events.
 * 
 * The function uses two lambda functions:
 * - handleTimerPress: Handles the press event for starting or pausing a timer.
 * - handleTimerReset: Handles the reset event for resetting a timer.
 * 
 * The function checks the following switches:
 * - sw1Timer: If pressed, it starts or pauses the active timer.
 * - sw2Timer: If pressed, it resets the active timer.
 * - tg1PosTimer: If pressed down, it processes the tg1 positional timer event.
 * - tg2PosTimer: If pressed down, it processes the tg2 positional timer event.
 */
void monitorTimerSwitches()
{
    auto handleTimerPress = [](int timerId, bool &timerStarted, const String &mqttTopic) {
        if (!timerStarted)
        {
            mqttSetup.mqtt.publish(mqttTopic + "started", "true");
            mqttSetup.mqtt.publish(mqttTopic + "paused", "false");
            if (timerId == 1)
                startTimer1();
            else
                startTimer2();
        }
        else
        {
            mqttSetup.mqtt.publish(mqttTopic + "paused", "true");
            pauseTimer(timerId);
        }
    };

    auto handleTimerReset = [](int timerId, const String &mqttTopic) {
        resetTimer(timerId);
        mqttSetup.mqtt.publish(mqttTopic + "started", "false");
        mqttSetup.mqtt.publish(mqttTopic + "paused", "false");
        mqttSetup.mqtt.publish(mqttTopic + "value", "00-00-00:000");
    };

    // Check sw1Timer state
    if (sw1Timer.read() == MD_UISwitch::KEY_PRESS)
    {
        if (activeTimer == 1)
            handleTimerPress(1, timer1Started, MQTT_TIMER1_TOPIC);
        else if (activeTimer == 2)
            handleTimerPress(2, timer2Started, MQTT_TIMER2_TOPIC);
    }

    // Check sw2Timer state
    if (sw2Timer.read() == MD_UISwitch::KEY_PRESS)
    {
        if (activeTimer == 1)
            handleTimerReset(1, MQTT_TIMER1_TOPIC);
        else if (activeTimer == 2)
            handleTimerReset(2, MQTT_TIMER2_TOPIC);
    }

    // Check tg1PosTimer state
    if (tg1PosTimer.read() == MD_UISwitch::KEY_DOWN)
    {
        processTg1Pos();
    }

    // Check tg2PosTimer state
    if (tg2PosTimer.read() == MD_UISwitch::KEY_DOWN)
    {
        processTg2Pos();
    }
}

/**
 * @brief Start a timer and create a corresponding task.
 * This function initiates the creation of a timer task and updates the secondary screen mode accordingly.
 * @param timerId The ID of the timer (1 or 2).
 * @param taskName The name of the task to create.
 * @param taskFunction The function to execute as the task.
 * @param screenMode The screen mode to set when the task is created.
 * @param timerStarted Reference to the timer started flag.
 */
void startTimer(int timerId, const char *taskName, TaskFunction_t taskFunction, const char *screenMode, bool &timerStarted)
{
    if (xTaskCreatePinnedToCore(taskFunction, taskName, configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL, 1) == pdPASS)
    {
        // Task creation successful
        strcpy(const_cast<char *>(secondaryScreenMode), screenMode);
        timerStarted = true;
    }
}

/**
 * @brief Start timer 1 and create a corresponding task.
 * This function initiates the creation of a timer task for timer 1 and updates the secondary screen mode accordingly.
 */
void startTimer1()
{
    startTimer(1, "timer1Chronometer", timer1Chronometer, "TIMER1", timer1Started);
}

/**
 * @brief Start timer 2 and create a corresponding task.
 * This function initiates the creation of a timer task for timer 2 and updates the secondary screen mode accordingly.
 */
void startTimer2()
{
    startTimer(2, "timer2Chronometer", timer2Chronometer, "TIMER2", timer2Started);
}

/**
 * @brief Process timer group position.
 * This function updates the active timer and screen mode based on the position of the timer group.
 * @param timerId The ID of the timer group (1 or 2).
 * @param screenMode The screen mode to set.
 */
void processTgPos(int timerId, const char *screenMode)
{
    activeTimer = timerId;
    // Check screen mode and update if necessary
    if (strcmp(const_cast<char *>(secondaryScreenMode), "WELCOME") != 0 && strcmp(const_cast<char *>(secondaryScreenMode), "MQTT") != 0)
    {
        if (strcmp(const_cast<char *>(secondaryScreenMode), screenMode) != 0)
        {
            strcpy(const_cast<char *>(secondaryScreenMode), screenMode);
        }
    }
}

/**
 * @brief Process timer group 1 position.
 * This function updates the active timer and screen mode based on the position of timer group 1.
 */
void processTg1Pos()
{
    processTgPos(1, "TIMER1");
}

/**
 * @brief Process timer group 2 position.
 * This function updates the active timer and screen mode based on the position of timer group 2.
 */
void processTg2Pos()
{
    processTgPos(2, "TIMER2");
}

/**
 * @brief Reset the specified timer.
 * This function stops the timer, resets its value, and updates the status for the specified timer number.
 *
 * @param timerNr The number of the timer to reset (1 or 2).
 */
void resetTimer(int timerNr)
{
    TimerHandle_t *timerHandle = nullptr;
    volatile unsigned long *timerValue = nullptr;
    bool *timerStarted = nullptr;
    bool *timerPaused = nullptr;

    if (timerNr == 1) {
        timerHandle = &timer1Handle;
        timerValue = &timer1Value;
        timerStarted = &timer1Started;
        timerPaused = &timer1Paused;
    }
    else if (timerNr == 2) {
        timerHandle = &timer2Handle;
        timerValue = &timer2Value;
        timerStarted = &timer2Started;
        timerPaused = &timer2Paused;
    }

    if (timerHandle != nullptr && *timerHandle != NULL) {
        xTimerStop(*timerHandle, 0);
        *timerValue = 0;
        *timerStarted = false;
        *timerPaused = false;
    }
}

/**
 * @brief Pause the specified timer.
 * This function stops the timer and updates the status for the specified timer number.
 *
 * @param timerNr The number of the timer to pause (1 or 2).
 */
void pauseTimer(int timerNr)
{
    TimerHandle_t *timerHandle = nullptr;
    volatile unsigned long *timerValue = nullptr;
    bool *timerStarted = nullptr;
    bool *timerPaused = nullptr;

    if (timerNr == 1) {
        timerHandle = &timer1Handle;
        timerValue = &timer1Value;
        timerStarted = &timer1Started;
        timerPaused = &timer1Paused;
    }
    else if (timerNr == 2) {
        timerHandle = &timer2Handle;
        timerValue = &timer2Value;
        timerStarted = &timer2Started;
        timerPaused = &timer2Paused;
    }

    if (timerHandle != nullptr && *timerHandle != NULL) {
        // Stop the timer first
        xTimerStop(*timerHandle, 0);
        *timerPaused = true;

        // Calculate time components
        int hours, minutes, seconds, hundredths;
        convertTimerToTime(*timerValue, hours, minutes, seconds, hundredths);

        // Update both display and MQTT with the same time value
        displayTime(hours, minutes, seconds, hundredths);
        setTimeToMqtt(timerNr, hours, minutes, seconds, hundredths);

        // Ensure the timer value is preserved
        *timerStarted = false;
    }
}