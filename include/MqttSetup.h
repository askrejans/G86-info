#ifndef MQTT_SETUP_H
#define MQTT_SETUP_H

#include <Arduino.h>
#include <MQTT.h>
#include <WiFiClient.h>
#include "WiFiSetup.h"

/**
 * @brief Global constants for MQTT client names and topics.
 */
extern const char *PRIMARY_MQTT_CLIENT_NAME;
extern const char *SECONDARY_MQTT_CLIENT_NAME;
extern const char *MQTT_TOPIC_BASE;

/**
 * @brief Global instances of WiFiSetup, message flags, and message buffers.
 */
extern WiFiSetup wifiSetup;
extern bool newMessageAvailable;
extern char newMessage[128];
extern volatile bool newMessageAvailable2;
extern volatile char newMessage2[128];

/**
 * @brief Class for setting up and managing MQTT communication.
 */
class MqttSetup
{
public:
    /**
     * @brief Callback function for receiving MQTT messages on the primary channel.
     * @param topic The MQTT topic.
     * @param payload The message payload.
     */
    static void MqttMessageReceivedPrimary(String &topic, String &payload);

    /**
     * @brief Callback function for receiving MQTT messages on the secondary channel.
     * @param topic The MQTT topic.
     * @param payload The message payload.
     */
    static void MqttMessageReceivedSecondary(String &topic, String &payload);

    /**
     * @brief Reverses the characters in the given string.
     * @param str The string to be reversed.
     */
    static void reverseString(String &str);

    /**
     * @brief Transforms the time string to a specific format.
     * @param timeString The time string to be transformed.
     */
    static void transformTime(String &timeString);

    /**
     * @brief Initializes the MQTT setup.
     */
    void begin();

    /**
     * @brief Sets up the MQTT connection.
     */
    void setupMqtt();

    /**
     * @brief Connects to the MQTT broker.
     */
    void connect();

    /**
     * @brief Gets the MQTT client for primary channel.
     * @return Reference to the MQTT client.
     */
    MQTTClient &getMqttClient();

    /**
     * @brief Gets the MQTT client for secondary channel.
     * @return Reference to the MQTT client.
     */
    MQTTClient mqtt;
    MQTTClient mqtt2;

private:
    WiFiClient net;
    WiFiClient net2;

    /**
     * @brief Static variables for managing LED blinking.
     */
    static unsigned long lastBlinkMillis;
    static bool colonVisible;

    /**
     * @brief Handles GPS payload transformations.
     * @param lastSegment The last segment of the topic.
     * @param payload The message payload.
     */
    static void handleGpsPayload(const String &lastSegment, String &payload);

    /**
     * @brief Handles ECU payload transformations.
     * @param lastSegment The last segment of the topic.
     * @param payload The message payload.
     */
    static void handleEcuPayload(const String &lastSegment, String &payload);

    /**
     * @brief Checks if the time string is in a valid format.
     * @param timeString The time string to check.
     * @return True if the time string is valid, false otherwise.
     */
    static bool isValidTimeFormat(const String &timeString);

    /**
     * @brief Formats the time string to "HH:MM".
     * @param timeString The time string to format.
     * @return The formatted time string.
     */
    static String formatTimeString(const String &timeString);

    /**
     * @brief Toggles the visibility of the colon in the time string.
     */
    static void toggleColonVisibility();
};

#endif // MQTT_SETUP_H