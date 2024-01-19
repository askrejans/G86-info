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
            if (secondLastSegment == "GPS" && lastSegment == "TME")
            {
                MqttSetup::transformTime(payload);
            }
            else if (secondLastSegment == "GPS" && lastSegment == "DTE")
            {
                // Remove dots between values, add slash, remove year. Incoming format dd.mm.yyyy
                payload.replace(".", "");
                String day = payload.substring(0, 2);
                String month = payload.substring(2, 4);
                payload = day + "/" + month;
            }
            else if (secondLastSegment == "GPS" && lastSegment == "SPD")
            {
                // Round the speed to the closest 1kmh + add kmh string
                float speed = payload.toFloat();
                payload = String(int(speed)) + "kmh";
            }
            else if (secondLastSegment == "GPS" && lastSegment == "ALT")
            {
                payload = payload + "m";
            }
            else if (secondLastSegment == "ECU" && (lastSegment == "TPS" || lastSegment == "VE1" || lastSegment == "TAE"))
            {
                // Add '%' sign to the end of the numeric value
                payload = payload + "%";
            }
            else if (secondLastSegment == "ECU" && (lastSegment == "MAT" || lastSegment == "CAD"))
            {
                // Add 'C'
                payload = payload + "C";
            }
            else if (secondLastSegment == "ECU" && lastSegment == "BAT")
            {
                // Add 'V'
                payload = payload + "V";
            }
            else if (secondLastSegment == "ECU" && lastSegment == "DWL")
            {
                // Add 'ms'
                payload = payload + "ms";
            }
        }
    }
    // Convert String to char array
    strncpy(newMessage, payload.c_str(), sizeof(newMessage) - 1);
    newMessage[sizeof(newMessage) - 1] = '\0'; // Ensure null-termination
    newMessageAvailable = true;
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
        char temp = str[i];
        str[i] = str[length - i - 1];
        str[length - i - 1] = temp;
    }
}

/**
 * Transform the time string to a specific format.
 * @param timeString The time string to transform.
 */
void MqttSetup::transformTime(String &timeString)
{
    // Check if the timeString has the expected format "HH:MM:SS"
    if (timeString.length() == 8 && timeString[2] == ':' && timeString[5] == ':')
    {
        // Extract the "HH:MM" part
        String transformedTime = timeString.substring(0, 5);

        // Replace the original timeString with the transformed one
        timeString = transformedTime;

        if (!MqttSetup::colonVisible)
        {
            std::replace(timeString.begin(), timeString.end(), ':', ' ');
        }

        // Blink the colon every second
        if (millis() - lastBlinkMillis >= 1000)
        {
            lastBlinkMillis = millis();
            MqttSetup::colonVisible = !MqttSetup::colonVisible; // Toggle colon visibility
        }
    }
    else
    {
        Serial.println("Invalid time format: " + timeString);
    }
}
