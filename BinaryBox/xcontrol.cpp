/*
      XControl.cpp - A library for controling Leds with a MAX7219/MAX7221
      Copyright (c) 2007 Eberhard Fahle

      Permission is hereby granted, free of charge, to any person
      obtaining a copy of this software and associated documentation
      files (the "Software"), to deal in the Software without
      restriction, including without limitation the rights to use,
      copy, modify, merge, publish, distribute, sublicense, and/or sell
      copies of the Software, and to permit persons to whom the
      Software is furnished to do so, subject to the following
      conditions:XControl

      This permission notice shall be included in all copies or
      substantial portions of the Software.

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
      EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
      OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
      NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
      HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
      WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
      FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
      OTHER DEALINGS IN THE SOFTWARE.
*/

#include "XControl.h"

//the opcodes for the MAX7221 and MAX7219
#define OP_NOOP   0
#define OP_DIGIT0 1
#define OP_DIGIT1 2
#define OP_DIGIT2 3
#define OP_DIGIT3 4
#define OP_DIGIT4 5
#define OP_DIGIT5 6
#define OP_DIGIT6 7
#define OP_DIGIT7 8
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

XControl::XControl(int dataPin, int clkPin, int csPin)
{
  SPI_MOSI = dataPin;
  SPI_CLK = clkPin;
  SPI_CS = csPin;
  
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_CLK, OUTPUT);
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);

  for (int i = 0; i < 8; i++)
  {
    status[i] = 0x00;
  }

  spiTransfer(OP_DISPLAYTEST, 0);
  spiTransfer(OP_DECODEMODE, 0);

  setIntensity(15);
  setScanLimit(7);
  clearDisplay();

  shutdown(false);
}

void XControl::spiTransfer(byte opcode, byte data)
{
  digitalWrite(SPI_CS, LOW);
 
  shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, opcode);
  shiftOut(SPI_MOSI, SPI_CLK, MSBFIRST, data);

  digitalWrite(SPI_CS, HIGH);
}

void XControl::shutdown(bool b)
{
  if (b)
    spiTransfer(OP_SHUTDOWN, 0);
  else
    spiTransfer(OP_SHUTDOWN, 1);
}

void XControl::setScanLimit(int limit)
{
  spiTransfer(OP_SCANLIMIT, limit & 7);
}

void XControl::setIntensity(int intensity)
{
  spiTransfer(OP_INTENSITY, intensity & 15);
}

void XControl::clearDisplay(void)
{
  for (int i = 0; i < 8; i++)
  {
    spiTransfer(i + OP_DIGIT0, 0x00);
  }
}

void XControl::updateDisplay(byte* data)
{
  for (int i = 0; i < 8; i++)
  {
    spiTransfer(i + OP_DIGIT0, data[i]);
  }
}

