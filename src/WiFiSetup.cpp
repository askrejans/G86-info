#include "WiFiSetup.h"
#include <Arduino.h>

// Constructor
WiFiSetup::WiFiSetup()
{
    prefs.begin("G86-INFO", false);
}

// Initialize WiFiSetup
void WiFiSetup::begin()
{
    WiFiManagerParameter custom_mqtt_server("server", "MQTT server", config.mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "MQTT port", config.mqtt_port, 6);

    // WiFiManager, Local initialization.
    WiFiManager wifiManager;

    // Set config save notify callback
    wifiManager.setSaveConfigCallback([this]()
                                      { saveConfigCallback(); });
    wifiManager.setPreSaveConfigCallback([this]()
                                         { saveConfigCallback(); });
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);

    wifiManager.setTimeout(180);

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ("G86-INFO"),
    // then goes into a blocking loop awaiting configuration and will return a success result
    if (!wifiManager.autoConnect(AP_NAME, WIFI_PASSWORD))
    {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        // Reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
    }

    // Save the custom parameters to FS
    if (shouldSaveConfig)
    {
        Serial.println("Saving config");
        strncpy(config.mqtt_server, custom_mqtt_server.getValue(), sizeof(config.mqtt_server) - 1);
        strncpy(config.mqtt_port, custom_mqtt_port.getValue(), sizeof(config.mqtt_port) - 1);
        paramSave();
        shouldSaveConfig = false; // Reset the flag
        Serial.println("MQTT config saved: ");
        Serial.println("\tmqtt_server : " + String(config.mqtt_server));
        Serial.println("\tmqtt_port : " + String(config.mqtt_port));
    }

    // Load config data from Preferences
    paramLoad();

    Serial.println("Local IP");
    Serial.println(WiFi.localIP());

    Serial.println("Connected to WiFi..");
}

// Connect to WiFi
bool WiFiSetup::connect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Connecting to WiFi...");
        WiFi.begin(AP_NAME, WIFI_PASSWORD);
        int attemptCount = 0;
        while (WiFi.status() != WL_CONNECTED && attemptCount < 30)
        {
            delay(1000);
            Serial.print(".");
            attemptCount++;
        }
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nConnected to WiFi");
            return true;
        }
        else
        {
            Serial.println("\nFailed to connect to WiFi");
            return false;
        }
    }
    return true; // Already connected
}

// Callback function for saving custom configuration
void WiFiSetup::saveConfigCallback()
{
    Serial.println("Custom config saved.");
    shouldSaveConfig = true; // Set the flag
}

// Save configuration parameters to Preferences
void WiFiSetup::paramSave()
{
    prefs.putBytes("config", &config, sizeof(config));
}

// Load configuration parameters from Preferences
void WiFiSetup::paramLoad()
{
    // Reading the stored configuration from preferences
    prefs.getBytes("config", &config, sizeof(config));

    // Optional: Add validation or set default values for individual fields
    if (strlen(config.mqtt_port) == 0)
    {
        strncpy(config.mqtt_port, "1883", sizeof(config.mqtt_port));
    }
    if (strlen(config.mqtt_server) == 0)
    {
        strncpy(config.mqtt_server, "localhost", sizeof(config.mqtt_server));
    }
}
