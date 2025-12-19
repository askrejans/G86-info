// WiFiSetup.h

#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <WiFiManager.h>
#include <Preferences.h>
#include <MD_Parola.h>

// Configuration constants
#define CONFIG_VERSION 1
#define MQTT_SERVER_SIZE 40
#define MQTT_PORT_SIZE 6
#define MAX_BRIGHTNESS 15
#define DEFAULT_BRIGHTNESS 5

/**
 * @brief Structure to hold configuration parameters.
 */
struct Config
{
    uint8_t version;              // Config version for migration
    uint8_t bright;               // Display intensity (0-15)
    textPosition_t align;         // Text alignment
    char mqtt_server[MQTT_SERVER_SIZE]; // MQTT server
    char mqtt_port[MQTT_PORT_SIZE];     // MQTT port
};

/**
 * @brief Class for managing WiFi setup and configuration.
 */
class WiFiSetup
{
public:
    /**
     * @brief Constructor for WiFiSetup class.
     */
    WiFiSetup();

    /**
     * @brief Initializes WiFiSetup.
     */
    void begin();

    /**
     * @brief Connects to WiFi.
     * @return True if connection is successful, false otherwise.
     */
    bool connect();

    /**
     * @brief Callback function to save configuration.
     */
    void saveConfigCallback();

    /**
     * @brief Saves configuration parameters.
     */
    void paramSave();

    /**
     * @brief Loads configuration parameters.
     */
    void paramLoad();

    Config config;     // Instance of the configuration structure
    Preferences prefs; // Instance of the Preferences library for persistent storage

    void setDefaultIfEmpty(char* field, const char* defaultValue, size_t fieldSize);

private:
    bool shouldSaveConfig = false; // Flag for saving data
};

// External declarations for global constants
extern const char *AP_NAME;       // Access point name
extern const char *WIFI_PASSWORD; // WiFi password

#endif // WIFI_SETUP_H
