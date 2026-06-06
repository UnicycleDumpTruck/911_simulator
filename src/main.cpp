#include <Arduino.h>
#include <Adafruit_SleepyDog.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"
 


// Project Includes
#include "Version.h"
#include <pinout.h>
#include <audio.h>
#include <radio.h>

// Button Header
// Include the Bounce2 library found here :
// https://github.com/thomasfredericks/Bounce2
#include <Bounce2.h>

// INSTANTIATE A Bounce OBJECT
Bounce bounce = Bounce();

// SET A VARIABLE TO STORE THE LED STATE
int ledState = LOW;

// Capacitive Touch Variables
#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// Create two MPR121 objects
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();
 
// Track touches for each board
uint16_t lasttouched1 = 0;
uint16_t currtouched1 = 0;
 
uint16_t lasttouched2 = 0;
uint16_t currtouched2 = 0;


// ███████╗███████╗████████╗██╗   ██╗██████╗
// ██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
// ███████╗█████╗     ██║   ██║   ██║██████╔╝
// ╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
// ███████║███████╗   ██║   ╚██████╔╝██║
// ╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.printf("\nProject version v%s, built %s\n", VERSION, BUILD_TIMESTAMP);
  Serial.println("Setup function commencing...");
  vsAudioSetup();
  delay(100);
  radioSetup();

  // BOUNCE SETUP

  // SELECT ONE OF THE FOLLOWING :
  // 1) IF YOUR INPUT HAS AN INTERNAL PULL-UP
  bounce.attach(BOUNCE_PIN, INPUT_PULLUP); // USE INTERNAL PULL-UP
  // 2) IF YOUR INPUT USES AN EXTERNAL PULL-UP
  //bounce.attach( BOUNCE_PIN, INPUT ); // USE EXTERNAL PULL-UP

  // DEBOUNCE INTERVAL IN MILLISECONDS
  bounce.interval(5); // interval in ms

  // LED SETUP
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, ledState);

  // Initialize first capacitive touch board at 0x5A
  if (!cap1.begin(0x5A)) {
    Serial.println("MPR121 #1 not found at 0x5A, check wiring!");
    while (1);
  }
  Serial.println("MPR121 #1 connected!");
 
  // Initialize second capacitive board at 0x5B
  if (!cap2.begin(0x5B)) {
    Serial.println("MPR121 #2 not found at 0x5B, check wiring!");
    while (1);
  }
  Serial.println("MPR121 #2 connected!");
 
  // Optional: Auto-configure both capacitive touch boards
  cap1.setAutoconfig(true);
  cap2.setAutoconfig(true);
 

  Watchdog.enable(4000);
  Serial.println("Setup Complete");
}

// ██╗      ██████╗  ██████╗ ██████╗
// ██║     ██╔═══██╗██╔═══██╗██╔══██╗
// ██║     ██║   ██║██║   ██║██████╔╝
// ██║     ██║   ██║██║   ██║██╔═══╝
// ███████╗╚██████╔╝╚██████╔╝██║
// ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝

void loop()
{
  // Update the Bounce instance (YOU MUST DO THIS EVERY LOOP)
  bounce.update();

  // <Bounce>.changed() RETURNS true IF THE STATE CHANGED (FROM HIGH TO LOW OR LOW TO HIGH)
  if (bounce.changed())
  {
    // THE STATE OF THE INPUT CHANGED
    // GET THE STATE
    int deboucedInput = bounce.read();
    // IF THE CHANGED VALUE IS LOW
    if (deboucedInput == LOW)
    {
      ledState = !ledState;            // SET ledState TO THE OPPOSITE OF ledState
      digitalWrite(LED_PIN, ledState); // WRITE THE NEW ledState
      startAudio();
      delay(1000);
      stopAudio();
      sendGoEvent(1); // Does not work inside VS1053 audio startPlayingFile!
    }
  }

  // Read and print capacitive touch input boards
    // Read touches from board 1
  currtouched1 = cap1.touched();
  for (uint8_t i = 0; i < 6; i++) { // Only first 6 sensors are connected
    if ((currtouched1 & _BV(i)) && !(lasttouched1 & _BV(i))) {
      Serial.print("Board 1: "); Serial.print(i); Serial.println(" touched");
    }
    if (!(currtouched1 & _BV(i)) && (lasttouched1 & _BV(i))) {
      Serial.print("Board 1: "); Serial.print(i); Serial.println(" released");
    }
  }
  lasttouched1 = currtouched1;
 
  // Read touches from board 2
  currtouched2 = cap2.touched();
  for (uint8_t i = 0; i < 7; i++) { // Only first 7 sensors are connected
    if ((currtouched2 & _BV(i)) && !(lasttouched2 & _BV(i))) {
      Serial.print("Board 2: "); Serial.print(i); Serial.println(" touched");
    }
    if (!(currtouched2 & _BV(i)) && (lasttouched2 & _BV(i))) {
      Serial.print("Board 2: "); Serial.print(i); Serial.println(" released");
    }
  }
  lasttouched2 = currtouched2;

  Watchdog.reset();
}