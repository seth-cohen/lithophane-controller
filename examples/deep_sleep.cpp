/**
 * Deep Sleep Example for Seeed XIAO ESP32C3
 * 
 * This example demonstrates different wake-up methods from deep sleep:
 * - Timer wake-up
 * - GPIO wake-up (using the built-in button)
 */

#include <Arduino.h>

// Wake-up interval in microseconds (10 seconds)
#define SLEEP_TIME_US 10000000

// GPIO wake-up pin (built-in button)
#define WAKE_UP_PIN 9

// RTC memory to store boot count
RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Wakeup caused by external signal using RTC_IO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("Wakeup caused by external signal using RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            Serial.println("Wakeup caused by touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            Serial.println("Wakeup caused by ULP program");
            break;
        default:
            Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Give serial time to initialize
    
    // Increment boot count and print it every reboot
    ++bootCount;
    Serial.println("XIAO ESP32C3 Deep Sleep Example");
    Serial.println("===============================");
    Serial.printf("Boot number: %d\n", bootCount);
    
    // Print the wakeup reason for ESP32
    print_wakeup_reason();
    
    // Configure GPIO wake-up
    // The button is connected to GPIO9 and is pulled HIGH
    // We want to wake up when it goes LOW (button pressed)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_9, 0); // Wake when pin goes LOW
    
    // Configure timer wake-up
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
    Serial.printf("Setup ESP32 to sleep for %d seconds\n", SLEEP_TIME_US / 1000000);
    
    // Optional: Configure what peripherals to shut down/keep on during sleep
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    
    Serial.println("Going to sleep now...");
    Serial.println("Press the button (GPIO9) or wait 10 seconds to wake up");
    Serial.flush(); // Make sure the message is sent before sleeping
    
    // Enter deep sleep
    esp_deep_sleep_start();
    
    // This line will never be reached
    Serial.println("This will never be printed");
}

void loop() {
    // This is not going to be called because we're using deep sleep
    // The ESP32 will restart after waking up
}
