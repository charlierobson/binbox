#include "XControl.h"

XControl* lc;

// switch states
#define ON 1
#define JUSTON 3
#define JUSTOFF 4
#define OFF 0

// teensy pins
#define TEENSY_DIN 2
#define TEENSY_CLK 3
#define TEENSY_LOAD 4

#define INP80 23
#define INP40 22
#define INP20 21
#define INP10 20

#define INPMO 19
#define INPAC 18

#define INP01 17
#define INP02 16
#define INP04 15
#define INP08 14

// bit numbers for LEDs
#define LED_MODE 1
#define LED_ACTN 5

#define LED_H80 0
#define LED_H01 1
#define LED_H10 2
#define LED_H08 3
#define LED_H40 4
#define LED_H02 5
#define LED_H20 6
#define LED_H04 7

byte debouncedSwitches[10];

// indices into debounced switch array
#define SW_MODE 8
#define SW_ACT 9

// teensy pin mapping to debounced switch array entry
int switchPinMap[10] =
{
  INP80, INP40, INP20, INP10,
  INP08, INP04, INP02, INP01,
  INPMO, INPAC
};

// 7-seg led bitmaps

unsigned char characterBitmaps[16] = 
{
  0b11011011,
  0b01001000,
  0b10111001,
  0b11101001,
  0b01101010,
  0b11100011,
  0b11110011,
  0b11001000,
  0b11111011,
  0b11101010,

  0b11111010,
  0b01110011,
  0b10010011,
  0b01111001,
  0b10110011,
  0b10110010
};

unsigned char yes[3] = 
{
  0b01101010, 
  0b10110011,
  0b11100011
};

byte cache[8];
byte data[8] = { 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x04 };


void setup()
{
  Serial.begin(19200);

  pinMode(13,OUTPUT);

  for (int i = 14; i < 24; ++i)
  {
    pinMode(i, INPUT);
  }

  lc = new XControl(TEENSY_DIN,TEENSY_CLK,TEENSY_LOAD);

  // initialise debouncing to reflect ON or OFF
  for (int i = 0; i < 10; ++i)
  {
    debouncedSwitches[i] = digitalRead(switchPinMap[i]) ? 0xff : 0;
  }
}


void updateSwitches(void)
{
  for (int i = 0; i < 10; ++i)
  {
    debouncedSwitches[i] <<= 1;
    debouncedSwitches[i] |= digitalRead(switchPinMap[i]);
  }
}

int switchStates[] = {OFF, JUSTOFF, JUSTON, ON};

int switchState(int switchID)
{
  byte before = (debouncedSwitches[switchID] & 0b11100000) == 0b11100000;
  byte after  = ((debouncedSwitches[switchID] & 0b00000111) == 0b00000111) << 1;

  return switchStates[before + after];
}

int collectBinary(void)
{
  int n = 0;
  for (int i = 0; i < 8; ++i)
  {
    n <<= 1;
    n |= switchState(i) & 1;
  }
  return n;
}

void outputBinary(int n)
{
  for (int i = 0; i < 8; ++i)
    data[i] &= 0b11111011;

  if (n & 0x80) data[LED_H80] |= 4;
  if (n & 0x40) data[LED_H40] |= 4;
  if (n & 0x20) data[LED_H20] |= 4;
  if (n & 0x10) data[LED_H10] |= 4;
  if (n & 0x08) data[LED_H08] |= 4;
  if (n & 0x04) data[LED_H04] |= 4;
  if (n & 0x02) data[LED_H02] |= 4;
  if (n & 0x01) data[LED_H01] |= 4;
}

void outputToModule(int moduleMask, byte bitmap)
{
  for (int i = 0; i < 8; ++i)
  {
    data[i] &= ~moduleMask;
    if (bitmap & 0x80)
      data[i] |= moduleMask;
    bitmap <<= 1;
  }
}

void outputDigit(int moduleMask, int digit)
{
  unsigned char bitmap = characterBitmaps[digit];
  outputToModule(moduleMask, bitmap);
}

void outputDecimal(int n)
{
  int hundreds = n / 100;
  n -= hundreds * 100;
  outputDigit(0x80, hundreds);

  int tens = n / 10;
  n -= tens * 10;
  outputDigit(0x40, tens);
  
  outputDigit(0x20, n);
}

void outputHex(int n)
{
  outputDigit(0x10, n >> 4);
  outputDigit(0x08, n & 15);
}

void clearModule(int mask)
{
  for (int i = 0; i < 8; ++i)
  {
    data[i] &= mask;
  }
}

void lightMode(bool mode)
{
  data[LED_H01] = data[LED_H01] & ~2;
  data[LED_H01] |= mode ? 2 : 0;
}

void lightAction(bool action)
{
  data[LED_H02] = data[LED_H02] & ~2;
  data[LED_H02] |= action ? 2 : 0;
}

void cacheData(void)
{
  memcpy(cache, data, 8);
}

void restoreData(void)
{
  memcpy(data, cache, 8);
}

void flashYes(void)
{
  cacheData();

  for (int i = 0; i < 12; ++i)
  {
    clearModule(0b00011111);
    lc->updateDisplay(data);
    delay(100);
    outputToModule(0x80, yes[0]);
    outputToModule(0x40, yes[1]);
    outputToModule(0x20, yes[2]);
    lc->updateDisplay(data);
    delay(100);
  }

  restoreData();
  lc->updateDisplay(data);
}


void loop()
{
  // should be called every 5-10ms
  updateSwitches();

  int binary = collectBinary();
  outputBinary(binary);
  outputDecimal(binary);  
  outputHex(binary);

  lightAction(switchState(SW_ACT));
  lightMode(switchState(SW_MODE));

  lc->updateDisplay(data);
}

