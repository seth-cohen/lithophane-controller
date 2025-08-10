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
#define NUM_PIXELS      6    // Number of NeoPixels
#define BRIGHTNESS      255  // Brightness (0-255) - Maximum brightness

// WiFi configuration - Update these with your network credentials
const char* ssid = "NETGEAR50";
const char* password = "deepcello816";

// Create NeoPixel objects
Adafruit_NeoPixel pixels(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Create web server object
WebServer server(80);

// Create WebSocket server object
WebSocketsServer webSocket = WebSocketsServer(81);

// Variables
bool rainbowMode = true;           // Start in rainbow mode
unsigned long previousRainbowMillis = 0;
unsigned long previousBroadcastMillis = 0;
const long rainbowInterval = 50;   // Rainbow update interval in milliseconds
const long broadcastInterval = 100; // Status broadcast interval in milliseconds (10fps for smooth rendering)
uint16_t rainbowHue = 0;           // Current hue for rainbow effect
uint32_t staticColor = 0xFF0000;   // Static color (default red)
uint8_t currentBrightness = 255;   // Current brightness (1-255)

// Command throttling variables
unsigned long lastBrightnessUpdate = 0;
unsigned long lastColorUpdate = 0;
const long commandThrottleMs = 50;

// Function to broadcast current status to all WebSocket clients
void broadcastStatus() {
  String mode = rainbowMode ? "Rainbow" : "Static";
  String colorHex = "";
  
  if (!rainbowMode) {
    // Static mode - use the stored static color
    uint8_t r = (staticColor >> 16) & 0xFF;
    uint8_t g = (staticColor >> 8) & 0xFF;
    uint8_t b = staticColor & 0xFF;
    colorHex = "#";
    if (r < 16) colorHex += "0";
    colorHex += String(r, HEX);
    if (g < 16) colorHex += "0";
    colorHex += String(g, HEX);
    if (b < 16) colorHex += "0";
    colorHex += String(b, HEX);
    colorHex.toUpperCase();
  } else {
    // Rainbow mode - get current rainbow color for display
    uint32_t currentRainbowColor = pixels.gamma32(pixels.ColorHSV(rainbowHue));
    uint8_t r = (currentRainbowColor >> 16) & 0xFF;
    uint8_t g = (currentRainbowColor >> 8) & 0xFF;
    uint8_t b = currentRainbowColor & 0xFF;
    colorHex = "#";
    if (r < 16) colorHex += "0";
    colorHex += String(r, HEX);
    if (g < 16) colorHex += "0";
    colorHex += String(g, HEX);
    if (b < 16) colorHex += "0";
    colorHex += String(b, HEX);
    colorHex.toUpperCase();
  }
  
  String json = "{\"type\":\"status\",\"mode\":\"" + mode + "\",\"color\":\"" + colorHex + "\",\"brightness\":" + String(currentBrightness) + "}";
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
        rainbowMode = true;
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
            rainbowMode = false;
            
            // Update pixels immediately
            for (int i = 0; i < NUM_PIXELS; i++) {
              pixels.setPixelColor(i, staticColor);
            }
            pixels.show();
            
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
              if (rainbowMode) {
                uint32_t color = pixels.gamma32(pixels.ColorHSV(rainbowHue));
                for (int i = 0; i < NUM_PIXELS; i++) {
                  pixels.setPixelColor(i, color);
                }
              } else {
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
      break;
    }
    
    default:
      break;
  }
}

// Function to setup web server routes
void setupWebServer() {
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
    rainbowMode = true;
    server.send(200, "text/plain", "Rainbow mode activated");
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
      rainbowMode = false;
      
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
        if (rainbowMode) {
          // Apply current rainbow color with new brightness
          uint32_t color = pixels.gamma32(pixels.ColorHSV(rainbowHue));
          for (int i = 0; i < NUM_PIXELS; i++) {
            pixels.setPixelColor(i, color);
          }
        } else {
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
    String mode = rainbowMode ? "Rainbow" : "Static";
    String colorHex = "";
    if (!rainbowMode) {
      uint8_t r = (staticColor >> 16) & 0xFF;
      uint8_t g = (staticColor >> 8) & 0xFF;
      uint8_t b = staticColor & 0xFF;
      colorHex = "#" + String(r, HEX) + String(g, HEX) + String(b, HEX);
      colorHex.toUpperCase();
    }
    String json = "{\"mode\":\"" + mode + "\",\"color\":\"" + colorHex + "\",\"brightness\":" + String(currentBrightness) + "}";
    server.send(200, "application/json", json);
  });
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for serial port to connect
  }
  
  Serial.println("Seeed XIAO ESP32C3 Starting...");
  
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
  
  // Initialize WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  // Wait for connection with timeout
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Setup web server routes
    setupWebServer();
    server.begin();
    Serial.println("Web server started!");
    
    // Setup and start WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started on port 81!");
  } else {
    Serial.println("\nFailed to connect to WiFi. Running in standalone mode.");
  }
  
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
  if (rainbowMode) {
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
  if (!rainbowMode) {
    for (int i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, staticColor);
    }
    pixels.show();
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Handle web server and WebSocket requests
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
    webSocket.loop();
  }
  
  // Status update every second
  static unsigned long lastStatusUpdate = 0;
  if (currentMillis - lastStatusUpdate >= 1000) {
    lastStatusUpdate = currentMillis;
    
    Serial.printf("Mode: %s, Free Heap: %d bytes\n", 
                  rainbowMode ? "Rainbow" : "Static",
                  ESP.getFreeHeap());
  }
  
  // Non-blocking rainbow effect (only if in rainbow mode)
  if (rainbowMode && currentMillis - previousRainbowMillis >= rainbowInterval) {
    previousRainbowMillis = currentMillis;
    rainbowCycle();
  }
  
  // Periodic status broadcast to keep all clients synchronized
  if (WiFi.status() == WL_CONNECTED && currentMillis - previousBroadcastMillis >= broadcastInterval) {
    previousBroadcastMillis = currentMillis;
    broadcastStatus();
  }
  
  // Check button state (toggle between rainbow and static mode)
  // Note: BUTTON_PIN is connected to the BOOT button on the XIAO ESP32C3
  if (digitalRead(BUTTON_PIN) == LOW) {
    rainbowMode = !rainbowMode;
    Serial.printf("Button pressed! Mode switched to: %s\n", rainbowMode ? "Rainbow" : "Static");
    
    if (!rainbowMode) {
      setStaticColor(); // Apply static color immediately
    }
    
    delay(200); // Simple debounce
  }
  
  delay(10); // Small delay to prevent watchdog issues
}
