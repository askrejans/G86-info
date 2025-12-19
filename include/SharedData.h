// SharedData.h
// Thread-safe shared data structures for multi-core communication

#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Buffer size constants
#define MESSAGE_BUFFER_SIZE 128
#define MODE_BUFFER_SIZE 16
#define DATA_INDEX_SIZE 4

// Display polling interval
#define DISPLAY_UPDATE_INTERVAL_MS 10

// MQTT connection retry settings
#define MQTT_MAX_CONNECT_ATTEMPTS 20
#define MQTT_CONNECT_RETRY_DELAY_MS 1000
#define MQTT_RECONNECT_INTERVAL_MS 5000

// Task stack sizes
#define SECONDARY_DISPLAY_STACK_SIZE 8192
#define TIMER_TASK_STACK_SIZE 4096

// Timer overflow protection (max ~11 hours)
#define MAX_TIMER_VALUE_MS (40000000UL) // 11.1 hours in centiseconds

/**
 * @brief Thread-safe wrapper for secondary display mode
 */
class SecondaryDisplayMode {
private:
    char mode[MODE_BUFFER_SIZE];
    SemaphoreHandle_t mutex;

public:
    SecondaryDisplayMode();
    ~SecondaryDisplayMode();
    
    bool set(const char* newMode);
    bool get(char* buffer, size_t bufferSize);
    bool equals(const char* compareMode);
};

/**
 * @brief Thread-safe wrapper for message passing between cores
 */
class ThreadSafeMessage {
private:
    char message[MESSAGE_BUFFER_SIZE];
    volatile bool available;
    SemaphoreHandle_t mutex;

public:
    ThreadSafeMessage();
    ~ThreadSafeMessage();
    
    bool setMessage(const char* newMessage);
    bool getMessage(char* buffer, size_t bufferSize);
    bool isAvailable();
    void clearAvailable();
    void setAvailable(bool value);
};

// Global thread-safe instances
extern SecondaryDisplayMode g_secondaryMode;
extern ThreadSafeMessage g_secondaryMessage;

// Legacy volatile variables (to be phased out)
extern volatile char secondaryScreenMode[MODE_BUFFER_SIZE];
extern volatile bool newMessageAvailable2;
extern volatile char newMessage2[MESSAGE_BUFFER_SIZE];

// Initialize synchronization primitives
void initSharedData();

#endif // SHARED_DATA_H
