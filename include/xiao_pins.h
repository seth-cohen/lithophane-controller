/**
 * Pin definitions for Seeed XIAO ESP32C3
 * 
 * This header file contains all pin definitions and constants
 * for the XIAO ESP32C3 development board.
 * 
 * Note: The XIAO ESP32C3 does NOT have a built-in LED
 */

#ifndef XIAO_PINS_H
#define XIAO_PINS_H

// Built-in components
#define BUTTON_PIN      9    // Built-in button (BOOT button)
#define USER_BUTTON     9    // Alias for BUTTON_PIN

// Analog pins (12-bit ADC, 0-4095 range)
#define A0_PIN          0    // A0/D0
#define A1_PIN          1    // A1/D1  
#define A2_PIN          2    // A2/D2
#define A3_PIN          3    // A3/D3

// Digital pins
#define D0_PIN          0    // D0/A0
#define D1_PIN          1    // D1/A1
#define D2_PIN          2    // D2/A2
#define D3_PIN          3    // D3/A3
#define D4_PIN          4    // D4
#define D5_PIN          5    // D5
#define D6_PIN          6    // D6/SDA
#define D7_PIN          7    // D7/SCL
#define D8_PIN          8    // D8
#define D9_PIN          9    // D9/BUTTON
#define D10_PIN         10   // D10

// SPI interface pins
#define SPI_MOSI        10   // D10/MOSI
#define SPI_MISO        9    // D9/MISO  
#define SPI_SCK         8    // D8/SCK
#define SPI_SS          7    // D7/SS (Chip Select)

// I2C interface pins
#define I2C_SDA         6    // D6/SDA
#define I2C_SCL         7    // D7/SCL

// UART pins (Hardware Serial)
#define UART_TX         21   // TX pin
#define UART_RX         20   // RX pin

// Alternative names for common interfaces
#define WIRE_SDA        I2C_SDA
#define WIRE_SCL        I2C_SCL
#define SS              SPI_SS
#define MOSI            SPI_MOSI
#define MISO            SPI_MISO
#define SCK             SPI_SCK

// ADC configuration constants
#define ADC_RESOLUTION  12              // 12-bit ADC
#define ADC_MAX_VALUE   4095            // Maximum ADC reading
#define ADC_VREF        3.3             // Reference voltage

// Timing constants
#define DEBOUNCE_DELAY  50              // Button debounce delay (ms)

// Wake-up pin for deep sleep
#define WAKEUP_PIN      BUTTON_PIN      // Use button for wake-up

// Helper macros
#define ANALOG_TO_VOLTAGE(reading) ((reading * ADC_VREF) / ADC_MAX_VALUE)
#define VOLTAGE_TO_ANALOG(voltage) ((voltage * ADC_MAX_VALUE) / ADC_VREF)

#endif // XIAO_PINS_H
