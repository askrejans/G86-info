# Quick Start Guide - G86-INFO

## Installation

### 1. Build & Upload
```bash
cd /Users/arvis/dev/G86-info
~/.platformio/penv/bin/platformio run --target upload
```

### 2. Monitor Serial Output
```bash
~/.platformio/penv/bin/platformio device monitor
```

### 3. Initial Configuration

**First Boot:**
- ESP32 creates WiFi AP: `G86-INFO-AP`
- Password: `golf1986`
- Connect to AP with phone/computer
- Captive portal opens automatically
- Enter your WiFi credentials
- Enter MQTT server IP and port
- Click "Save"

**System will:**
- Connect to your WiFi
- Connect to MQTT broker
- Display "Golf'86" welcome message
- Start secondary display with "GOLF'86" scroll

## Verifying Operation

### Check Serial Output
You should see:
```
Initializing shared data synchronization...
Shared data synchronization initialized
WiFi connected!
Local IP: 192.168.x.x
MQTT Primary connected!
MQTT Secondary connected!
7-Segment display initialized
HTTP server started
```

### Check Web Interface
Open browser to: `http://[device-ip]/`

Should display:
- Uptime
- Free heap
- WiFi signal strength
- MQTT server/port

### Test Display Modes

**Main Display (DOT matrix):**
- Default: Scrolling welcome message
- Menu: Rotate encoder to access settings
- Data: Select ECU/GPS parameters via menu

**Secondary Display (7-segment):**
- WELCOME: Scrolling "GOLF'86"
- MQTT: Selected ECU/GPS data
- TIMER1/TIMER2: Chronometer

## Menu Navigation

**Rotary Encoder:**
- Rotate: Navigate menu items
- Press: Select/confirm

**Menu Items:**
- `P:ECU` - Primary display ECU data selection
- `P:GPS` - Primary display GPS data selection
- `S:ECU` - Secondary display ECU data selection
- `S:GPS` - Secondary display GPS data selection
- `POS` - Text alignment (Left/Center/Right)
- `BRT` - Brightness (0-15)

**Auto-Exit:** Menu closes after 3 seconds of inactivity

## Timer Controls

**Buttons:**
- SW1 (Pin 14): Start/Pause active timer
- SW2 (Pin 12): Reset active timer

**Toggle Switches:**
- TG1 (Pin 33): Select Timer 1
- TG2 (Pin 13): Select Timer 2

**MQTT Topics:**
- `/GOLF86/TM1/value` - Timer 1 value
- `/GOLF86/TM1/started` - Timer 1 running state
- `/GOLF86/TM1/paused` - Timer 1 paused state
- `/GOLF86/TM2/...` - Timer 2 equivalent

## Troubleshooting

### System Won't Boot
**Check:**
- Serial output for error messages
- Power supply (5V, 2A minimum)
- USB cable quality

**Try:**
- Erase flash: `~/.platformio/penv/bin/platformio run --target erase`
- Re-upload firmware

### WiFi Won't Connect
**Check:**
- SSID and password correct
- 2.4GHz network (ESP32 doesn't support 5GHz)
- Router within range (check RSSI in web interface)

**Try:**
- Reset WiFi settings: Hold reset button during power-on
- Check serial output for connection attempts

### MQTT Not Connecting
**Check:**
- MQTT broker running and accessible
- Correct IP address and port (default: 1883)
- Firewall rules allow connection
- Credentials correct (default: public/public)

**Serial Output:**
```
Connecting to MQTT Primary at [ip]:[port]
...................
MQTT Primary connected!
```

If you see 20 dots without "connected", broker is unreachable.

## Configuration Reset

### Method 1: Serial Command
In development - not yet implemented

### Method 2: Erase Flash
```bash
~/.platformio/penv/bin/platformio run --target erase
~/.platformio/penv/bin/platformio run --target upload
```

### Method 3: Code Modification
In `WiFiSetup.cpp`, temporarily add:
```cpp
void WiFiSetup::begin() {
    prefs.clear();  // Add this line
    // ... rest of code
}
```

Upload, boot once, remove line, re-upload.

## Performance Monitoring

### Serial Logging
Enable verbose logging by uncommenting in `Constants.h`:
```cpp
#define DEBUG_ENABLED
```

### Web Interface
Access real-time stats:
- `http://[device-ip]/` - System overview
- Check every few hours during initial testing

### MQTT Monitoring
Subscribe to all topics:
```bash
mosquitto_sub -h [broker-ip] -t "/GOLF86/#" -v
```
