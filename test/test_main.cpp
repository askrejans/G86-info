#include <Arduino.h>
#include <unity.h>

// Mock for Preferences class
class PreferencesMock
{
public:
    void putBytes(const char *key, const void *value, size_t size)
    {
    }

    void getBytes(const char *key, void *value, size_t size)
    {
    }
};

// Mock instance
PreferencesMock prefs;

class TimeTransformer
{
public:
    static void transformTime(String &timeString);
    static void messageReceived(String &topic, String &payload);

private:
    static unsigned long lastBlinkMillis;
};

// Define the static member outside the class
unsigned long TimeTransformer::lastBlinkMillis = 0;
char newMessage[256];
bool newMessageAvailable = false;
bool colonVisible = true;

void TimeTransformer::transformTime(String &timeString)
{
    // Check if the timeString has the expected format "HH:MM:SS"
    if (timeString.length() == 8 && timeString[2] == ':' && timeString[5] == ':')
    {
        // Extract the "HH:MM" part
        String transformedTime = timeString.substring(0, 5);

        // Replace the original timeString with the transformed one
        timeString = transformedTime;

        if (!colonVisible)
        {
            std::replace(timeString.begin(), timeString.end(), ':', ' ');
        }

        // Blink the colon every second
        if (millis() - lastBlinkMillis >= 1000)
        {
            lastBlinkMillis = millis();
            colonVisible = !colonVisible; // Toggle colon visibility
        }
    }
    else
    {
        Serial.println("Invalid time format: " + timeString);
    }
}

void TimeTransformer::messageReceived(String &topic, String &payload)
{
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
                transformTime(payload);
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

// Mock configuration structure
struct cfgParameter_t
{
    uint8_t brightness;   // display brightness
    char mqtt_server[40]; // mqtt server
    char mqtt_port[6];    // mqtt port
} Config;

// Declare an instance of the mock configuration
cfgParameter_t MockConfig;

class ConfigManager
{
public:
    static void paramSave();
    static void paramLoad();
};

void ConfigManager::paramSave()
{
    prefs.putBytes("config", &MockConfig, sizeof(MockConfig));
}

void ConfigManager::paramLoad()
{
    prefs.getBytes("config", &MockConfig, sizeof(MockConfig));
}

void test_transformTime_validFormat(void)
{
    String timeString = "12:34:56";
    unsigned long currentMillis = 1000;
    bool colonVisible = true;

    TimeTransformer::transformTime(timeString);

    TEST_ASSERT_EQUAL_STRING("12:34", timeString.c_str());
    TEST_ASSERT_EQUAL(true, colonVisible);
}

void test_led_builtin_pin_number(void)
{
    TEST_ASSERT_EQUAL(2, LED_BUILTIN);
}

void test_led_state_high(void)
{
    digitalWrite(LED_BUILTIN, HIGH);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(LED_BUILTIN));
}

void test_led_state_low(void)
{
    digitalWrite(LED_BUILTIN, LOW);
    TEST_ASSERT_EQUAL(LOW, digitalRead(LED_BUILTIN));
}

void test_messageReceived_GPS_TME(void)
{
    String topic = "some/topic/GPS/TME";
    String payload = "12:34:56";

    // Reset newMessageAvailable and newMessage
    newMessageAvailable = false;
    newMessage[0] = '\0';

    TimeTransformer::messageReceived(topic, payload);

    TEST_ASSERT_EQUAL_STRING("12 34", newMessage);
}

void test_messageReceived_GPS_DTE(void)
{
    String topic = "some/topic/GPS/DTE";
    String payload = "25.01234567.2022";

    // Reset newMessageAvailable and newMessage
    newMessageAvailable = false;
    newMessage[0] = '\0';

    TimeTransformer::messageReceived(topic, payload);

    TEST_ASSERT_EQUAL_STRING("25/01", newMessage);
}

void test_messageReceived_GPS_SPD(void)
{
    String topic = "some/topic/GPS/SPD";
    String payload = "45.678";

    // Reset newMessageAvailable and newMessage
    newMessageAvailable = false;
    newMessage[0] = '\0';

    TimeTransformer::messageReceived(topic, payload);

    TEST_ASSERT_EQUAL_STRING("45kmh", newMessage);
}

void test_messageReceived_GPS_ALT(void)
{
    String topic = "some/topic/GPS/ALT";
    String payload = "1234";

    // Reset newMessageAvailable and newMessage
    newMessageAvailable = false;
    newMessage[0] = '\0';

    TimeTransformer::messageReceived(topic, payload);

    TEST_ASSERT_EQUAL_STRING("1234m", newMessage);
}

void test_messageReceived_ECU_TPS(void)
{
    String topic = "some/topic/ECU/TPS";
    String payload = "75.3";

    // Reset newMessageAvailable and newMessage
    newMessageAvailable = false;
    newMessage[0] = '\0';

    TimeTransformer::messageReceived(topic, payload);

    TEST_ASSERT_EQUAL_STRING("75.3%", newMessage);
}

void test_messageReceived_ECU_MAT(void)
{
    String topic = "some/topic/ECU/MAT";
    String payload = "30.5";

    // Reset newMessageAvailable and newMessage
    newMessageAvailable = false;
    newMessage[0] = '\0';

    TimeTransformer::messageReceived(topic, payload);

    TEST_ASSERT_EQUAL_STRING("30.5C", newMessage);
}

void test_messageReceived_ECU_BAT(void)
{
    String topic = "some/topic/ECU/BAT";
    String payload = "12.8";

    // Reset newMessageAvailable and newMessage
    newMessageAvailable = false;
    newMessage[0] = '\0';

    TimeTransformer::messageReceived(topic, payload);

    TEST_ASSERT_EQUAL_STRING("12.8V", newMessage);
}

void test_messageReceived_ECU_DWL(void)
{
    String topic = "some/topic/ECU/DWL";
    String payload = "8.5";

    // Reset newMessageAvailable and newMessage
    newMessageAvailable = false;
    newMessage[0] = '\0';

    TimeTransformer::messageReceived(topic, payload);

    TEST_ASSERT_EQUAL_STRING("8.5ms", newMessage);
}

void test_paramSave(void)
{
    // Set values in the mock configuration
    MockConfig.brightness = 75; // Example brightness value
    strncpy(MockConfig.mqtt_server, "test_server", sizeof(MockConfig.mqtt_server));
    strncpy(MockConfig.mqtt_port, "1234", sizeof(MockConfig.mqtt_port));

    // Save the configuration
    ConfigManager::paramSave();
}

void test_paramLoad(void)
{
    // Set some initial values in preferences using the mock configuration
    prefs.putBytes("config", &MockConfig, sizeof(MockConfig));

    // Load the configuration
    ConfigManager::paramLoad();

    // Check if the loaded configuration matches the mock configuration
    TEST_ASSERT_EQUAL(MockConfig.brightness, 75); // Because brightness is not set in paramLoad
    TEST_ASSERT_EQUAL_STRING(MockConfig.mqtt_server, "test_server");
    TEST_ASSERT_EQUAL_STRING(MockConfig.mqtt_port, "1234");
}

uint8_t i = 0;
uint8_t max_blinks = 5;

void setup()
{
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    pinMode(LED_BUILTIN, OUTPUT);

    UNITY_BEGIN();
    // Test if board functions and is able to blink
    RUN_TEST(test_led_builtin_pin_number);
    // test time transform from GPS time
    RUN_TEST(test_transformTime_validFormat);

    // tests for messageReceived
    RUN_TEST(test_messageReceived_GPS_TME);
    RUN_TEST(test_messageReceived_GPS_DTE);
    RUN_TEST(test_messageReceived_GPS_SPD);
    RUN_TEST(test_messageReceived_GPS_ALT);
    RUN_TEST(test_messageReceived_ECU_TPS);
    RUN_TEST(test_messageReceived_ECU_MAT);
    RUN_TEST(test_messageReceived_ECU_BAT);
    RUN_TEST(test_messageReceived_ECU_DWL);

    // Test cases for paramSave and paramLoad
    RUN_TEST(test_paramSave);
    RUN_TEST(test_paramLoad);
}

void loop()
{
    if (i < max_blinks)
    {
        RUN_TEST(test_led_state_high);
        delay(500);
        RUN_TEST(test_led_state_low);
        delay(500);
        i++;
    }
    else if (i == max_blinks)
    {
        UNITY_END();
    }
}
