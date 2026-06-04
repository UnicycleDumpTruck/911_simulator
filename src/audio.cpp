#include <Arduino.h>
#include <audio.h>

Adafruit_VS1053_FilePlayer musicPlayer =
    Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

void vsAudioSetup()
{
    pinMode(CARDCS, OUTPUT);
    pinMode(SHIELD_CS, OUTPUT);

    digitalWrite(SHIELD_CS, LOW);
    delay(50);

    if (!musicPlayer.begin())
    { // initialise the music player
        Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
        while (1)
            ;
    }
    digitalWrite(SHIELD_CS, HIGH);
    Serial.println(F("VS1053 found"));

    digitalWrite(CARDCS, LOW);
    if (!SD.begin(CARDCS))
    {
        Serial.println(F("SD failed, or not present"));
        while (1)
            ; // don't do anything more
    }
    digitalWrite(CARDCS, HIGH);

    // Set volume for left, right channels. lower numbers == louder volume!
    digitalWrite(SHIELD_CS, LOW);
    musicPlayer.setVolume(0, 0);
    musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT); // DREQ int
    digitalWrite(SHIELD_CS, HIGH);
}

void startAudio()
{
    Serial.println(F("Playing Sound"));
    digitalWrite(SHIELD_CS, LOW);
    musicPlayer.startPlayingFile("/track002.mp3");
    digitalWrite(SHIELD_CS, HIGH);
}

void stopAudio()
{
    digitalWrite(SHIELD_CS, LOW);
    musicPlayer.stopPlaying();
    digitalWrite(SHIELD_CS, HIGH);
}