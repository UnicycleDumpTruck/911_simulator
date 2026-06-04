#ifndef PINOUT_H
#define PINOUT_H

#include <Arduino.h>

// Radio Pins
#define RFM69_CS A4
#define RFM69_INT A3
#define RFM69_RST 11

// Audio Featherwing Pins
#define SHIELD_RESET -1 // VS1053 reset pin (unused!)
#define SHIELD_CS 6     // VS1053 chip select pin (output)
#define SHIELD_DCS 10   // VS1053 Data/command select pin (output)
#define CARDCS 5        // Card chip select pin
#define DREQ 9          // VS1053 Data request, ideally an Interrupt pin

// Button Pins
#define BOUNCE_PIN A0
#define LED_PIN 13

#endif
