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
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Already connected to WiFi.");
        return true;
    }

    Serial.println("Connecting to WiFi...");
    WiFi.begin(AP_NAME, WIFI_PASSWORD);

    const unsigned long startAttemptTime = millis();
    const unsigned long timeout = 30000; // 30 seconds timeout

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout)
    {
        delay(500);
        Serial.print(".");
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

    // Validate or set default values for configuration fields
    setDefaultIfEmpty(config.mqtt_port, "1883", sizeof(config.mqtt_port));
    setDefaultIfEmpty(config.mqtt_server, "localhost", sizeof(config.mqtt_server));
}

void WiFiSetup::setDefaultIfEmpty(char* field, const char* defaultValue, size_t fieldSize)
{
    if (strlen(field) == 0)
    {
        strncpy(field, defaultValue, fieldSize);
    }
}