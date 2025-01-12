#include "MqttSetup.h"

unsigned long MqttSetup::lastBlinkMillis = 0;
bool MqttSetup::colonVisible = true;

/**
 * Initialize MQTT setup.
 */
void MqttSetup::begin()
{
    // Initialize MQTT connection for Primary client
    mqtt.begin(wifiSetup.config.mqtt_server, net);
    mqtt.onMessage(MqttMessageReceivedPrimary);

    Serial.print("\nconnecting to MQTT Primary to " + String(wifiSetup.config.mqtt_server) + ":" + String(wifiSetup.config.mqtt_port));

    // Attempt to connect to MQTT Primary
    while (!mqtt.connect(PRIMARY_MQTT_CLIENT_NAME, "public", "public"))
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nMQTT Primary connected!");

    // Initialize MQTT connection for Secondary client
    mqtt2.begin(wifiSetup.config.mqtt_server, net2);
    mqtt2.onMessage(MqttMessageReceivedSecondary);

    Serial.print("\nconnecting to MQTT Secondary to " + String(wifiSetup.config.mqtt_server) + ":" + String(wifiSetup.config.mqtt_port));

    // Attempt to connect to MQTT Secondary
    while (!mqtt2.connect(SECONDARY_MQTT_CLIENT_NAME, "public", "public"))
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nMQTT Secondary connected!");
}

/**
 * Handle MQTT connections for both Primary and Secondary clients.
 */
void MqttSetup::connect()
{
    mqtt.loop();
    mqtt2.loop();
}

/**
 * Callback function for receiving MQTT messages on the Primary channel.
 * @param topic The MQTT topic.
 * @param payload The MQTT payload.
 */
void MqttSetup::MqttMessageReceivedPrimary(String &topic, String &payload)
{
    // Extract the last two segments from the MQTT topic
    int lastSlashIndex = topic.lastIndexOf('/');
    if (lastSlashIndex != -1)
    {
        String lastSegment = topic.substring(lastSlashIndex + 1);

        int secondLastSlashIndex = topic.lastIndexOf('/', lastSlashIndex - 1);
        if (secondLastSlashIndex != -1)
        {
            String secondLastSegment = topic.substring(secondLastSlashIndex + 1, lastSlashIndex);

            // Switch between the last two segments
            if (secondLastSegment == "GPS")
            {
                handleGpsPayload(lastSegment, payload);
            }
            else if (secondLastSegment == "ECU")
            {
                handleEcuPayload(lastSegment, payload);
            }
        }
    }
    // Convert String to char array
    strncpy(newMessage, payload.c_str(), sizeof(newMessage) - 1);
    newMessage[sizeof(newMessage) - 1] = '\0'; // Ensure null-termination
    newMessageAvailable = true;
}

/**
 * @brief Handles the GPS payload based on the last segment of the topic.
 * 
 * This function processes the payload string according to the last segment of the topic.
 * It performs different transformations based on the value of the last segment.
 * 
 * @param lastSegment The last segment of the topic indicating the type of data.
 * @param payload The payload string to be transformed.
 * 
 * The function handles the following segments:
 * - "TME": Transforms the payload using the transformTime function.
 * - "DTE": Formats the date by removing dots, adding a slash, and removing the year.
 * - "SPD": Rounds the speed to the nearest integer and appends "kmh".
 * - "ALT": Appends "m" to the altitude value.
 */
void MqttSetup::handleGpsPayload(const String &lastSegment, String &payload)
{
    if (lastSegment == "TME")
    {
        MqttSetup::transformTime(payload);
    }
    else if (lastSegment == "DTE")
    {
        // Remove dots between values, add slash, remove year. Incoming format dd.mm.yyyy
        payload.replace(".", "");
        String day = payload.substring(0, 2);
        String month = payload.substring(2, 4);
        payload = day + "/" + month;
    }
    else if (lastSegment == "SPD")
    {
        // Round the speed to the closest 1kmh + add kmh string
        float speed = payload.toFloat();
        payload = String(int(speed)) + "kmh";
    }
    else if (lastSegment == "ALT")
    {
        payload = payload + "m";
    }
}

/**
 * @brief Handles the ECU payload by appending appropriate units based on the last segment.
 *
 * This function modifies the payload string by appending a unit suffix based on the value of the lastSegment parameter.
 * The following suffixes are appended:
 * - "TPS", "VE1", "TAE": Adds '%' to the payload.
 * - "MAT", "CAD": Adds 'C' to the payload.
 * - "BAT": Adds 'V' to the payload.
 * - "DWL": Adds 'ms' to the payload.
 *
 * @param lastSegment The last segment of the topic which determines the unit to be appended.
 * @param payload The payload string which will be modified to include the appropriate unit.
 */
void MqttSetup::handleEcuPayload(const String &lastSegment, String &payload)
{
    if (lastSegment == "TPS" || lastSegment == "VE1" || lastSegment == "TAE")
    {
        // Add '%' sign to the end of the numeric value
        payload = payload + "%";
    }
    else if (lastSegment == "MAT" || lastSegment == "CAD")
    {
        // Add 'C'
        payload = payload + "C";
    }
    else if (lastSegment == "BAT")
    {
        // Add 'V'
        payload = payload + "V";
    }
    else if (lastSegment == "DWL")
    {
        // Add 'ms'
        payload = payload + "ms";
    }
}

/**
 * Callback function for receiving MQTT messages on the Secondary channel.
 * @param topic The MQTT topic.
 * @param payload The MQTT payload.
 */
void MqttSetup::MqttMessageReceivedSecondary(String &topic, String &payload)
{
    // reverse string, as 7segment display expects it.
    MqttSetup::reverseString(payload);

    // Extract the last two segments from the MQTT topic
    int lastSlashIndex = topic.lastIndexOf('/');
    if (lastSlashIndex != -1)
    {
        String lastSegment = topic.substring(lastSlashIndex + 1);

        int secondLastSlashIndex = topic.lastIndexOf('/', lastSlashIndex - 1);
        if (secondLastSlashIndex != -1)
        {
            String secondLastSegment = topic.substring(secondLastSlashIndex + 1, lastSlashIndex);

            if (secondLastSegment == "ECU" && (lastSegment == "MAT" || lastSegment == "CAD"))
            {
                // Add 'C'
                payload = "C" + payload;
            }
            else if (secondLastSegment == "ECU" && lastSegment == "BAT")
            {
                // Add 'V'
                payload = "V" + payload;
            }
        }
    }
    // Convert String to char array
    strncpy(const_cast<char *>(newMessage2), payload.c_str(), sizeof(newMessage2) - 1);
    newMessage2[sizeof(newMessage2) - 1] = '\0'; // Ensure null-termination
    newMessageAvailable2 = true;
}

/**
 * Reverse a given string.
 * @param str The string to reverse.
 */
void MqttSetup::reverseString(String &str)
{
    int length = str.length();
    for (int i = 0; i < length / 2; i++)
    {
        std::swap(str[i], str[length - i - 1]);
    }
}

/**
 * @brief Transforms the given time string to a specific format and optionally hides the colon.
 * 
 * This function first checks if the provided time string is in a valid format.
 * If the format is invalid, it prints an error message and returns.
 * If the format is valid, it formats the time string and optionally replaces
 * colons with spaces based on the colonVisible flag. Finally, it toggles the
 * visibility of the colon for future calls.
 * 
 * @param timeString The time string to be transformed. This parameter is modified in place.
 */
void MqttSetup::transformTime(String &timeString)
{
    if (!isValidTimeFormat(timeString))
    {
        Serial.println("Invalid time format: " + timeString);
        return;
    }

    timeString = formatTimeString(timeString);

    if (!MqttSetup::colonVisible)
    {
        std::replace(timeString.begin(), timeString.end(), ':', ' ');
    }

    toggleColonVisibility();
}

/**
 * @brief Checks if the given time string is in a valid format.
 *
 * This function verifies that the provided time string follows the format "HH:MM:SS".
 * The string must be exactly 8 characters long, with colons at the 3rd and 6th positions.
 *
 * @param timeString The time string to validate.
 * @return true if the time string is in the format "HH:MM:SS", false otherwise.
 */
bool MqttSetup::isValidTimeFormat(const String &timeString)
{
    return timeString.length() == 8 && timeString[2] == ':' && timeString[5] == ':';
}

/**
 * @brief Formats a given time string to a shorter version.
 *
 * This function takes a time string and returns a substring containing
 * the first 5 characters of the input string.
 *
 * @param timeString The original time string to be formatted.
 * @return A substring of the input time string containing the first 5 characters.
 */
String MqttSetup::formatTimeString(const String &timeString)
{
    return timeString.substring(0, 5);
}

/**
 * @brief Toggles the visibility of the colon in the display.
 *
 * This function checks the elapsed time since the last toggle and if it has been
 * at least 1000 milliseconds (1 second), it toggles the visibility of the colon.
 * The function uses the current time from the `millis()` function to determine
 * the elapsed time.
 *
 * @note This function should be called periodically to ensure the colon visibility
 * is toggled at the correct intervals.
 */
void MqttSetup::toggleColonVisibility()
{
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkMillis >= 1000)
    {
        lastBlinkMillis = currentMillis;
        MqttSetup::colonVisible = !MqttSetup::colonVisible;
    }
}