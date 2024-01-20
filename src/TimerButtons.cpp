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
 * @brief Monitor the state of timer switches.
 * This function reads the state of the timer input buttons and takes appropriate actions based on the button press events.
 */
void monitorTimerSwitches()
{
    // Check sw1Timer state
    switch (sw1Timer.read())
    {
    case MD_UISwitch::KEY_PRESS:
        // Check active timer and timer state, then perform appropriate action
        if (activeTimer == 1 && !timer1Started)
        {
            startTimer1();
        }
        else if (activeTimer == 1)
        {
            pauseTimer(1);
        }
        else if (activeTimer == 2 && !timer2Started)
        {
            startTimer2();
        }
        else if (activeTimer == 2)
        {
            pauseTimer(2);
        }
    }

    // Check sw2Timer state
    switch (sw2Timer.read())
    {
    case MD_UISwitch::KEY_PRESS:
        // Check active timer and perform reset
        if (activeTimer == 1)
        {
            resetTimer(1);
        }
        else if (activeTimer == 2)
        {
            resetTimer(2);
        }
    }

    // Check tg1PosTimer state
    switch (tg1PosTimer.read())
    {
    case MD_UISwitch::KEY_DOWN:
        // Process timer group 1 position
        processTg1Pos();
    }

    // Check tg2PosTimer state
    switch (tg2PosTimer.read())
    {
    case MD_UISwitch::KEY_DOWN:
        // Process timer group 2 position
        processTg2Pos();
    }
}

/**
 * @brief Start timer 1 and create a corresponding task.
 * This function initiates the creation of a timer task for timer 1 and updates the secondary screen mode accordingly.
 */
void startTimer1()
{
    if (xTaskCreatePinnedToCore(timer1Chronometer, "timer1Chronometer", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL, 1) == pdPASS)
    {
        // Task creation successful
        strcpy(const_cast<char *>(secondaryScreenMode), "TIMER1");
        timer1Started = true;
    }
}

/**
 * @brief Start timer 2 and create a corresponding task.
 * This function initiates the creation of a timer task for timer 2 and updates the secondary screen mode accordingly.
 */
void startTimer2()
{
    if (xTaskCreatePinnedToCore(timer2Chronometer, "timer2Chronometer", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL, 1) == pdPASS)
    {
        // Task creation successful
        strcpy(const_cast<char *>(secondaryScreenMode), "TIMER2");
        timer2Started = true;
    }
}

/**
 * @brief Process timer group 1 position.
 * This function updates the active timer and screen mode based on the position of timer group 1.
 */
void processTg1Pos()
{
    activeTimer = 1;
    // Check screen mode and update if necessary
    if (strcmp(const_cast<char *>(secondaryScreenMode), "WELCOME") != 0 && strcmp(const_cast<char *>(secondaryScreenMode), "MQTT") != 0)
    {
        if (strcmp(const_cast<char *>(secondaryScreenMode), "TIMER1") != 0)
        {
            strcpy(const_cast<char *>(secondaryScreenMode), "TIMER1");
        }
    }
}

/**
 * @brief Process timer group 2 position.
 * This function updates the active timer and screen mode based on the position of timer group 2.
 */
void processTg2Pos()
{
    activeTimer = 2;
    // Check screen mode and update if necessary
    if (strcmp(const_cast<char *>(secondaryScreenMode), "WELCOME") != 0 && strcmp(const_cast<char *>(secondaryScreenMode), "MQTT") != 0)
    {
        if (strcmp(const_cast<char *>(secondaryScreenMode), "TIMER2") != 0)
        {
            strcpy(const_cast<char *>(secondaryScreenMode), "TIMER2");
        }
    }
}

/**
 * @brief Reset the specified timer.
 * This function stops the timer, resets its value, and updates the status for the specified timer number.
 *
 * @param timerNr The number of the timer to reset (1 or 2).
 */
void resetTimer(int timerNr)
{
    switch (timerNr)
    {
    case 1:
        if (timer1Handle != NULL)
        {
            // Stop timer, reset value, and update status
            xTimerStop(timer1Handle, 0);
            timer1Value = 0;
            timer1Started = false;
        }
        break;
    case 2:
        if (timer2Handle != NULL)
        {
            // Stop timer, reset value, and update status
            xTimerStop(timer2Handle, 0);
            timer2Value = 0;
            timer2Started = false;
        }
        break;
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
    switch (timerNr)
    {
    case 1:
        if (timer1Handle != NULL)
        {
            // Stop timer and update status
            xTimerStop(timer1Handle, 0);
            timer1Started = false;
        }
        break;
    case 2:
        if (timer2Handle != NULL)
        {
            // Stop timer and update status
            xTimerStop(timer2Handle, 0);
            timer2Started = false;
        }
        break;
    }
}
