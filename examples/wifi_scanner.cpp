/**
 * WiFi Scanner Example for Seeed XIAO ESP32C3
 * 
 * This example scans for available WiFi networks and displays them
 * in the serial monitor.
 */

#include <Arduino.h>
#include <WiFi.h>

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("XIAO ESP32C3 WiFi Scanner");
    Serial.println("========================");
    
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    Serial.println("Setup done");
}

void loop() {
    Serial.println("Scanning for WiFi networks...");
    
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        Serial.printf("%d networks found:\n", n);
        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        Serial.println("---|----------------------------------|------|----|-----------");
        
        for (int i = 0; i < n; ++i) {
            Serial.printf("%2d", i + 1);
            Serial.print(" | ");
            Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            Serial.print(" | ");
            Serial.printf("%4d", WiFi.RSSI(i));
            Serial.print(" | ");
            Serial.printf("%2d", WiFi.channel(i));
            Serial.print(" | ");
            
            switch (WiFi.encryptionType(i)) {
                case WIFI_AUTH_OPEN:
                    Serial.print("Open");
                    break;
                case WIFI_AUTH_WEP:
                    Serial.print("WEP");
                    break;
                case WIFI_AUTH_WPA_PSK:
                    Serial.print("WPA");
                    break;
                case WIFI_AUTH_WPA2_PSK:
                    Serial.print("WPA2");
                    break;
                case WIFI_AUTH_WPA_WPA2_PSK:
                    Serial.print("WPA+WPA2");
                    break;
                case WIFI_AUTH_WPA2_ENTERPRISE:
                    Serial.print("WPA2-EAP");
                    break;
                case WIFI_AUTH_WPA3_PSK:
                    Serial.print("WPA3");
                    break;
                default:
                    Serial.print("Unknown");
            }
            Serial.println();
        }
    }
    Serial.println("");
    
    // Wait a bit before scanning again
    delay(5000);
}
