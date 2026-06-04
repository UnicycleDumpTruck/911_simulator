#include <Arduino.h>
#include <radio.h>

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

struct EvenDataPacket
{
    uint32_t counter;
    float batteryVoltage;
    uint8_t cubeID;
    uint8_t side;
} eventData;

uint8_t eventBuffer[sizeof(eventData)];
uint8_t from;
uint8_t len = sizeof(eventData);
int16_t packetnum = 0; // packet counter, we increment per xmission
uint32_t localCounter = 0;

void sendGoEvent(uint8_t s)
{
    eventData.side = s;
    eventData.cubeID = MY_ADDRESS;
    eventData.batteryVoltage = 0;
    eventData.counter++;
    Serial.print("About to send transmission number: ");
    Serial.println(eventData.counter);
    digitalWrite(RFM69_CS, LOW);
    delay(10);
    bool t_result = rf69.send((uint8_t *)&eventData, sizeof(eventData));
    rf69.waitPacketSent();
    if (t_result)
    {
        Serial.println("Send true");
    }
    else
    {
        Serial.println("Send false");
    }
    // delay(50);
    digitalWrite(RFM69_CS, HIGH);
}

void radioSetup()
{
    pinMode(RFM69_CS, OUTPUT); // This line caused the audio shield to not be found?
    // pinMode(RFM69_EN, OUTPUT);
    pinMode(RFM69_RST, OUTPUT);

    digitalWrite(RFM69_CS, LOW);
    //digitalWrite(RFM69_EN, HIGH);
    digitalWrite(RFM69_RST, LOW);
    delay(50);
    // manual reset
    digitalWrite(RFM69_RST, HIGH);
    delay(50);
    digitalWrite(RFM69_RST, LOW);
    delay(50);

    if (!rf69_manager.init())
    {
        delay(2000);
        Serial.println("RFM69 radio init failed");
        //while (1);
    }
    Serial.println("RFM69 radio init OK!");
    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!rf69.setFrequency(RF69_FREQ))
    {
        Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    rf69.setTxPower(20, true); // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same as the one in the server
    uint8_t key[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                     0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    rf69.setEncryptionKey(key);

    eventData.counter = 0;

    sendGoEvent(0);
    digitalWrite(RFM69_CS, HIGH);
    Serial.println("Radio Setup Complete");
}