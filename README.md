# Seeed XIAO ESP32C3 Development Workspace

This workspace is set up for developing projects with the **Seeed XIAO ESP32C3** microcontroller using PlatformIO and the Arduino framework.

## Hardware Overview

The Seeed XIAO ESP32C3 is a compact development board featuring:
- **ESP32-C3** RISC-V single-core processor
- **WiFi & Bluetooth 5.0** connectivity
- **400KB SRAM, 4MB Flash**
- **11 GPIO pins** including analog inputs
- **USB-C connector** for programming and power
- **Built-in button (BOOT button)**
- **Compact form factor** (20x17.5mm)

**Note:** The XIAO ESP32C3 does NOT have a built-in LED. External NeoPixels or other LED components must be connected for lighting effects.

## Pin Configuration

| Pin | Function | Notes |
|-----|----------|-------|
| 0   | A0/D0    | Analog input (12-bit ADC) |
| 1   | A1/D1    | Analog input (12-bit ADC) |
| 2   | A2/D2    | Analog input (12-bit ADC) |
| 3   | A3/D3    | Analog input (12-bit ADC) |
| 4   | D4       | Digital I/O |
| 5   | D5       | Digital I/O |
| 6   | D6/SDA   | I2C Data |
| 7   | D7/SCL   | I2C Clock |
| 8   | D8       | Digital I/O, SPI SCK |
| 9   | D9/BTN   | Built-in button (BOOT), SPI MISO |
| 10  | D10      | Digital I/O, SPI MOSI, **NeoPixels** |

## Current Project: NeoPixel Web Controller

The main project (`src/main.cpp`) is a **NeoPixel web controller** that provides:

### 🌈 **Features**
- **Rainbow Mode**: Automatic color cycling across all pixels
- **Static Color Mode**: Set any custom color via web interface
- **Brightness Control**: Adjustable brightness from 1-255
- **Web Interface**: Control via browser or REST API
- **WebSocket Support**: Real-time updates and control
- **Button Control**: Toggle between modes using the BOOT button

### 🌐 **Web Interface**
- **Main Page**: `/` - Interactive color picker and controls
- **Rainbow Mode**: `/rainbow` - Activate automatic color cycling
- **Color Control**: `/color?value=FF0000` - Set static color (hex format)
- **Brightness**: `/brightness?value=128` - Set brightness (1-255)
- **Status**: `/status` - Get current mode and settings (JSON)

### 🔌 **Hardware Requirements**
- **NeoPixels**: Connect WS2812B LED strip to pin D10 (pin 10)
- **Power Supply**: 5V power for NeoPixels (if using more than a few pixels)
- **WiFi Network**: For web interface access

### 📱 **Usage**
1. Connect NeoPixels to pin D10
2. Upload the firmware
3. Connect to the WiFi network
4. Open the web interface at the device's IP address
5. Use the color picker or REST API to control the lights

## Getting Started

### Prerequisites

1. **PlatformIO IDE** - Install from [platformio.org](https://platformio.org/)
   - VS Code extension recommended
   - Or use PlatformIO Core CLI

2. **USB Driver** - Usually auto-detected on modern systems
   - For Windows: Install CP210x driver if needed

3. **Hardware Components**
   - XIAO ESP32C3 board
   - WS2812B NeoPixels (recommended: 5-30 pixels)
   - 5V power supply (if using many pixels)
   - Jumper wires

### Quick Start

1. **Clone or download this workspace**
   ```bash
   cd ~/seeed-xiao-esp32c3-workspace
   ```

2. **Connect Hardware**
   - Connect NeoPixels to pin D10 (pin 10)
   - Connect power and ground for NeoPixels
   - Connect XIAO ESP32C3 via USB-C

3. **Configure WiFi**
   - Update `ssid` and `password` in `src/main.cpp`
   - Or modify the WiFi credentials in the code

4. **Build and upload**
   - Press Upload button in PlatformIO
   - Or use CLI: `pio run -t upload`

5. **Access the web interface**
   - Check serial monitor for IP address
   - Open browser to `http://[IP_ADDRESS]`
   - Control your NeoPixels!

### Project Structure

```
seeed-xiao-esp32c3-workspace/
├── src/
│   └── main.cpp          # NeoPixel web controller
├── include/              # Header files
│   └── xiao_pins.h      # Pin definitions
├── lib/                  # Private libraries
├── examples/             # Example code
│   ├── wifi_scanner.cpp  # WiFi network scanner
│   ├── analog_read.cpp   # Analog input reading
│   ├── i2c_scanner.cpp   # I2C device scanner
│   └── deep_sleep.cpp    # Deep sleep functionality
├── data/                 # Web interface files
│   └── index.html        # Main web page
├── docs/                 # Documentation
├── platformio.ini        # PlatformIO configuration
└── README.md            # This file
```

## Example Projects

### 1. NeoPixel Web Controller (main.cpp)
The main project provides a complete NeoPixel control system:
- Web-based color picker and controls
- Rainbow mode with automatic color cycling
- Brightness control
- WebSocket real-time updates
- REST API for programmatic control

### 2. WiFi Scanner (examples/wifi_scanner.cpp)
Scans and displays available WiFi networks with:
- SSID, signal strength, channel
- Encryption type detection
- Formatted output table

### 3. Analog Reading (examples/analog_read.cpp)
Demonstrates analog input capabilities:
- Reading from pins A0-A3
- ADC resolution configuration
- Voltage conversion

### 4. I2C Scanner (examples/i2c_scanner.cpp)
Scans for I2C devices on pins D6 (SDA) and D7 (SCL)

### 5. Deep Sleep (examples/deep_sleep.cpp)
Power management examples for battery-powered applications

## Troubleshooting

### Common Issues

1. **NeoPixels not working**
   - Check power supply (5V required)
   - Verify connection to pin D10
   - Ensure proper ground connection

2. **WiFi connection fails**
   - Verify SSID and password in code
   - Check WiFi network availability
   - Ensure ESP32C3 is in range

3. **Web interface not accessible**
   - Check serial monitor for IP address
   - Verify WiFi connection status
   - Check firewall settings

4. **Upload fails**
   - Ensure correct USB port selection
   - Check USB cable connection
   - Try pressing BOOT button during upload

### Serial Monitor Output
The device provides detailed status information via serial monitor:
- WiFi connection status
- IP address assignment
- Web server startup confirmation
- NeoPixel initialization
- Button press events

## Development

### Adding New Features
- Modify `src/main.cpp` for main functionality
- Add new web routes in `setupWebServer()`
- Extend WebSocket handling in `webSocketEvent()`
- Update pin definitions in `include/xiao_pins.h`

### Libraries Used
- **Adafruit NeoPixel**: LED strip control
- **WebSockets**: Real-time communication
- **LittleFS**: File system for web interface
- **WebServer**: HTTP server functionality
- **WiFi**: Network connectivity

## License

This project is open source. Feel free to modify and distribute according to your needs.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.
