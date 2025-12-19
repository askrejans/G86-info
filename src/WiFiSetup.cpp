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
    
    // Reduce debug output to minimize ESP32 core error messages
    wifiManager.setDebugOutput(false);

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

// Connect to WiFi (reconnection attempt)
bool WiFiSetup::connect()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return true; // Already connected
    }

    Serial.println("Attempting WiFi reconnection...");
    
    // Try to reconnect using stored credentials
    WiFi.reconnect();

    const unsigned long startAttemptTime = millis();
    const unsigned long timeout = 30000; // 30 seconds timeout

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nReconnected to WiFi");
        Serial.printf("IP: %s, RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    }
    else
    {
        Serial.println("\nFailed to reconnect to WiFi");
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
    config.version = CONFIG_VERSION;
    size_t bytesWritten = prefs.putBytes("config", &config, sizeof(config));
    if (bytesWritten == 0) {
        Serial.println("ERROR: Failed to save config to preferences");
    } else {
        Serial.printf("Config saved successfully (%d bytes)\n", bytesWritten);
    }
}

// Load configuration parameters from Preferences
void WiFiSetup::paramLoad()
{
    // Reading the stored configuration from preferences
    size_t bytesRead = prefs.getBytes("config", &config, sizeof(config));
    
    if (bytesRead == 0) {
        Serial.println("No config found in preferences, using defaults");
        // Initialize with defaults
        config.version = CONFIG_VERSION;
        config.bright = DEFAULT_BRIGHTNESS;
        config.align = PA_CENTER;
        strncpy(config.mqtt_server, "localhost", sizeof(config.mqtt_server) - 1);
        config.mqtt_server[sizeof(config.mqtt_server) - 1] = '\0';
        strncpy(config.mqtt_port, "1883", sizeof(config.mqtt_port) - 1);
        config.mqtt_port[sizeof(config.mqtt_port) - 1] = '\0';
        return;
    }
    
    // Version check for config migration
    if (config.version != CONFIG_VERSION) {
        Serial.printf("Config version mismatch (found: %d, expected: %d), resetting to defaults\n", 
                     config.version, CONFIG_VERSION);
        config.version = CONFIG_VERSION;
        config.bright = DEFAULT_BRIGHTNESS;
        config.align = PA_CENTER;
    }
    
    // Validate and bound brightness
    if (config.bright > MAX_BRIGHTNESS) {
        Serial.printf("WARNING: Invalid brightness %d, capping to %d\n", config.bright, MAX_BRIGHTNESS);
        config.bright = MAX_BRIGHTNESS;
    }

    // Validate or set default values for configuration fields
    setDefaultIfEmpty(config.mqtt_port, "1883", sizeof(config.mqtt_port));
    setDefaultIfEmpty(config.mqtt_server, "localhost", sizeof(config.mqtt_server));
    
    // Ensure null-termination
    config.mqtt_server[sizeof(config.mqtt_server) - 1] = '\0';
    config.mqtt_port[sizeof(config.mqtt_port) - 1] = '\0';
    
    Serial.println("Config loaded successfully");
}

void WiFiSetup::setDefaultIfEmpty(char* field, const char* defaultValue, size_t fieldSize)
{
    if (strlen(field) == 0)
    {
        strncpy(field, defaultValue, fieldSize);
    }
}