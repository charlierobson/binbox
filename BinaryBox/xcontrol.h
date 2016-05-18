/*
 *    XControl.h - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 * 
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 * 
 *    This permission notice shall be included in all copies or 
 *    substantial portions of the Software.
 * 
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef XControl_h
#define XControl_h

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class XControl
{
private:
    void spiTransfer(byte opcode, byte data);

    byte status[8];
    byte statusTransposed[8];

    int SPI_MOSI;
    int SPI_CLK;
    int SPI_CS;

 public:
    /* 
     * Create a new controler 
     * Params :
     * dataPin		pin on the Arduino where data gets shifted out
     * clockPin		pin for the clock
     * csPin		pin for selecting the device 
     */
     XControl(int dataPin, int clkPin, int csPin);

    /* 
     * Set the shutdown (power saving) mode for the device
     * Params :
     * addr	The address of the display to control
     * status	If true the device goes into power-down mode. Set to false
     *		for normal operation.
     */
     void shutdown(bool status);

    /* 
     * Set the number of digits (or rows) to be displayed.
     * See datasheet for sideeffects of the scanlimit on the brightness
     * of the display.
     * Params :
     * addr	address of the display to control
     * limit	number of digits to be displayed (1..8)
     */
     void setScanLimit(int limit);

    /* 
     * Set the brightness of the display.
     * Params:
     * addr		the address of the display to control
     * intensity	the brightness of the display. (0..15)
     */
     void setIntensity(int intensity);

    /* 
     * Switch all Leds on the display off. 
     * Params:
     * addr	address of the display to control
     */
     void clearDisplay(void);

     void updateDisplay(byte* data);
};

#endif	//XControl.h

