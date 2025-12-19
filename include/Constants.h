// Constants.h
// Centralized constants for the G86-INFO embedded system

#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================

// DOT Matrix Display (MD_Parola)
#define DOT_MATRIX_CS_PIN 21
#define DOT_MATRIX_MAX_DEVICES 4
#define DOT_MATRIX_CHAR_SPACING 1
#define DOT_MATRIX_DEFAULT_BRIGHTNESS 5
#define DOT_MATRIX_MAX_BRIGHTNESS 15

// 7-Segment Display (LedController)
#define SEVEN_SEG_DIN_PIN 5
#define SEVEN_SEG_CS_PIN 4
#define SEVEN_SEG_CLK_PIN 15
#define SEVEN_SEG_NUM_DIGITS 8
#define SEVEN_SEG_DEFAULT_INTENSITY 8
#define SEVEN_SEG_SCROLL_DELAY_MS 250

// ============================================================================
// NETWORK CONFIGURATION
// ============================================================================

// WiFi Configuration
#define WIFI_AP_NAME "G86-INFO-AP"
#define WIFI_AP_PASSWORD "golf1986"
#define WIFI_CONNECTION_TIMEOUT_MS 30000  // 30 seconds
#define WIFI_MANAGER_TIMEOUT_S 180        // 3 minutes for config portal

// MQTT Configuration
#define MQTT_CLIENT_PRIMARY "G86-INFO"
#define MQTT_CLIENT_SECONDARY "G86-INFO2"
#define MQTT_TOPIC_BASE "GOLF86"
#define MQTT_MAX_CONNECT_ATTEMPTS 20
#define MQTT_CONNECT_RETRY_DELAY_MS 1000
#define MQTT_RECONNECT_INTERVAL_MS 5000
#define MQTT_DEFAULT_PORT 1883
#define MQTT_USERNAME "public"
#define MQTT_PASSWORD "public"

// Web Server
#define WEB_SERVER_PORT 8080  // Changed from 80 to avoid conflict with WiFiManager
#define WEB_SERVER_CACHE_CONTROL "no-cache, no-store, must-revalidate"

// MQTT Topic Buffer Size
#define MQTT_TOPIC_BUFFER_SIZE 64

// ============================================================================
// MENU CONFIGURATION
// ============================================================================

#define MENU_TIMEOUT_MS 3000
#define MENU_ROTARY_A_PIN 26
#define MENU_ROTARY_B_PIN 25
#define MENU_BUTTON_PIN 27

// ============================================================================
// TIMER CONFIGURATION
// ============================================================================

// Timer Button Pins
#define SW1_TIMER_PIN 32  // Timer Start/Pause
#define SW2_TIMER_PIN 33  // Timer Reset
#define TG1_TIMER_PIN 34  // Timer 1 Selector
#define TG2_TIMER_PIN 35  // Timer 2 Selector

// Timer Button Configuration
#define TIMER_BUTTON_PRESS_TIME_MS 500
#define TIMER_BUTTON_LONG_PRESS_TIME_MS 1000

// Timer Operation
#define TIMER_TICK_INTERVAL_MS 10         // 10ms = centisecond precision
#define TIMER_MAX_VALUE_MS 40000000UL     // ~11.1 hours in centiseconds
#define TIMER_MQTT_UPDATE_INTERVAL_MS 100 // Update MQTT every 100ms

// ============================================================================
// TASK CONFIGURATION
// ============================================================================

// FreeRTOS Task Stack Sizes
#define SECONDARY_DISPLAY_TASK_STACK 8192   // 8KB for secondary display
#define SECONDARY_DISPLAY_STACK_SIZE 16384  // 16KB for secondary display (legacy name)
#define TIMER_TASK_STACK 4096               // 4KB for timer tasks
#define TIMER_TASK_STACK_SIZE 8192          // 8KB for timer tasks (increased)

// FreeRTOS Task Priorities
#define SECONDARY_DISPLAY_TASK_PRIORITY 1
#define TIMER_TASK_PRIORITY 2

// Task Update Intervals
#define DISPLAY_UPDATE_INTERVAL_MS 10     // Poll display mode every 10ms
#define BUTTON_POLL_INTERVAL_MS 50        // Poll buttons every 50ms

// ============================================================================
// BUFFER SIZES
// ============================================================================

#define MESSAGE_BUFFER_SIZE 128     // Message buffers for display text
#define MODE_BUFFER_SIZE 16         // Display mode string buffer
#define DATA_INDEX_SIZE 4           // ECU/GPS data index (3 chars + null)
#define MQTT_SERVER_SIZE 40         // MQTT server hostname
#define MQTT_PORT_SIZE 6            // MQTT port string

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================

// System Core Assignment
#define MAIN_LOOP_CORE 1           // Main loop runs on Core 1
#define SECONDARY_DISPLAY_CORE 0   // Secondary display runs on Core 0

// Watchdog Configuration
#define WATCHDOG_TIMEOUT_S 10      // 10 second watchdog timeout

// Web Server
#define WEB_SERVER_PORT 80         // HTTP server port

// ============================================================================
// WELCOME MESSAGES
// ============================================================================

#define WELCOME_MSG_PRIMARY "Golf'86"
#define WELCOME_MSG_SECONDARY "GOLF'86"

// ============================================================================
// DISPLAY MODES
// ============================================================================

#define DISPLAY_MODE_WELCOME "WELCOME"
#define DISPLAY_MODE_MQTT "MQTT"
#define DISPLAY_MODE_TIMER1 "TIMER1"
#define DISPLAY_MODE_TIMER2 "TIMER2"

// ============================================================================
// CONFIG VERSION
// ============================================================================

#define CONFIG_VERSION 1  // Increment when Config struct changes

#endif // CONSTANTS_H
