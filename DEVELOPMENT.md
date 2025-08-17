# Development Guide for Cursor (without PlatformIO IDE Extension)

## Quick Commands

### Build and Upload
- **Build**: `Ctrl+Shift+B` or `Cmd+Shift+B`
- **Upload**: `Ctrl+Shift+U` or `Cmd+Shift+U`
- **Clean**: `Ctrl+Shift+C` or `Cmd+Shift+C`
- **Monitor**: `Ctrl+Shift+M` or `Cmd+Shift+M`

### Alternative: Command Palette
- `Ctrl+Shift+P` → "Tasks: Run Task" → Select your task

## Development Workflow

1. **Edit your code** in `src/main.cpp`
2. **Build** to check for errors: `Ctrl+Shift+B`
3. **Upload** to your ESP32C3: `Ctrl+Shift+U`
4. **Monitor** serial output: `Ctrl+Shift+M`

## Manual Commands (Terminal)

If you prefer using the terminal directly:

```bash
# Build project
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor

# Clean build files
pio run --target clean

# List connected devices
pio device list

# Update libraries
pio lib update
```

## Troubleshooting

### Build Errors
- Check your code syntax
- Verify library dependencies in `platformio.ini`
- Run `pio lib update` to update libraries

### Upload Issues
- Ensure your ESP32C3 is connected via USB
- Check the correct COM port is selected
- Try pressing the BOOT button during upload if needed

### Serial Monitor Issues
- Make sure the correct port is selected
- Check baud rate (115200)
- Ensure no other application is using the serial port

## Project Structure

- `src/main.cpp` - Main Arduino code
- `platformio.ini` - PlatformIO configuration
- `include/` - Header files
- `lib/` - Custom libraries
- `.vscode/` - Cursor/VS Code configuration files

## WiFi Configuration

Update these values in `src/main.cpp`:
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

## Features

Your ESP32C3 project includes:
- WiFi connectivity with web interface
- NeoPixel LED control (6 LEDs on pin D10)
- WebSocket server for real-time communication
- Web server for configuration
- Rainbow and static color modes
