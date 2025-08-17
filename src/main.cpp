/**
 * Seeed XIAO ESP32C3 Basic Template
 * 
 * This template provides a starting point for XIAO ESP32C3 development
 * with common pin definitions and basic setup.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>

// Forward declarations
String getPatternName(uint8_t pattern);
void loadPreferences();
void savePreferences();

// XIAO ESP32C3 Pin Definitions
// Note: The XIAO ESP32C3 does NOT have a built-in LED
#define BUTTON_PIN      9    // Built-in button (BOOT button)

// Analog pins
#define A0_PIN          0    // A0/D0
#define A1_PIN          1    // A1/D1  
#define A2_PIN          2    // A2/D2
#define A3_PIN          3    // A3/D3

// Digital pins
#define D4_PIN          4    // D4
#define D5_PIN          5    // D5
#define D6_PIN          6    // D6/SDA
#define D7_PIN          7    // D7/SCL
#define D8_PIN          8    // D8
#define D9_PIN          9    // D9 (same as BUTTON_PIN)
#define D10_PIN         10   // D10

// SPI pins
#define SPI_MOSI        10   // D10/MOSI
#define SPI_MISO        9    // D9/MISO  
#define SPI_SCK         8    // D8/SCK
#define SPI_SS          7    // D7/SS

// I2C pins
#define I2C_SDA         6    // D6/SDA
#define I2C_SCL         7    // D7/SCL

// UART pins
#define UART_TX         21   // TX
#define UART_RX         20   // RX

// NeoPixel configuration
// Note: Connect your external NeoPixels to D10 (pin 10)
#define NEOPIXEL_PIN    10   // D10 pin for NeoPixels
#define NUM_PIXELS      60    // Number of NeoPixels
#define GRID_WIDTH      6     // Grid width (columns)
#define GRID_HEIGHT     10    // Grid height (rows)
#define BRIGHTNESS       64   // Brightness (0-255) - 25% of max

// WiFi configuration - Access Point mode
const char* ap_ssid = "LithophaneController";
const char* ap_password = "12345678";  // 8 character minimum for ESP32
const char* hostname = "lithophane";   // Hostname for the device

// Create NeoPixel objects
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Create web server object
WebServer server(80);

// Create WebSocket server object
WebSocketsServer webSocket = WebSocketsServer(81);

// Create Preferences object for persistent storage
Preferences preferences;

// Global variables for patterns
uint8_t currentPattern = 2;           // Current pattern (0=Rainbow, 1=Static, 2=Wave, 3=Fire, 4=Matrix, 5=Spiral, 6=Pulse) - Start on Wave
uint32_t previousPatternMillis = 0;   // Last pattern update time
uint32_t patternInterval = 50;        // Pattern update interval (ms)
uint8_t patternStep = 0;              // Pattern step counter
uint16_t waveOffset = 0;              // Wave pattern offset
uint8_t fireIntensity = 0;            // Fire intensity
uint16_t rainbowHue = 0;              // Rainbow hue
uint32_t staticColor = 0xFF0000;     // Static color (default red)
uint8_t currentBrightness = 64;       // Current brightness (25% of max)

// Auto-cycle variables
bool autoCycleEnabled = false;        // Whether auto-cycling is enabled
uint32_t autoCycleInterval = 6000;    // Auto-cycle interval (6 seconds default)
uint32_t lastAutoCycleMillis = 0;     // Last auto-cycle time

// Command throttling variables
unsigned long lastBrightnessUpdate = 0;
unsigned long lastColorUpdate = 0;
const long commandThrottleMs = 50;

// Function to broadcast current status to all WebSocket clients
void broadcastStatus() {
  String mode = getPatternName(currentPattern);
  
  // Always send the saved static color for the color picker display
  uint8_t r = (staticColor >> 16) & 0xFF;
  uint8_t g = (staticColor >> 8) & 0xFF;
  uint8_t b = staticColor & 0xFF;
  String colorHex = "#";
  if (r < 16) colorHex += "0";
  colorHex += String(r, HEX);
  if (g < 16) colorHex += "0";
  colorHex += String(g, HEX);
  if (b < 16) colorHex += "0";
  colorHex += String(b, HEX);
  colorHex.toUpperCase();
  
  String json = "{\"type\":\"status\",\"mode\":\"" + mode + "\",\"pattern\":" + String(currentPattern) + ",\"color\":\"" + colorHex + "\",\"brightness\":" + String(currentBrightness) + ",\"autoCycle\":" + String(autoCycleEnabled ? "true" : "false") + ",\"autoCycleInterval\":" + String(autoCycleInterval) + "}";
  
  // Debug output to serial
  Serial.printf("Broadcasting status - Mode: %s, Pattern: %d, Color: %s, Brightness: %d\n", 
                mode.c_str(), currentPattern, colorHex.c_str(), currentBrightness);
  Serial.printf("JSON: %s\n", json.c_str());
  
  webSocket.broadcastTXT(json);
}

// WebSocket event handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      
      // Send current status to newly connected client
      broadcastStatus();
      break;
    }
    
    case WStype_TEXT: {
      Serial.printf("[%u] get Text: %s\n", num, payload);
      
      String message = String((char*)payload);
      
      // Parse JSON commands
      if (message.indexOf("\"command\":\"rainbow\"") > -1) {
        currentPattern = 0; // Rainbow
        savePreferences();
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"static\"") > -1) {
        currentPattern = 1; // Static
        savePreferences();
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"wave\"") > -1) {
        currentPattern = 2; // Wave
        savePreferences();
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"fire\"") > -1) {
        currentPattern = 3; // Fire
        savePreferences();
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"matrix\"") > -1) {
        currentPattern = 4; // Matrix
        savePreferences();
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"spiral\"") > -1) {
        currentPattern = 5; // Spiral
        savePreferences();
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"pulse\"") > -1) {
        currentPattern = 6; // Pulse
        savePreferences();
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"next\"") > -1) {
        currentPattern = (currentPattern + 1) % 7; // Cycle to next pattern
        savePreferences();
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"color\"") > -1) {
        unsigned long now = millis();
        if (now - lastColorUpdate >= commandThrottleMs) {
          lastColorUpdate = now;
          
          // Extract color value from JSON
          int colorStart = message.indexOf("\"value\":\"") + 9;
          int colorEnd = message.indexOf("\"", colorStart);
          if (colorStart > 8 && colorEnd > colorStart) {
            String colorStr = message.substring(colorStart, colorEnd);
            long hexColor = strtol(colorStr.c_str(), NULL, 16);
            uint8_t r = (hexColor >> 16) & 0xFF;
            uint8_t g = (hexColor >> 8) & 0xFF;
            uint8_t b = hexColor & 0xFF;
            
            staticColor = pixels.Color(r, g, b);
            if (currentPattern != 5) { // Spiral
              currentPattern = 1; // Static
            }
            
            // Update pixels immediately
            for (int i = 0; i < NUM_PIXELS; i++) {
              pixels.setPixelColor(i, staticColor);
            }
            pixels.show();
            
            savePreferences(); // Save the color to flash
            broadcastStatus();
          }
        }
      }
      else if (message.indexOf("\"command\":\"brightness\"") > -1) {
        unsigned long now = millis();
        if (now - lastBrightnessUpdate >= commandThrottleMs) {
          lastBrightnessUpdate = now;
          
          // Extract brightness value from JSON
          int brightStart = message.indexOf("\"value\":") + 8;
          int brightEnd = message.indexOf("}", brightStart);
          if (brightStart > 7 && brightEnd > brightStart) {
            String brightStr = message.substring(brightStart, brightEnd);
            int brightness = brightStr.toInt();
            if (brightness >= 1 && brightness <= 255) {
              currentBrightness = brightness;
              pixels.setBrightness(currentBrightness);
              
              // Update current display immediately
              if (currentPattern == 0) { // Rainbow
                uint32_t color = pixels.gamma32(pixels.ColorHSV(rainbowHue));
                for (int i = 0; i < NUM_PIXELS; i++) {
                  pixels.setPixelColor(i, color);
                }
              } else if (currentPattern == 1) { // Static
                for (int i = 0; i < NUM_PIXELS; i++) {
                  pixels.setPixelColor(i, staticColor);
                }
              }
              pixels.show();
              
              broadcastStatus();
            }
          }
        }
      }
      else if (message.indexOf("\"command\":\"status\"") > -1) {
        // Client requesting status update
        broadcastStatus();
      }
      else if (message.indexOf("\"command\":\"autoCycle\"") > -1) {
        autoCycleEnabled = !autoCycleEnabled;
        savePreferences(); // Save the auto-cycle state
        broadcastStatus();
        Serial.printf("Auto-cycling enabled: %s\n", autoCycleEnabled ? "true" : "false");
      }
      else if (message.indexOf("\"command\":\"autoCycleInterval\"") > -1) {
        int interval = message.indexOf("\"value\":") + 8;
        int end = message.indexOf("}", interval);
        if (interval > 7 && end > interval) {
          String intervalStr = message.substring(interval, end);
          autoCycleInterval = intervalStr.toInt();
          savePreferences(); // Save the interval setting
          Serial.printf("Auto-cycle interval set to: %d ms\n", autoCycleInterval);
        }
        broadcastStatus();
      }
      break;
    }
    
    default:
      break;
  }
}

// Function to setup web server routes
void setupWebServer() {
  // Test endpoint
  server.on("/test", []() {
    Serial.println("Test endpoint hit!");
    server.send(200, "text/plain", "Web server is working!");
  });
  
  // Main page - serve from filesystem
  server.on("/", []() {
    File file = LittleFS.open("/index.html", "r");
    if (file) {
      server.streamFile(file, "text/html");
      file.close();
    } else {
      server.send(404, "text/plain", "File not found");
    }
  });
  
  // Set rainbow mode
  server.on("/rainbow", []() {
    currentPattern = 0; // Rainbow
    server.send(200, "text/plain", "Rainbow mode activated");
  });
  
  // Set static mode
  server.on("/static", []() {
    currentPattern = 1; // Static
    server.send(200, "text/plain", "Static mode activated");
  });
  
  // Set wave mode
  server.on("/wave", []() {
    currentPattern = 2; // Wave
    server.send(200, "text/plain", "Wave mode activated");
  });
  
  // Set fire mode
  server.on("/fire", []() {
    currentPattern = 3; // Fire
    server.send(200, "text/plain", "Fire mode activated");
  });
  
  // Set matrix mode
  server.on("/matrix", []() {
    currentPattern = 4; // Matrix
    server.send(200, "text/plain", "Matrix mode activated");
  });
  
  // Set spiral mode
  server.on("/spiral", []() {
    currentPattern = 5; // Spiral
    server.send(200, "text/plain", "Spiral mode activated");
  });
  
  // Set pulse mode
  server.on("/pulse", []() {
    currentPattern = 6; // Pulse
    server.send(200, "text/plain", "Pulse mode activated");
  });
  
  // Next pattern
  server.on("/next", []() {
    currentPattern = (currentPattern + 1) % 7; // Cycle to next pattern
    server.send(200, "text/plain", "Next pattern activated");
  });
  
  // Set static color
  server.on("/color", []() {
    if (server.hasArg("value")) {
      String colorStr = server.arg("value");
      // Convert hex string to RGB values
      long hexColor = strtol(colorStr.c_str(), NULL, 16);
      uint8_t r = (hexColor >> 16) & 0xFF;
      uint8_t g = (hexColor >> 8) & 0xFF;
      uint8_t b = hexColor & 0xFF;
      
      staticColor = pixels.Color(r, g, b);
      currentPattern = 1; // Static
      
      // Set all pixels to the static color immediately
      for (int i = 0; i < NUM_PIXELS; i++) {
        pixels.setPixelColor(i, staticColor);
      }
      pixels.show();
      
      server.send(200, "text/plain", "Color set to #" + colorStr);
    } else {
      server.send(400, "text/plain", "Missing color value");
    }
  });
  
  // Set brightness
  server.on("/brightness", []() {
    if (server.hasArg("value")) {
      int brightness = server.arg("value").toInt();
      if (brightness >= 1 && brightness <= 255) {
        currentBrightness = brightness;
        pixels.setBrightness(currentBrightness);
        
        // Update current display immediately
        if (currentPattern == 0) { // Rainbow
          // Apply current rainbow color with new brightness
          uint32_t color = pixels.gamma32(pixels.ColorHSV(rainbowHue));
          for (int i = 0; i < NUM_PIXELS; i++) {
            pixels.setPixelColor(i, color);
          }
        } else if (currentPattern == 1) { // Static
          // Apply static color with new brightness
          for (int i = 0; i < NUM_PIXELS; i++) {
            pixels.setPixelColor(i, staticColor);
          }
        }
        pixels.show();
        
        server.send(200, "text/plain", "Brightness set to " + String(brightness));
      } else {
        server.send(400, "text/plain", "Invalid brightness value. Use 1-255.");
      }
    } else {
      server.send(400, "text/plain", "Missing brightness value");
    }
  });
  
  // Status endpoint
  server.on("/status", []() {
    String mode = getPatternName(currentPattern);
    String colorHex = "";
    if (currentPattern == 1) { // Static mode
      uint8_t r = (staticColor >> 16) & 0xFF;
      uint8_t g = (staticColor >> 8) & 0xFF;
      uint8_t b = staticColor & 0xFF;
      colorHex = "#" + String(r, HEX) + String(g, HEX) + String(b, HEX);
      colorHex.toUpperCase();
    }
    String json = "{\"mode\":\"" + mode + "\",\"pattern\":" + String(currentPattern) + ",\"color\":\"" + colorHex + "\",\"brightness\":" + String(currentBrightness) + "}";
    server.send(200, "application/json", json);
  });
}

// Function to load saved preferences
void loadPreferences() {
  preferences.begin("lithophane", false); // false = read/write mode
  
  // Always start on Wave pattern - don't save/load it
  currentPattern = 2; // Wave
  
  currentBrightness = preferences.getUChar("brightness", 64);
  autoCycleEnabled = preferences.getBool("autoCycle", false);
  autoCycleInterval = preferences.getULong("autoCycleInt", 6000); // Default to 6 seconds
  
  // Load static color
  uint32_t savedColor = preferences.getULong("staticColor", 0xFF0000);
  if (savedColor != 0) {
    staticColor = savedColor;
  }
  
  Serial.println("Preferences loaded from flash:");
  Serial.printf("  Pattern: %d (%s) - Always Wave on startup\n", currentPattern, getPatternName(currentPattern).c_str());
  Serial.printf("  Brightness: %d\n", currentBrightness);
  Serial.printf("  Auto-cycle: %s\n", autoCycleEnabled ? "enabled" : "disabled");
  Serial.printf("  Auto-cycle interval: %d ms\n", autoCycleInterval);
  Serial.printf("  Static color: 0x%06X\n", staticColor);
}

// Function to save preferences
void savePreferences() {
  // Don't save pattern - always start on Wave
  preferences.putUChar("brightness", currentBrightness);
  preferences.putBool("autoCycle", autoCycleEnabled);
  preferences.putULong("autoCycleInt", autoCycleInterval);
  preferences.putULong("staticColor", staticColor);
  
  Serial.println("Preferences saved to flash:");
  Serial.printf("  Pattern: %d (%s) - Not saved (always Wave on startup)\n", currentPattern, getPatternName(currentPattern).c_str());
  Serial.printf("  Brightness: %d\n", currentBrightness);
  Serial.printf("  Auto-cycle: %s\n", autoCycleEnabled ? "enabled" : "disabled");
  Serial.printf("  Auto-cycle interval: %d ms\n", autoCycleInterval);
  Serial.printf("  Static color: 0x%06X\n", staticColor);
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  Serial.println("Seeed XIAO ESP32C3 Starting...");
  
  // Initialize random seed for matrix effect
  randomSeed(analogRead(A0_PIN));
  
  // Initialize NeoPixels
  pixels.begin();
  pixels.setBrightness(currentBrightness);
  pixels.clear();
  pixels.show();
  Serial.println("NeoPixels initialized!");
  
  // Initialize filesystem
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
  } else {
    Serial.println("LittleFS mounted successfully!");
  }
  
  // Load preferences
  loadPreferences();
  
  // Initialize WiFi
  Serial.println("Setting up Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.hostname(hostname);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.printf("Access Point \"%s\" created with IP: %s\n", ap_ssid, WiFi.softAPIP().toString().c_str());
  Serial.printf("You can also access it at: %s.local\n", hostname);
  
  // Setup web server routes
  setupWebServer();
  server.begin();
  Serial.println("Web server started!");
  
  // Setup and start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started on port 81!");
  
  Serial.printf("Access your device at: %s\n", WiFi.softAPIP().toString().c_str());
  
  // Print chip information
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  
  Serial.println("Setup complete!");
}

// Function to create rainbow effect - all pixels same color
void rainbowCycle() {
  if (currentPattern == 0) { // Rainbow
    // Convert current hue to RGB color
    uint32_t color = pixels.gamma32(pixels.ColorHSV(rainbowHue));
    
    // Set all pixels to the same color
    for (int i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, color);
    }
    pixels.show();
    
    // Increment hue for next cycle
    rainbowHue += 256;
    if (rainbowHue >= 65536) {
      rainbowHue = 0;
    }
  }
}

// Function to set static color
void setStaticColor() {
  if (currentPattern == 1) { // Static
    for (int i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, staticColor);
    }
    pixels.show();
  }
}

// Function to get pixel index from grid coordinates (serpentine pattern)
uint16_t getPixelIndex(uint8_t col, uint8_t row) {
  if (col >= GRID_WIDTH || row >= GRID_HEIGHT) return 0;
  
  // Serpentine pattern: down then up, left to right
  if (col % 2 == 0) {
    // Even columns: top to bottom
    return col * GRID_HEIGHT + row;
  } else {
    // Odd columns: bottom to top
    return col * GRID_HEIGHT + (GRID_HEIGHT - 1 - row);
  }
}

// Function to create wave effect
void waveEffect() {
  if (currentPattern == 2) { // Wave
    // Debug output every 50 frames
    if (patternStep % 50 == 0) {
      Serial.printf("Wave Debug - waveOffset: %d, patternStep: %d\n", waveOffset, patternStep);
    }
    
    for (int col = 0; col < GRID_WIDTH; col++) {
      for (int row = 0; row < GRID_HEIGHT; row++) {
        uint16_t pixelIndex = getPixelIndex(col, row);
        
        // Create rainbow effect that travels right to left
        // Calculate hue based on column position and time
        uint16_t hue = ((waveOffset * 300) - (col * 10922)) % 65536; // Faster movement, reverse direction
        
        // Debug first few pixels every 50 frames
        if (patternStep % 50 == 0 && col < 3 && row == 0) {
          Serial.printf("Col %d: hue=%d, waveOffset=%d\n", col, hue, waveOffset);
        }
        
        // Add some wave variation based on row for more dynamic effect
        float wave = sin((row * 0.5 + waveOffset * 0.1) * PI / 180.0);
        uint8_t saturation = 255; // Full saturation for vibrant colors
        uint8_t value = 200 + (wave * 55); // Vary brightness slightly with wave
        
        uint32_t color = pixels.gamma32(pixels.ColorHSV(hue, saturation, value));
        pixels.setPixelColor(pixelIndex, color);
      }
    }
    pixels.show();
    
    waveOffset++;
    patternStep++;
  }
}

// Function to create fire effect
void fireEffect() {
  if (currentPattern == 3) { // Fire
    // Create fire effect with orange/yellow base and red tips
    for (int i = 0; i < NUM_PIXELS; i++) {
      // Get grid coordinates from pixel index using correct serpentine layout
      uint8_t col = i / GRID_HEIGHT;
      uint8_t row;
      
      if (col % 2 == 0) {
        // Even columns: top to bottom
        row = i % GRID_HEIGHT;
      } else {
        // Odd columns: bottom to top
        row = GRID_HEIGHT - 1 - (i % GRID_HEIGHT);
      }
      
      // Calculate distance from bottom (fire source)
      // Top row is row 0, bottom row is row 9
      uint8_t distanceFromBottom = GRID_HEIGHT - 1 - row;
      
      // Base fire intensity decreases with height (stronger at bottom)
      uint8_t baseIntensity = max(0, 255 - (distanceFromBottom * 40));
      
      // Add some variation based on column position
      uint8_t columnVariation = (col * 17 + patternStep * 3) % 50;
      baseIntensity = max(0, min(255, baseIntensity + columnVariation - 25));
      
      // Create fire colors: red at bottom, orange in middle, yellow at top
      uint16_t hue;
      if (distanceFromBottom < 3) {
        // Bottom: red-orange
        hue = 0; // Red
      } else if (distanceFromBottom < 6) {
        // Middle: orange
        hue = 3000; // Orange
      } else {
        // Top: yellow
        hue = 6000; // Yellow
      }
      
      // Add sparkle effect with orange/yellow
      uint8_t sparkleChance = (i * 13 + patternStep * 7) % 100;
      if (sparkleChance < 15) { // 15% chance for sparkles
        // Random sparkle color: orange, yellow, or bright orange
        uint16_t sparkleHues[] = {3000, 6000, 2500}; // Orange, Yellow, Bright Orange
        hue = sparkleHues[(i + patternStep) % 3];
        baseIntensity = min(255, baseIntensity + 50); // Make sparkles brighter
      }
      
      // Add random white flashes near the top
      if (distanceFromBottom <= 2 && (i * 19 + patternStep * 11) % 200 < 3) { // 1.5% chance for white flashes
        hue = 0; // White (hue 0 with high saturation)
        baseIntensity = 255; // Full brightness for flashes
      }
      
      // Calculate saturation and value
      uint8_t saturation = max(200, 255 - (distanceFromBottom * 10)); // Higher saturation at bottom
      uint8_t value = baseIntensity;
      
      // Create the color
      uint32_t color = pixels.gamma32(pixels.ColorHSV(hue, saturation, value));
      pixels.setPixelColor(i, color);
    }
    
    pixels.show();
    patternStep++;
  }
}

// Function to create matrix effect
void matrixEffect() {
  if (currentPattern == 4) { // Matrix
    // Create falling green "code" effect
    for (int col = 0; col < GRID_WIDTH; col++) {
      // Random chance to start a new "drop" at the top
      if (random(100) < 15) {
        uint16_t pixelIndex = getPixelIndex(col, 0); // Top of column
        pixels.setPixelColor(pixelIndex, pixels.Color(0, 255, 0)); // Bright green
      }
      
      // Move existing drops down each column
      for (int row = GRID_HEIGHT - 1; row > 0; row--) {
        uint16_t currentPixel = getPixelIndex(col, row);
        uint16_t abovePixel = getPixelIndex(col, row - 1);
        
        uint32_t aboveColor = pixels.getPixelColor(abovePixel);
        if (aboveColor != 0) {
          // Move the color down one row
          pixels.setPixelColor(currentPixel, aboveColor);
          pixels.setPixelColor(abovePixel, 0); // Clear the above pixel
        }
      }
      
      // Fade out pixels at the bottom
      uint16_t bottomPixel = getPixelIndex(col, GRID_HEIGHT - 1);
      uint32_t bottomColor = pixels.getPixelColor(bottomPixel);
      if (bottomColor != 0) {
        // Extract green component and fade it
        uint8_t g = (bottomColor >> 8) & 0xFF;
        if (g > 20) {
          g -= 20; // Fade out
          pixels.setPixelColor(bottomPixel, pixels.Color(0, g, 0));
        } else {
          pixels.setPixelColor(bottomPixel, 0); // Turn off when too dim
        }
      }
    }
    pixels.show();
  }
}

// Function to create spiral effect
void spiralEffect() {
  if (currentPattern == 5) { // Spiral
    // Create a proper spiral using polar coordinates with serpentine grid indexing
    uint8_t centerCol = GRID_WIDTH / 2;
    uint8_t centerRow = GRID_HEIGHT / 2;
    
    // Calculate total pixels in the spiral and current pixel to light
    uint8_t totalPixels = NUM_PIXELS; // Use actual number of pixels in grid
    
    // Create expanding/contracting effect
    uint8_t currentPixel = patternStep % (totalPixels * 2); // *2 for expand + contract cycle
    
    // Determine if we're expanding or contracting
    bool isExpanding = currentPixel < totalPixels;
    
    // For expanding: light pixels 0, 1, 2, 3, ...
    // For contracting: light pixels totalPixels-1, totalPixels-2, totalPixels-3, ..., 0
    uint8_t pixelsToLight;
    if (isExpanding) {
      pixelsToLight = currentPixel;
    } else {
      // Contracting phase - reverse the order
      pixelsToLight = totalPixels - 1 - (currentPixel - totalPixels);
    }
    
    // Debug output every 50 frames
    if (patternStep % 50 == 0) {
      Serial.printf("Spiral: step=%d, currentPixel=%d, isExpanding=%s, pixelsToLight=%d, totalPixels=%d\n", 
                    patternStep, currentPixel, isExpanding ? "true" : "false", pixelsToLight, totalPixels);
    }
    
    // Create a stable spiral sequence (only calculate once)
    static uint16_t spiralSequence[NUM_PIXELS];
    static bool sequenceInitialized = false;
    
    if (!sequenceInitialized) {
      // Generate the spiral sequence once, properly ordered from center outward
      uint8_t sequenceIndex = 0;
      
      // Start from center and spiral outward
      for (int layer = 0; layer <= max(GRID_WIDTH, GRID_HEIGHT) && sequenceIndex < NUM_PIXELS; layer++) {
        // Top row of current layer
        for (int col = centerCol - layer; col <= centerCol + layer && sequenceIndex < NUM_PIXELS; col++) {
          if (col >= 0 && col < GRID_WIDTH) {
            uint8_t row = centerRow - layer;
            if (row >= 0 && row < GRID_HEIGHT) {
              uint16_t pixelIndex = getPixelIndex(col, row);
              if (pixelIndex < NUM_PIXELS) {
                spiralSequence[sequenceIndex++] = pixelIndex;
              }
            }
          }
        }
        
        // Right column of current layer
        for (int row = centerRow - layer + 1; row <= centerRow + layer && sequenceIndex < NUM_PIXELS; row++) {
          if (row >= 0 && row < GRID_HEIGHT) {
            uint8_t col = centerCol + layer;
            if (col >= 0 && col < GRID_WIDTH) {
              uint16_t pixelIndex = getPixelIndex(col, row);
              if (pixelIndex < NUM_PIXELS) {
                spiralSequence[sequenceIndex++] = pixelIndex;
              }
            }
          }
        }
        
        // Bottom row of current layer
        for (int col = centerCol + layer - 1; col >= centerCol - layer && sequenceIndex < NUM_PIXELS; col--) {
          if (col >= 0 && col < GRID_WIDTH) {
            uint8_t row = centerRow + layer;
            if (row >= 0 && row < GRID_HEIGHT) {
              uint16_t pixelIndex = getPixelIndex(col, row);
              if (pixelIndex < NUM_PIXELS) {
                spiralSequence[sequenceIndex++] = pixelIndex;
              }
            }
          }
        }
        
        // Left column of current layer
        for (int row = centerRow + layer - 1; row >= centerRow - layer + 1 && sequenceIndex < NUM_PIXELS; row--) {
          if (row >= 0 && row < GRID_HEIGHT) {
            uint8_t col = centerCol - layer;
            if (col >= 0 && col < GRID_WIDTH) {
              uint16_t pixelIndex = getPixelIndex(col, row);
              if (pixelIndex < NUM_PIXELS) {
                spiralSequence[sequenceIndex++] = pixelIndex;
              }
            }
          }
        }
      }
      sequenceInitialized = true;
    }
    
    // Clear all pixels first
    for (int i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, 0);
    }
    
    // Light pixels according to the stable spiral sequence
    for (int i = 0; i < pixelsToLight && i < NUM_PIXELS; i++) {
      uint16_t pixelIndex = spiralSequence[i];
      if (pixelIndex < NUM_PIXELS) {
        // Use the current static color for the spiral
        pixels.setPixelColor(pixelIndex, staticColor);
      }
    }
    
    pixels.show();
    patternStep++;
  }
}

// Function to create pulse effect
void pulseEffect() {
    static uint16_t baseHue = 0;
    static bool initialized = false;
    
    if (!initialized) {
        initialized = true;
    }
    
    // Clear all pixels first
    pixels.clear();
    
    // Draw 8 static rings at different distances from center
    for (int col = 0; col < GRID_WIDTH; col++) {
        for (int row = 0; row < GRID_HEIGHT; row++) {
            // Calculate distance from center
            float dx = col - (GRID_WIDTH - 1) / 2.0;
            float dy = row - (GRID_HEIGHT - 1) / 2.0;
            float distance = sqrt(dx * dx + dy * dy);
            
            // Check if this pixel should be lit by any of the 20 rings
            for (int ring = 0; ring < 20; ring++) {
                float ringRadius = ring * 1.5; // Fixed ring positions, 1.5 pixels apart
                
                if (abs(distance - ringRadius) < 0.2) {
                    // Each ring has a hue offset by 1/20 of the full range
                    // Reverse the order so rainbow goes from center outward
                    uint16_t ringHue = (baseHue + ((19 - ring) * 3277)) % 65536; // 65536 / 20 = 3277
                    uint32_t color = pixels.ColorHSV(ringHue, 255, 255);
                    
                    // Get the pixel index using the proper grid mapping
                    uint16_t pixelIndex = getPixelIndex(col, row);
                    pixels.setPixelColor(pixelIndex, color);
                    break; // Only light by one ring
                }
            }
        }
    }
    
    // Increment base hue for all rings (creates the cycling effect)
    baseHue = (baseHue + 300) % 65536; // Adjust speed by changing this value
    
    pixels.show();
}

// Helper function to get pattern name
String getPatternName(uint8_t pattern) {
  switch(pattern) {
    case 0: return "Rainbow";
    case 1: return "Static";
    case 2: return "Wave";
    case 3: return "Fire";
    case 4: return "Matrix";
    case 5: return "Spiral";
    case 6: return "Pulse";
    default: return "Unknown";
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Handle web server and WebSocket requests (AP mode)
  server.handleClient();
  webSocket.loop();
  
  // Status update every second
  static unsigned long lastStatusUpdate = 0;
  if (currentMillis - lastStatusUpdate >= 1000) {
    lastStatusUpdate = currentMillis;
    
    Serial.printf("Mode: %s, Free Heap: %d bytes\n", 
                  getPatternName(currentPattern),
                  ESP.getFreeHeap());
  }
  
  // Non-blocking pattern effects
  if (currentMillis - previousPatternMillis >= (currentPattern == 6 ? 20 : patternInterval)) { // Even faster updates for pulse
    previousPatternMillis = currentMillis;
    
    switch(currentPattern) {
      case 0: // Rainbow
        rainbowCycle();
        break;
      case 1: // Static
        setStaticColor();
        break;
      case 2: // Wave
        waveEffect();
        break;
      case 3: // Fire
        fireEffect();
        break;
      case 4: // Matrix
        matrixEffect();
        break;
      case 5: // Spiral
        spiralEffect();
        break;
      case 6: // Pulse
        pulseEffect();
        break;
    }
  }
  
  // Auto-cycle patterns if enabled
  if (autoCycleEnabled && currentMillis - lastAutoCycleMillis >= autoCycleInterval) {
    lastAutoCycleMillis = currentMillis;
    currentPattern = (currentPattern + 1) % 7; // Cycle to next pattern
    
    Serial.print("Auto-cycled to pattern: ");
    Serial.println(getPatternName(currentPattern));
    
    // Reset pattern step for new pattern
    patternStep = 0;
    waveOffset = 0;
    
    // Broadcast status update
    broadcastStatus();
  }
  
  // Periodic status broadcast to keep all clients synchronized
  // This variable was removed, so it will cause a compilation error.
  // Assuming it should be re-added or handled differently.
  // For now, commenting out to avoid compilation errors.
  // if (WiFi.status() == WL_CONNECTED && currentMillis - previousBroadcastMillis >= broadcastInterval) {
  //   previousBroadcastMillis = currentMillis;
  //   broadcastStatus();
  // }
  
  // Check button state (toggle between rainbow and static mode)
  static bool buttonPressed = false;
  static unsigned long buttonPressTime = 0;
  
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
  }
  
  if (digitalRead(BUTTON_PIN) == HIGH && buttonPressed) {
    if (millis() - buttonPressTime < 50) { // Debounce
      // Short press - cycle through patterns
      currentPattern = (currentPattern + 1) % 7; // 7 patterns total
      
      // Print the actual mode name
      Serial.print("Pattern changed to: ");
      Serial.println(getPatternName(currentPattern));
      
      // Reset pattern step for new pattern
      patternStep = 0;
      waveOffset = 0;
    } else {
      // Long press - toggle brightness
      currentBrightness = (currentBrightness == 64) ? 128 : 64; // Toggle between 25% and 50%
      pixels.setBrightness(currentBrightness);
      Serial.printf("Brightness changed to: %d%%\n", (currentBrightness * 100) / 255);
    }
    buttonPressed = false;
  }
  
  delay(10); // Small delay to prevent watchdog issues
}
