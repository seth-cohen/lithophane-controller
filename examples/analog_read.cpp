/**
 * Analog Read Example for Seeed XIAO ESP32C3
 * 
 * This example reads analog values from pins A0-A3 and displays them
 * in the serial monitor. Also demonstrates ADC calibration.
 */

#include <Arduino.h>

// Analog pin definitions
#define A0_PIN  0
#define A1_PIN  1
#define A2_PIN  2
#define A3_PIN  3

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("XIAO ESP32C3 Analog Read Example");
    Serial.println("================================");
    
    // Set ADC resolution (ESP32C3 supports 12-bit ADC)
    analogReadResolution(12); // 0-4095 range
    
    // Set ADC attenuation for all pins
    // ADC_11db provides full 0-3.3V range (with some non-linearity)
    analogSetAttenuation(ADC_11db);
    
    Serial.println("Reading analog values from A0-A3...");
    Serial.println("Time(ms) | A0   | A1   | A2   | A3   | A0(V) | A1(V) | A2(V) | A3(V)");
    Serial.println("---------|------|------|------|------|-------|-------|-------|-------");
}

void loop() {
    // Read raw ADC values (0-4095 for 12-bit)
    int a0_raw = analogRead(A0_PIN);
    int a1_raw = analogRead(A1_PIN);
    int a2_raw = analogRead(A2_PIN);
    int a3_raw = analogRead(A3_PIN);
    
    // Convert to voltage (assuming 3.3V reference)
    float a0_voltage = (a0_raw * 3.3) / 4095.0;
    float a1_voltage = (a1_raw * 3.3) / 4095.0;
    float a2_voltage = (a2_raw * 3.3) / 4095.0;
    float a3_voltage = (a3_raw * 3.3) / 4095.0;
    
    // Print formatted output
    Serial.printf("%8lu | %4d | %4d | %4d | %4d | %5.2f | %5.2f | %5.2f | %5.2f\n",
                  millis(),
                  a0_raw, a1_raw, a2_raw, a3_raw,
                  a0_voltage, a1_voltage, a2_voltage, a3_voltage);
    
    delay(1000); // Read every second
}
