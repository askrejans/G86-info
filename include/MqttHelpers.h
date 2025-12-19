// MqttHelpers.h
// Helper functions for MQTT operations to reduce code duplication

#ifndef MQTT_HELPERS_H
#define MQTT_HELPERS_H

#include "Constants.h"
#include <Arduino.h>

/**
 * @brief Safely builds an MQTT topic string without heap allocation
 * @param buffer Output buffer for the topic
 * @param bufferSize Size of the output buffer
 * @param base Base topic string (e.g., "/GOLF86/ECU/")
 * @param suffix Suffix to append (e.g., "RPM" or "started")
 * @return true if successful, false if buffer too small
 */
inline bool buildMqttTopic(char *buffer, size_t bufferSize, const char *base, const char *suffix) {
    if (buffer == nullptr || base == nullptr || suffix == nullptr) {
        return false;
    }
    
    int written = snprintf(buffer, bufferSize, "%s%s", base, suffix);
    
    if (written < 0 || (size_t)written >= bufferSize) {
        Serial.printf("ERROR: MQTT topic too long (%d chars, max %d)\n", written, bufferSize - 1);
        return false;
    }
    
    return true;
}

/**
 * @brief Safely publishes to an MQTT topic with base + suffix
 * @param client MQTT client to use
 * @param base Base topic string
 * @param suffix Suffix to append
 * @param payload Payload to publish
 * @return true if successful
 */
inline bool publishMqtt(MQTTClient &client, const char *base, const char *suffix, const char *payload) {
    char topic[MQTT_TOPIC_BUFFER_SIZE];
    
    if (!buildMqttTopic(topic, sizeof(topic), base, suffix)) {
        return false;
    }
    
    return client.publish(topic, payload);
}

/**
 * @brief Validates MQTT topic string for dangerous characters
 * @param topic Topic string to validate
 * @return true if safe, false if contains dangerous characters
 */
inline bool validateMqttTopic(const char *topic) {
    if (topic == nullptr) return false;
    
    // Check for null bytes, control characters, and wildcards in inappropriate places
    for (const char *p = topic; *p != '\0'; p++) {
        // Control characters are not allowed
        if (*p < 32 && *p != '\n' && *p != '\r' && *p != '\t') {
            return false;
        }
        // # wildcard should only be at the end
        if (*p == '#' && *(p+1) != '\0') {
            return false;
        }
    }
    
    return true;
}

#endif // MQTT_HELPERS_H
