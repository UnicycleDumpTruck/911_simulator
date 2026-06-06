// Simulator to teach kids how to dial 911 and what to expect when they do. 

#include <Wire.h> // For i2c communications with the MCP23017 i/o expanders and the HT16K33 LED matrix driver
#include <SPI.h> // For communication with the Rugged Audio Shield
#include <RAS.h> // Rugged Audio Shield library
#include "Adafruit_LEDBackpack.h" // For the HT16K33
#include "Adafruit_GFX.h" // For the HT16K33
#include <avr/wdt.h> // Watchdog Timer

RAS RAS;
Adafruit_LEDBackpack matrix = Adafruit_LEDBackpack();

// HT16K33 connections C1-C7. C0 not used.
#define ASEG 5
#define BSEG 6
#define CSEG 3
#define DSEG 1
#define ESEG 2
#define FSEG 4
#define GSEG 7

// MCP23017 registers (everything except direction defaults to 0)
#define IODIRA   0x00   // IO direction  (0 = output, 1 = input (Default))
#define IODIRB   0x01
#define IOPOLA   0x02   // IO polarity   (0 = normal, 1 = inverse)
#define IOPOLB   0x03
#define GPINTENA 0x04   // Interrupt on change (0 = disable, 1 = enable)
#define GPINTENB 0x05
#define DEFVALA  0x06   // Default comparison for interrupt on change (interrupts on opposite)
#define DEFVALB  0x07
#define INTCONA  0x08   // Interrupt control (0 = interrupt on change from previous, 1 = interrupt on change from DEFVAL)
#define INTCONB  0x09
#define IOCON    0x0A   // IO Configuration: bank/mirror/seqop/disslw/haen/odr/intpol/notimp
//#define IOCON 0x0B  // same as 0x0A
#define GPPUA    0x0C   // Pull-up resistor (0 = disabled, 1 = enabled)
#define GPPUB    0x0D
#define INFTFA   0x0E   // Interrupt flag (read only) : (0 = no interrupt, 1 = pin caused interrupt)
#define INFTFB   0x0F
#define INTCAPA  0x10   // Interrupt capture (read only) : value of GPIO at time of last interrupt
#define INTCAPB  0x11
#define GPIOA    0x12   // Port value. Write to change, read to obtain value
#define GPIOB    0x13
#define OLLATA   0x14   // Output latch. Write to latch output.
#define OLLATB   0x15


#define portLED 0x20  // MCP23017 for the keypad and script LEDs is on I2C port 0x26

volatile boolean keyPressed; // set when interrupt fires

//int led = 13;
unsigned long resetTimer = 0;
unsigned long blinkTimer = 0;
unsigned long soundTimer = 0;
unsigned long responseTimer = 0;
unsigned long hintTimer = 0;
int placeMarker = 0;
int keyBuffer = -1;

#define RESETTIME 33000
#define BLINKDURATION 500
#define FASTBLINKDURATION 100
#define HINTDURATION 2000

#define ONEKEY 7
#define TWOKEY 4
#define THREEKEY 11
#define FOURKEY 6
#define FIVEKEY 0
#define SIXKEY 10
#define SEVENKEY 5
#define EIGHTKEY 1
#define NINEKEY 9
#define ZEROKEY 2
#define POUNDKEY 16
#define STARKEY 17

#define ONELED 14
#define TWOLED 13
#define THREELED 8
#define FOURLED 15
#define FIVELED 9
#define SIXLED 7
#define SEVENLED 12
#define EIGHTLED 10
#define NINELED 6
#define ZEROLED 11
#define POUNDLED 6
#define STARLED 7

#define AKEY 15
#define BKEY 8
#define CKEY 14
#define DKEY 13
#define EKEY 12
#define FKEY 3

#define ALED 0
#define BLED 1
#define CLED 2
#define DLED 3
#define ELED 4
#define FLED 5

// Placemarkers in main switch statement
#define DIALNINE 1
#define DIALFONE 2
#define DIALSONE 3
#define RINGING 4
#define SCRIPTA 5
#define RESPONDINGA 6
#define HINTB 7
#define SCRIPTB 8
#define RESPONDINGB 9
#define HINTC 10
#define SCRIPTC 11
#define RESPONDINGC 12
#define HINTD 13
#define SCRIPTD 14
#define RESPONDINGD 15
#define HINTE 16
#define SCRIPTE 17
#define RESPONDINGE 18
#define HINTF 19
#define SCRIPTF 20

// Lengths of sound files
#define TONELENGTH 200
#define RINGLENGTH 2100
#define SCRIPTALENGTH 2600 //2350
#define SCRIPTBLENGTH 920 //700
#define SCRIPTCLENGTH 1110 //940
#define SCRIPTDLENGTH 960 //900
#define SCRIPTELENGTH 2610 //2700
#define SCRIPTFLENGTH 1930 //1600

// Length to wait for response to script
#define RESPONSEWAIT 2000

void clearLED (void)
{
  matrix.clear();
  matrix.writeDisplay();
}

void numberToLED (uint8_t number, uint8_t led)
{
  switch (number) {
    case 10:
      matrix.clear();
      break;    
    case 0:
      matrix.displaybuffer[DSEG] |= _BV(led);
      matrix.displaybuffer[BSEG] |= _BV(led);
      matrix.displaybuffer[ASEG] |= _BV(led);
      matrix.displaybuffer[FSEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      matrix.displaybuffer[ESEG] |= _BV(led);
      break;
    case 1:
      matrix.displaybuffer[BSEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      break;
    case 2:
      matrix.displaybuffer[ASEG] |= _BV(led);
      matrix.displaybuffer[BSEG] |= _BV(led);
      matrix.displaybuffer[GSEG] |= _BV(led);
      matrix.displaybuffer[ESEG] |= _BV(led);
      matrix.displaybuffer[DSEG] |= _BV(led);
      break;
    case 3:
      matrix.displaybuffer[ASEG] |= _BV(led);
      matrix.displaybuffer[BSEG] |= _BV(led);
      matrix.displaybuffer[GSEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      matrix.displaybuffer[DSEG] |= _BV(led);
      break;
    case 4:
      matrix.displaybuffer[FSEG] |= _BV(led);
      matrix.displaybuffer[GSEG] |= _BV(led);
      matrix.displaybuffer[BSEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      break;
    case 5:
      matrix.displaybuffer[ASEG] |= _BV(led);
      matrix.displaybuffer[FSEG] |= _BV(led);
      matrix.displaybuffer[GSEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      matrix.displaybuffer[DSEG] |= _BV(led);
      break;
    case 6:
      matrix.displaybuffer[ASEG] |= _BV(led);
      matrix.displaybuffer[FSEG] |= _BV(led);
      matrix.displaybuffer[GSEG] |= _BV(led);
      matrix.displaybuffer[ESEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      matrix.displaybuffer[DSEG] |= _BV(led);
      break;
    case 7:
      matrix.displaybuffer[ASEG] |= _BV(led);
      matrix.displaybuffer[BSEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      break;
    case 8:
      matrix.displaybuffer[ASEG] |= _BV(led);
      matrix.displaybuffer[BSEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      matrix.displaybuffer[DSEG] |= _BV(led);
      matrix.displaybuffer[ESEG] |= _BV(led);
      matrix.displaybuffer[FSEG] |= _BV(led);
      matrix.displaybuffer[GSEG] |= _BV(led);
      break;
    case 9:
      matrix.displaybuffer[ASEG] |= _BV(led);
      matrix.displaybuffer[BSEG] |= _BV(led);
      matrix.displaybuffer[CSEG] |= _BV(led);
      matrix.displaybuffer[DSEG] |= _BV(led);
      matrix.displaybuffer[FSEG] |= _BV(led);
      matrix.displaybuffer[GSEG] |= _BV(led);
      break;

    default:
      break;
  }
  matrix.writeDisplay();
  //Serial.println("written");
}

void ledWrite (uint8_t p, uint8_t d) {
  uint8_t gpio;
  uint8_t gpioaddr, olataddr;
  // only 16 bits!
  if (p > 15)
    return;
  if (p < 8) {
    olataddr = OLLATA;
    gpioaddr = GPIOA;
  } else {
    olataddr = OLLATB;
    gpioaddr = GPIOB;
    p -= 8;
  }
  // read the current GPIO output latches
  Wire.beginTransmission(portLED);
  Wire.write(olataddr);	
  Wire.endTransmission();
  Wire.requestFrom(portLED, 1);
   gpio = Wire.read();
  // set the pin and direction
  if (d == HIGH) {
    gpio |= 1 << p; 
  } else {
    gpio &= ~(1 << p);
  }
  // write the new GPIO
  Wire.beginTransmission(portLED);
  Wire.write(gpioaddr);
  Wire.write(gpio);	
  Wire.endTransmission();
}

void ledWriteTwo (uint8_t p, uint8_t d) {
  uint8_t gpio;
  uint8_t gpioaddr, olataddr;
  // only 16 bits!
  if (p > 15)
    return;
  if (p < 8) {
    olataddr = OLLATA;
    gpioaddr = GPIOA;
  } else {
    olataddr = OLLATB;
    gpioaddr = GPIOB;
    p -= 8;
  }
  // read the current GPIO output latches
  Wire.beginTransmission(portLEDTWO);
  Wire.write(olataddr);	
  Wire.endTransmission();
  Wire.requestFrom(portLEDTWO, 1);
   gpio = Wire.read();
  // set the pin and direction
  if (d == HIGH) {
    gpio |= 1 << p; 
  } else {
    gpio &= ~(1 << p);
  }
  // write the new GPIO
  Wire.beginTransmission(portLEDTWO);
  Wire.write(gpioaddr);
  Wire.write(gpio);	
  Wire.endTransmission();
}

uint8_t ledRead(uint8_t p) {
  uint8_t gpioaddr;

  // only 16 bits!
  if (p > 15)
    return 0;

  if (p < 8)
    gpioaddr = GPIOA;
  else {
    gpioaddr = GPIOB;
    p -= 8;
  }

  // read the current GPIO
  Wire.beginTransmission(0x26);
  Wire.write(gpioaddr);	
  Wire.endTransmission();
  
  Wire.requestFrom(0x26, 1);
  return (Wire.read() >> p) & 0x1;
}

uint8_t ledReadTwo(uint8_t p) {
  uint8_t gpioaddr;

  // only 16 bits!
  if (p > 15)
    return 0;

  if (p < 8)
    gpioaddr = GPIOA;
  else {
    gpioaddr = GPIOB;
    p -= 8;
  }

  // read the current GPIO
  Wire.beginTransmission(portLEDTWO);
  Wire.write(gpioaddr);	
  Wire.endTransmission();
  
  Wire.requestFrom(portLEDTWO, 1);
  return (Wire.read() >> p) & 0x1;
}


// set register "reg" on expander to "data"
// for example, IO direction
void expanderWriteBoth (const byte reg, const byte data ) 
{
  Wire.beginTransmission (port);
  Wire.write (reg);
  Wire.write (data);  // port A
  Wire.write (data);  // port B
  Wire.endTransmission ();
} // end of expanderWrite

void expanderWriteBothTwo (const byte reg, const byte data ) 
{
  Wire.beginTransmission (portTWO);
  Wire.write (reg);
  Wire.write (data);  // port A
  Wire.write (data);  // port B
  Wire.endTransmission ();
} // end of expanderWrite

void expanderWriteLED (const byte reg, const byte data ) 
{
  Wire.beginTransmission (portLED);
  Wire.write (reg);
  Wire.write (data);  // port A
  Wire.write (data);  // port B
  Wire.endTransmission ();
} // end of expanderWrite

void expanderWriteLEDTwo (const byte reg, const byte data ) 
{
  Wire.beginTransmission (portLEDTWO);
  Wire.write (reg);
  Wire.write (data);  // port A
  Wire.write (data);  // port B
  Wire.endTransmission ();
} // end of expanderWrite

void expanderWriteALED (const byte reg, const byte data ) 
{
  Wire.beginTransmission (portLED);
  Wire.write (reg);
  Wire.write (data);  // port A
  Wire.endTransmission ();
} // end of expanderWrite

void expanderWriteALEDTwo (const byte reg, const byte data ) 
{
  Wire.beginTransmission (portLEDTWO);
  Wire.write (reg);
  Wire.write (data);  // port A
  Wire.endTransmission ();
} // end of expanderWrite


// read a byte from the expander
unsigned int expanderRead (const byte reg) 
{
  Wire.beginTransmission (port);
  Wire.write (reg);
  Wire.endTransmission ();
  Wire.requestFrom (port, 1);
  return Wire.read();
} // end of expanderRead

// read a byte from the expander
unsigned int expanderReadTwo (const byte reg) 
{
  Wire.beginTransmission (portTWO);
  Wire.write (reg);
  Wire.endTransmission ();
  Wire.requestFrom (portTWO, 1);
  return Wire.read();
} // end of expanderRead


unsigned int expanderReadLED (const byte reg) 
{
  Wire.beginTransmission (portLED);
  Wire.write (reg);
  Wire.endTransmission ();
  Wire.requestFrom (portLED, 1);
  return Wire.read();
} // end of expanderRead

unsigned int expanderReadLEDTwo (const byte reg) 
{
  Wire.beginTransmission (portLEDTWO);
  Wire.write (reg);
  Wire.endTransmission ();
  Wire.requestFrom (portLEDTWO, 1);
  return Wire.read();
} // end of expanderRead


// interrupt service routine, called when pin D2 goes from 1 to 0
void keypress ()
{
  //digitalWrite (ISR_INDICATOR, HIGH);  // debugging
  keyPressed = true;   // set flag so main loop knows
  //Serial.println("keypress");
}  // end of keypress

void allLEDSoff ()
{
  expanderWriteLED (GPIOA, 0x00);
  expanderWriteLEDTwo (GPIOA, 0x00);
}

void lightKeypad ()
{
  ledWrite(ONELED,HIGH);
  ledWrite(TWOLED,HIGH);
  ledWrite(THREELED,HIGH);
  ledWrite(FOURLED,HIGH);
  ledWrite(FIVELED,HIGH);
  ledWrite(SIXLED,HIGH);
  ledWrite(SEVENLED,HIGH);
  ledWrite(EIGHTLED,HIGH);
  ledWrite(NINELED,HIGH);
  ledWrite(ZEROLED,HIGH);
  ledWriteTwo(POUNDLED,HIGH);
  ledWriteTwo(STARLED,HIGH);
}

void lightScript ()
{
  ledWrite(ALED,HIGH);
  ledWrite(BLED,HIGH);
  ledWrite(CLED,HIGH);
  ledWrite(DLED,HIGH);
  ledWrite(ELED,HIGH);
  ledWrite(FLED,HIGH);
}


void readScript (uint8_t b)
{
      switch (b) {
        case AKEY:
          RAS.Stop();
          RAS.PlayWAV("SCRIPTA.WAV");
          delay(SCRIPTALENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case BKEY:
          RAS.Stop();
          RAS.PlayWAV("SCRIPTB.WAV");
          delay(SCRIPTBLENGTH);
          RAS.Stop();
          break;
        case CKEY:
          RAS.Stop();
          RAS.PlayWAV("SCRIPTC.WAV");
          delay(SCRIPTCLENGTH);
          RAS.Stop();
          break;
        case DKEY:
          RAS.Stop();
          RAS.PlayWAV("SCRIPTD.WAV");
          delay(SCRIPTDLENGTH);
          RAS.Stop();
          break;
        case EKEY:
          RAS.Stop();
          RAS.PlayWAV("SCRIPTE.WAV");
          delay(SCRIPTELENGTH);
          RAS.Stop();
          break;
        case FKEY:
          RAS.Stop();
          RAS.PlayWAV("SCRIPTF.WAV");
          delay(SCRIPTFLENGTH);
          RAS.Stop();
          break;
        default:
          break;
      }
}

  
  
void displayNumber (uint8_t b)
{
      switch (b) {
        case ONEKEY:
          //Serial.print("1");
          RAS.Stop();
          RAS.PlayWAV("1.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case TWOKEY:
          //Serial.print("2");
          RAS.Stop();
          RAS.PlayWAV("2.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case THREEKEY:
          //Serial.print("3");
          RAS.Stop();
          RAS.PlayWAV("3.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case FOURKEY:
          //Serial.print("4");
          RAS.Stop();
          RAS.PlayWAV("4.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case FIVEKEY:
          //Serial.print("5");
          RAS.Stop();
          RAS.PlayWAV("5.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case SIXKEY:
          //Serial.print("6");
          RAS.Stop();
          RAS.PlayWAV("6.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case SEVENKEY:
          //Serial.print("7");
          RAS.Stop();
          RAS.PlayWAV("7.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case EIGHTKEY:
          //Serial.print("8");
          RAS.Stop();
          RAS.PlayWAV("8.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case NINEKEY: 
          //Serial.print("9");
          RAS.Stop();
          RAS.PlayWAV("9.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          RAS.WaitForIdle();
          break;
        case ZEROKEY:
          //Serial.print("0");
          RAS.Stop();
          RAS.PlayWAV("0.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case POUNDKEY:
          //Serial.print("#");
          RAS.Stop();
          RAS.PlayWAV("1.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;
        case STARKEY:
          //Serial.print("*");
          RAS.Stop();
          RAS.PlayWAV("3.WAV");
          delay(TONELENGTH);
          RAS.Stop();
          //RAS.WaitForIdle();
          break;

        default:
          break;
      }
}

void setupExpanders() {
  // expander configuration register
  expanderWriteBoth (IOCON, 0b01100000); // mirror interrupts, disable sequential mode 
  expanderWriteBothTwo (IOCON, 0b01100000); // mirror interrupts, disable sequential mode 
  // enable pull-up on switches
  expanderWriteBoth (GPPUA, 0xFF);   // pull-up resistor for switch - both ports
  expanderWriteBothTwo (GPPUA, 0xFF);   // pull-up resistor for switch - both ports
  // invert polarity
  expanderWriteBoth (IOPOLA, 0xFF);  // invert polarity of signal - both ports
  expanderWriteBothTwo (IOPOLA, 0xFF);  // invert polarity of signal - both ports
  // enable all interrupts
  expanderWriteBoth (GPINTENA, 0xFF); // enable interrupts - both ports
  expanderWriteBothTwo (GPINTENA, 0xFF); // enable interrupts - both ports
 
  // read from interrupt capture ports to clear them
  expanderRead (INTCAPA);
  expanderReadTwo (INTCAPA);
  expanderRead (INTCAPB);
  expanderReadTwo (INTCAPB);

}

void setup ()
{
  Wire.begin ();  // to communicate with i2c
  Serial.begin (9600);  // for debugging via serial terminal on computer
  setupExpanders();
/*  // expander configuration register
  expanderWriteBoth (IOCON, 0b01100000); // mirror interrupts, disable sequential mode 
  expanderWriteBothTwo (IOCON, 0b01100000); // mirror interrupts, disable sequential mode 
  // enable pull-up on switches
  expanderWriteBoth (GPPUA, 0xFF);   // pull-up resistor for switch - both ports
  expanderWriteBothTwo (GPPUA, 0xFF);   // pull-up resistor for switch - both ports
  // invert polarity
  expanderWriteBoth (IOPOLA, 0xFF);  // invert polarity of signal - both ports
  expanderWriteBothTwo (IOPOLA, 0xFF);  // invert polarity of signal - both ports
  // enable all interrupts
  expanderWriteBoth (GPINTENA, 0xFF); // enable interrupts - both ports
  expanderWriteBothTwo (GPINTENA, 0xFF); // enable interrupts - both ports
  // no interrupt yet*/
  keyPressed = false;
  // read from interrupt capture ports to clear them
  /*expanderRead (INTCAPA);
  expanderReadTwo (INTCAPA);
  expanderRead (INTCAPB);
  expanderReadTwo (INTCAPB);*/
  // pin 19 of MCP23017 is plugged into D2 of the Arduino which is interrupt 0
  attachInterrupt(0, keypress, FALLING);
  attachInterrupt(1, keypress, FALLING);
  
  expanderWriteLED (IODIRA, 0x00); // Set ports A and B to output
  expanderWriteLEDTwo (IODIRA, 0x00); // Set ports A and B to output
  
  resetTimer = millis(); // Timer that determines if the player has walked away.
  placeMarker = DIALNINE; // Start off waiting for the first nine
  lightKeypad();
  lightScript();
  //allLEDSoff();
  
  RAS.begin(); // Fire up the audio shield
  RAS.InitSD();
  delay(100);
  RAS.OutputEnable();

  matrix.begin(0x70);  // pass in the address of the HT16K33 for the 911 LEDs
  clearLED();
  
  wdt_enable(WDTO_8S); // Enable watchdog timer with eight second timeout.

}  // end of setup

// called from main loop when we know we had an interrupt
void handleKeypress ()
{
  resetTimer = millis();
  //Serial.println("keyhandled");
  unsigned int keyValue = 0;
  int secondEx = 0;
  
  delay (10);  // de-bounce before we re-enable interrupts
  
  keyPressed = false;  // ready for next time through the interrupt service routine
  
  // Read port values, as required. Note that this re-arms the interrupts.
  if (expanderRead (INFTFA))
    keyValue |= expanderRead (INTCAPA) << 8;    // read value at time of interrupt
  if (expanderRead (INFTFB))
    keyValue |= expanderRead (INTCAPB);        // port B is in low-order byte
  // Read port values, as required. Note that this re-arms the interrupts.
  if (expanderReadTwo (INFTFA)) {
    keyValue |= expanderReadTwo (INTCAPA) << 8;    // read value at time of interrupt
    secondEx = 1;
  }
  if (expanderReadTwo (INFTFB)) {
    keyValue |= expanderReadTwo (INTCAPB);      // port B is in low-order byte
    secondEx = 1;
  }

  
  // display which buttons were down at the time of the interrupt
  for (byte button = 0; button < 16; button++)
    {
    // this key down?
    if (keyValue & (1 << button))
      {
      if (secondEx) {
        keyBuffer = button + 16;
      } else {
        keyBuffer = button;
      }
      //Serial.println(keyBuffer);
      /*switch (button) { // For debugging purposes to determine which buttons are where
        case ONEKEY:
          ledWrite(ONELED,HIGH);
          break;
        case TWOKEY:
          ledWrite(TWOLED,HIGH);
          break;
        case THREEKEY:
          ledWrite(THREELED,HIGH);
          break;
        case FOURKEY:
          ledWrite(FOURLED,HIGH);
          break;
        case FIVEKEY:
          ledWrite(FIVELED,HIGH);
          break;
        case SIXKEY:
          ledWrite(SIXLED,HIGH);
          break;
        case SEVENKEY:
          ledWrite(SEVENLED,HIGH);
          break;
        case EIGHTKEY:
          ledWrite(EIGHTLED,HIGH);
          break;
        case NINEKEY: 
          ledWrite(NINELED,HIGH);
          break;
        case ZEROKEY:
          ledWrite(ZEROLED,HIGH);
          break;
        case AKEY:
          ledWrite(ALED,HIGH);
          break;
        case BKEY:
          ledWrite(BLED,HIGH);
          break;
        case CKEY:
          ledWrite(CLED,HIGH);
          break;
        case DKEY:
          ledWrite(DLED,HIGH);
          break;
        case EKEY:
          ledWrite(ELED,HIGH);
          break;
        case FKEY:
          ledWrite(FLED,HIGH);
          break;
        default:
          break;
        }*/
      }  // end of if this bit changed
    
    } // end of for each button
}  // end of handleKeypress


void loop() {
  wdt_reset(); // Pat the watchdog
  
  if (RESETTIME < (millis() - resetTimer)) { // Determining if kid left
    setupExpanders();
    placeMarker = DIALNINE;
    lightKeypad();
    lightScript();
    clearLED();
    resetTimer = millis();
    
    // IMPLEMENT RESET OF IO EXPANDERS //
  }
  
  switch (placeMarker) { // This switch statement handles the dialing and response script
    case DIALNINE: // Waiting for the nine to be dialed
      if (keyPressed) {
        handleKeypress ();
      }
      if (BLINKDURATION < (millis() - blinkTimer)) { // Blink the LED slowly
        blinkTimer = millis();
        ledWrite(NINELED, !ledRead(NINELED));
      }
      if (keyBuffer == NINEKEY) { // if nine was dialed
        lightKeypad();
        //allLEDSoff();
        placeMarker = DIALFONE;
        numberToLED(9,1);
        displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) { // if other key pressed
        displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case DIALFONE: // Waiting for the first one to be dialed
      if (keyPressed) {
        handleKeypress ();
      }
      if (BLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(ONELED, !ledRead(ONELED));
      }
      if (keyBuffer == ONEKEY) { // if one was dialed
        lightKeypad();
        //allLEDSoff();
        placeMarker = DIALSONE;
        clearLED();
        numberToLED(9,2);
        numberToLED(1,1);
        displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) { // if other key pressed
        displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;
  
    case DIALSONE: // Waiting for the second one to be dialed
      if (keyPressed) {
        handleKeypress ();
      }
      if (BLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(ONELED, !ledRead(ONELED));
      }
      if (keyBuffer == ONEKEY) { // if one was dialed
        allLEDSoff();
        lightScript();
        placeMarker = RINGING;
        clearLED();
        numberToLED(9,3);
        numberToLED(1,2);
        numberToLED(1,1);
        displayNumber(keyBuffer);
        keyBuffer = -1;
        // start playing ringback sound
        RAS.Stop(); // reduces popping at start of playback
        RAS.PlayWAV("RING.WAV"); 
        soundTimer = millis(); // Start timing the sound
      } else if (keyBuffer != -1) { // if other key pressed
        displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;
    
    case RINGING: // Ringing sound playing
      if (RINGLENGTH < (millis() - soundTimer)) { // if the sound is done playing...
        placeMarker = SCRIPTA;
        ledWrite(ALED, HIGH);
        //start playing ScriptA sound
        RAS.Stop();
        RAS.PlayWAV("SCRIPTA.WAV");
        soundTimer = millis();
      }
      break;
      
    case SCRIPTA: // "911 operator, what is your emergency?" playing
      //if (keyPressed) {
      //  handleKeypress ();
      //}
      if (FASTBLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(ALED, !ledRead(ALED));
      }
      if (SCRIPTALENGTH < (millis() - soundTimer)) {
        RAS.Stop();
        //allLEDSoff();
        placeMarker = RESPONDINGA;
        //keyPressed = false;
        responseTimer = millis();
        //displayNumber(keyBuffer);
        //keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        //readScript(keyBuffer);
        //keyBuffer = -1;
      }
      break;
      
    case RESPONDINGA:
      if (keyPressed) {
        handleKeypress ();
      }
      if (RESPONSEWAIT < (millis() - responseTimer)) {
        //placeMarker = SCRIPTB;
        placeMarker = HINTB;
        hintTimer = millis();
        //etc.
      }
      if (keyBuffer == BKEY) {
        //allLEDSoff();
        placeMarker = SCRIPTB;
        //displayNumber(keyBuffer);
        keyBuffer = -1;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTB.WAV");
        soundTimer = millis();
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;
    
    case HINTB:
      if (keyPressed) {
        handleKeypress ();
      }
      /*if (BLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(BLED, !ledRead(BLED));
      }*/
      if ((keyBuffer == BKEY) || (HINTDURATION < (millis() - hintTimer))) {
        RAS.Stop();
        //allLEDSoff();
        ledWrite(BLED, HIGH);
        placeMarker = SCRIPTB;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTB.WAV");
        soundTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;
      
    case SCRIPTB:
      if (keyPressed) {
        handleKeypress ();
      }
      if (FASTBLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(BLED, !ledRead(BLED));
      }
      if (SCRIPTBLENGTH < (millis() - soundTimer)) {
        RAS.Stop();
        //allLEDSoff();
        lightScript();
        placeMarker = RESPONDINGB;
        responseTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case RESPONDINGB:
      if (keyPressed) {
        handleKeypress ();
      }
      if (RESPONSEWAIT < (millis() - responseTimer)) {
        //placeMarker = SCRIPTC;
        placeMarker = HINTC;
        hintTimer = millis();
        //etc.
      }
      if (keyBuffer == CKEY) {
        //allLEDSoff();
        placeMarker = SCRIPTC;
        //displayNumber(keyBuffer);
        keyBuffer = -1;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTC.WAV");
        soundTimer = millis();
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case HINTC:
      if (keyPressed) {
        handleKeypress ();
      }
      /*if (BLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(CLED, !ledRead(CLED));
      }*/
      if ((keyBuffer == CKEY) || (HINTDURATION < (millis() - hintTimer))) {
        RAS.Stop();
        //allLEDSoff();
        ledWrite(CLED, HIGH);
        placeMarker = SCRIPTC;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTC.WAV");
        soundTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case SCRIPTC:
      if (keyPressed) {
        handleKeypress ();
      }
      if (FASTBLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(CLED, !ledRead(CLED));
      }
      if (SCRIPTCLENGTH < (millis() - soundTimer)) {
        RAS.Stop();
        //allLEDSoff();
        lightScript();
        placeMarker = RESPONDINGC;
        responseTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case RESPONDINGC:
      if (keyPressed) {
        handleKeypress ();
      }
      if (RESPONSEWAIT < (millis() - responseTimer)) {
        //placeMarker = SCRIPTD;
        placeMarker = HINTD;
        hintTimer = millis();
        //etc.
      }
      if (keyBuffer == DKEY) {
        //allLEDSoff();
        placeMarker = SCRIPTD;
        //displayNumber(keyBuffer);
        keyBuffer = -1;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTD.WAV");
        soundTimer = millis();
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case HINTD:
      if (keyPressed) {
        handleKeypress ();
      }
      /*if (BLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(DLED, !ledRead(DLED));
      }*/
      if ((keyBuffer == DKEY) || (HINTDURATION < (millis() - hintTimer))) {
        RAS.Stop();
        //allLEDSoff();
        ledWrite(DLED, HIGH);
        placeMarker = SCRIPTD;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTD.WAV");
        soundTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case SCRIPTD:
      if (keyPressed) {
        handleKeypress ();
      }
      if (FASTBLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(DLED, !ledRead(DLED));
      }
      if (SCRIPTDLENGTH < (millis() - soundTimer)) {
        RAS.Stop();
        //allLEDSoff();
        lightScript();
        placeMarker = RESPONDINGD;
        responseTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case RESPONDINGD:
      if (keyPressed) {
        handleKeypress ();
      }
      if (RESPONSEWAIT < (millis() - responseTimer)) {
        //placeMarker = SCRIPTE;
        placeMarker = HINTE;
        hintTimer = millis();
        //etc.
      }
      if (keyBuffer == EKEY) {
        //allLEDSoff();
        placeMarker = SCRIPTE;
        //displayNumber(keyBuffer);
        keyBuffer = -1;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTE.WAV");
        soundTimer = millis();
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case HINTE:
      if (keyPressed) {
        handleKeypress ();
      }
      /*if (BLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(ELED, !ledRead(ELED));
      }*/
      if ((keyBuffer == EKEY) || (HINTDURATION < (millis() - hintTimer))) {
        RAS.Stop();
        //allLEDSoff();
        ledWrite(ELED, HIGH);
        placeMarker = SCRIPTE;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTE.WAV");
        soundTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case SCRIPTE:
      if (keyPressed) {
        handleKeypress ();
      }
      if (FASTBLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(ELED, !ledRead(ELED));
      }
      if (SCRIPTELENGTH < (millis() - soundTimer)) {
        RAS.Stop();
        //allLEDSoff();
        lightScript();
        placeMarker = RESPONDINGE;
        responseTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case RESPONDINGE:
      if (keyPressed) {
        handleKeypress ();
      }
      if (RESPONSEWAIT < (millis() - responseTimer)) {
        placeMarker = HINTF;
        //placeMarker = SCRIPTF;
        hintTimer = millis();
        //etc.
      }
      if (keyBuffer == FKEY) {
        //allLEDSoff();
        placeMarker = SCRIPTF;
        //displayNumber(keyBuffer);
        keyBuffer = -1;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTF.WAV");
        soundTimer = millis();
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case HINTF:
      if (keyPressed) {
        handleKeypress ();
      }
      /*if (BLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(FLED, !ledRead(FLED));
      }*/
      if ((keyBuffer == FKEY) || (HINTDURATION < (millis() - hintTimer))) {
        RAS.Stop();
        //allLEDSoff();
        ledWrite(FLED, HIGH);
        placeMarker = SCRIPTF;
        RAS.Stop();
        RAS.PlayWAV("SCRIPTF.WAV");
        soundTimer = millis();
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      } else if (keyBuffer != -1) {
        //displayNumber(keyBuffer);
        readScript(keyBuffer);
        keyBuffer = -1;
      }
      break;

    case SCRIPTF:
      if (keyPressed) {
        handleKeypress ();
      }
      if (FASTBLINKDURATION < (millis() - blinkTimer)) {
        blinkTimer = millis();
        ledWrite(FLED, !ledRead(FLED));
      }
      if (SCRIPTFLENGTH < (millis() - soundTimer)) {
        RAS.Stop();
        allLEDSoff();
        lightKeypad();
        lightScript();
        clearLED();
        placeMarker = DIALNINE;
        // Clear display
        //displayNumber(keyBuffer);
        keyBuffer = -1;
      }
      break;

    
    default:
    	// Error
        break;
    
  }
}
