// SharedData.cpp
// Implementation of thread-safe shared data structures

#include "SharedData.h"
#include <string.h>

// Global instances
SecondaryDisplayMode g_secondaryMode;
ThreadSafeMessage g_secondaryMessage;

// Legacy volatile variables (for backward compatibility during transition)
volatile char secondaryScreenMode[MODE_BUFFER_SIZE] = "WELCOME";
volatile bool newMessageAvailable2 = false;
volatile char newMessage2[MESSAGE_BUFFER_SIZE] = "";

/**
 * @brief Constructor for SecondaryDisplayMode
 */
SecondaryDisplayMode::SecondaryDisplayMode() {
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL) {
        Serial.println("ERROR: Failed to create SecondaryDisplayMode mutex!");
    }
    strncpy(mode, "WELCOME", MODE_BUFFER_SIZE - 1);
    mode[MODE_BUFFER_SIZE - 1] = '\0';
}

/**
 * @brief Destructor for SecondaryDisplayMode
 */
SecondaryDisplayMode::~SecondaryDisplayMode() {
    if (mutex != NULL) {
        vSemaphoreDelete(mutex);
    }
}

/**
 * @brief Thread-safe set mode
 * @param newMode The new mode string
 * @return true if successful, false if mutex timeout
 */
bool SecondaryDisplayMode::set(const char* newMode) {
    if (mutex == NULL || newMode == NULL) {
        return false;
    }
    
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        strncpy(mode, newMode, MODE_BUFFER_SIZE - 1);
        mode[MODE_BUFFER_SIZE - 1] = '\0';
        
        // Update legacy volatile for backward compatibility
        strncpy((char*)secondaryScreenMode, newMode, MODE_BUFFER_SIZE - 1);
        ((char*)secondaryScreenMode)[MODE_BUFFER_SIZE - 1] = '\0';
        
        xSemaphoreGive(mutex);
        return true;
    }
    
    Serial.println("WARNING: SecondaryDisplayMode::set() mutex timeout");
    return false;
}

/**
 * @brief Thread-safe get mode
 * @param buffer Buffer to copy mode into
 * @param bufferSize Size of the buffer
 * @return true if successful, false if mutex timeout
 */
bool SecondaryDisplayMode::get(char* buffer, size_t bufferSize) {
    if (mutex == NULL || buffer == NULL || bufferSize == 0) {
        return false;
    }
    
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        strncpy(buffer, mode, bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        xSemaphoreGive(mutex);
        return true;
    }
    
    Serial.println("WARNING: SecondaryDisplayMode::get() mutex timeout");
    return false;
}

/**
 * @brief Thread-safe compare mode
 * @param compareMode Mode to compare against
 * @return true if modes match, false otherwise
 */
bool SecondaryDisplayMode::equals(const char* compareMode) {
    if (mutex == NULL || compareMode == NULL) {
        return false;
    }
    
    bool result = false;
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        result = (strcmp(mode, compareMode) == 0);
        xSemaphoreGive(mutex);
    }
    
    return result;
}

/**
 * @brief Constructor for ThreadSafeMessage
 */
ThreadSafeMessage::ThreadSafeMessage() {
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL) {
        Serial.println("ERROR: Failed to create ThreadSafeMessage mutex!");
    }
    message[0] = '\0';
    available = false;
}

/**
 * @brief Destructor for ThreadSafeMessage
 */
ThreadSafeMessage::~ThreadSafeMessage() {
    if (mutex != NULL) {
        vSemaphoreDelete(mutex);
    }
}

/**
 * @brief Thread-safe set message
 * @param newMessage The new message string
 * @return true if successful, false if mutex timeout
 */
bool ThreadSafeMessage::setMessage(const char* newMessage) {
    if (mutex == NULL || newMessage == NULL) {
        return false;
    }
    
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        size_t len = strlen(newMessage);
        if (len >= MESSAGE_BUFFER_SIZE) {
            Serial.printf("WARNING: Message truncated from %d to %d chars\n", 
                         len, MESSAGE_BUFFER_SIZE - 1);
        }
        
        strncpy(message, newMessage, MESSAGE_BUFFER_SIZE - 1);
        message[MESSAGE_BUFFER_SIZE - 1] = '\0';
        
        // Update legacy volatile for backward compatibility
        strncpy((char*)newMessage2, message, MESSAGE_BUFFER_SIZE - 1);
        ((char*)newMessage2)[MESSAGE_BUFFER_SIZE - 1] = '\0';
        
        available = true;
        newMessageAvailable2 = true;
        
        xSemaphoreGive(mutex);
        return true;
    }
    
    Serial.println("WARNING: ThreadSafeMessage::setMessage() mutex timeout");
    return false;
}

/**
 * @brief Thread-safe get message
 * @param buffer Buffer to copy message into
 * @param bufferSize Size of the buffer
 * @return true if successful, false if mutex timeout or no message
 */
bool ThreadSafeMessage::getMessage(char* buffer, size_t bufferSize) {
    if (mutex == NULL || buffer == NULL || bufferSize == 0) {
        return false;
    }
    
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (available) {
            strncpy(buffer, message, bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
            xSemaphoreGive(mutex);
            return true;
        }
        xSemaphoreGive(mutex);
    }
    
    return false;
}

/**
 * @brief Check if message is available
 * @return true if message available, false otherwise
 */
bool ThreadSafeMessage::isAvailable() {
    if (mutex == NULL) {
        return false;
    }
    
    bool result = false;
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        result = available;
        xSemaphoreGive(mutex);
    }
    
    return result;
}

/**
 * @brief Clear the available flag
 */
void ThreadSafeMessage::clearAvailable() {
    if (mutex == NULL) {
        return;
    }
    
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        available = false;
        newMessageAvailable2 = false;
        xSemaphoreGive(mutex);
    }
}

/**
 * @brief Set the available flag
 * @param value The value to set
 */
void ThreadSafeMessage::setAvailable(bool value) {
    if (mutex == NULL) {
        return;
    }
    
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        available = value;
        newMessageAvailable2 = value;
        xSemaphoreGive(mutex);
    }
}

/**
 * @brief Initialize all shared data synchronization primitives
 */
void initSharedData() {
    Serial.println("Initializing shared data synchronization...");
    // Objects are initialized via their constructors
    Serial.println("Shared data synchronization initialized");
}
