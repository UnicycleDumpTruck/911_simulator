// Jeff's reusable code for Adafruit's VS1053 Audio Featherwing
// Remember not to send RFM69 radio messages during audio plays

#ifndef audio_h
#define audio_h

#include <Arduino.h>
#include <pinout.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

void vsAudioSetup();
void startAudio();
void stopAudio();
#endif