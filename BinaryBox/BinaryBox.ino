#include "XControl.h"

#include "game1.h"
#include "game2.h"

/*

  80
0    0
2    8
  20
1    4
0    0
  01    04
  
 */


XControl* lc;

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

// switch state
#define SS_OFF      0
#define SS_JUSTOFF  1
#define SS_JUSTON   2
#define SS_ON       3

#define SWITCHISON(x) ((switchStates[x]&2)==2)

byte debouncedSwitches[10];
byte switchStates[10];

// indices into debounced switch array
#define SN_MODE 8
#define SN_ACT 9

// teensy pin mapping to debounced switch array entry
int switchPinMap[10] =
{
  INP80, INP40, INP20, INP10,
  INP08, INP04, INP02, INP01,
  INPMO, INPAC
};

// 7-seg led bitmaps

byte characterBitmaps[16] = 
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

byte yes[3] = 
{
  0b01101010, 
  0b10110011,
  0b11100011
};

byte cache[8];
byte data[8] = { 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x04 };

int gt;

void setup()
{
  Serial.begin(19200);

  randomSeed(analogRead(0));
  
  pinMode(5,OUTPUT); // test pin
  
  pinMode(13,OUTPUT);

  for (int i = 14; i < 24; ++i)
  {
    pinMode(i, INPUT);
  }

  lc = new XControl(TEENSY_DIN,TEENSY_CLK,TEENSY_LOAD);

  // initialise debouncing/states to reflect ON or OFF
  for (int i = 0; i < 10; ++i)
  {
    debouncedSwitches[i] = digitalRead(switchPinMap[i]) ? 0xff : 0;
    switchStates[i] = debouncedSwitches[i] & SS_ON;
  }
}

void updateSwitches(void)
{
  byte v, before, after;
  
  for (int i = 0; i < 10; ++i)
  {
    v = debouncedSwitches[i];
    v <<= 1;
    v |= digitalRead(switchPinMap[i]);
    debouncedSwitches[i] = v;

    before = (v & 0b11100000) == 0b11100000;
    after  = (v & 0b00000111) == 0b00000111;
    switchStates[i] = before + (after << 1);
  }
}

bool actFlipped(void)
{
  return switchStates[SN_ACT] == SS_JUSTON;
}

byte collectBinary(void)
{
  int n = 0;
  for (int i = 0; i < 8; ++i)
  {
    n <<= 1;
    n |= SWITCHISON(i);
  }
  return n;
}

void outputBinary(byte n)
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

void outputToModule(byte moduleMask, byte bitmap)
{
  for (int i = 0; i < 8; ++i)
  {
    data[i] &= ~moduleMask;
    if (bitmap & 0x80)
      data[i] |= moduleMask;
    bitmap <<= 1;
  }
}

void outputDigit(byte moduleMask, byte digit)
{
  unsigned char bitmap = characterBitmaps[digit];
  outputToModule(moduleMask, bitmap);
}

void outputDecimal(byte n)
{
  int hundreds = n / 100;
  n -= hundreds * 100;
  outputDigit(0x80, hundreds);

  int tens = n / 10;
  n -= tens * 10;
  outputDigit(0x40, tens);
  
  outputDigit(0x20, n);
}

void outputHex(byte n)
{
  outputDigit(0x10, n >> 4);
  outputDigit(0x08, n & 15);
}

void clearModule(byte mask)
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




//

void* play_begin(void)
{
  return (void*)play_loop;
}

void* play_loop(void)
{
  int binary = collectBinary();
  outputBinary(binary);
  outputDecimal(binary);  
  outputHex(binary);

  return (void*)play_loop;
}

typedef void*(*WFN)(void);


extern void* game1_begin(void);
extern void* game2_begin(void);

int mode = 0;       // play mode. 1 = game
int loopCount = 31; // we want the buttons to be read on the first iteration

WFN modes[3] = { play_begin, game1_begin, game2_begin };
WFN fn = (WFN)modes[0];

byte randy;

void loop()
{
  loopCount = (loopCount + 1) & 31;
  if (loopCount == 0)
  {
    // shonky game timer, counts in milliseconds, _very_ roughly
    gt += 7;
    
    // about 7.5ms update rate
    digitalWrite(5, !digitalRead(5));
    updateSwitches();

    // switch mode when mode switch makes a positive edge transition
    if (switchStates[SN_MODE] == SS_JUSTON)
    {
      ++mode;
      mode %= sizeof(modes);
  
      fn = modes[mode];
    }
  }

  fn = (WFN)fn();

  lc->updateDisplay(data);
}

