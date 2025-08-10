/**
 * I2C Scanner Example for Seeed XIAO ESP32C3
 * 
 * This example scans for I2C devices on the default SDA/SCL pins
 * and displays their addresses.
 */

#include <Arduino.h>
#include <Wire.h>

// I2C pins for XIAO ESP32C3
#define I2C_SDA 6
#define I2C_SCL 7

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("XIAO ESP32C3 I2C Scanner");
    Serial.println("========================");
    
    // Initialize I2C with custom pins
    Wire.begin(I2C_SDA, I2C_SCL);
    
    Serial.printf("I2C initialized with SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
    Serial.println("Scanning for I2C devices...");
}

void loop() {
    byte error, address;
    int nDevices = 0;
    
    Serial.println("Scanning I2C bus...");
    Serial.println("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
    Serial.print("00:          ");
    
    for (address = 3; address < 127; address++) {
        if (address % 16 == 0) {
            Serial.printf("\n%02x:", address);
        }
        
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf(" %02x", address);
            nDevices++;
        } else if (error == 4) {
            Serial.print(" XX"); // Unknown error
        } else {
            Serial.print(" --"); // No device
        }
        
        delay(10); // Small delay between attempts
    }
    
    Serial.println();
    Serial.println();
    
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
    } else {
        Serial.printf("Found %d I2C device(s)\n", nDevices);
    }
    
    Serial.println("Scan completed. Waiting 5 seconds before next scan...");
    Serial.println();
    
    delay(5000); // Wait 5 seconds before next scan
}
