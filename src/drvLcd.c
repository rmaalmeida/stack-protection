/* Functions to drive the Dragon12 LCD module 
 * V1.0 March 9, 2004
 * Louis Bertrand <louis.bertrand@durhamc.on.ca>
 * Keith Baldwin <baldwinka@hotmail.com>
 * Justin Ryan <sparkyelectrician@hotmail.com>
 * Durham College, Oshawa ON
 *
 * Based on Dragon12 documentation and Lcd.asm example
 * by Wayne Chu, EVBPLUS.COM/Wytec Co
 * http://www.evbplus.com/
 *
 * Compile with EmbeddedGNU/gcc-m68hc12, 
 * Dragon12 hardware profile
 *
 * LCD Module is in 4-bit mode.
 * Port K bit assignment:
 *   7 = unused
 *   6 = unused
 *   5 = Module DB7
 *   4 = Module DB6
 *   3 = Module DB5
 *   2 = Module DB4
 *   1 = EN (pulse 1 to write)
 *   0 = RS (0=command, 1=data)
 *
 * Released under a Berkeley-style licence
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Durham College nor the names of the authors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COLLEGE AND AUTHORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COLLEGE OR AUTHORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/* register definitions follow Motorola documentation for
 * MC9S12DP256B MCU
 */

/* fw-02-05 :  - added header file lcd.h to export public functions
               - replaced _io_base[portk] by PORTK
               - renamed initLCD to LCD_init (consistency with our prgs.) */ 

#include "derivative.h"       /* derivative information */


/* Magic delay constants, based on 24MHz CPU clock */
#define ENBIT (0x02)
#define DELAY40US (2000L)
#define DELAY4_1MS (220000L)
#define DELAY100US (5000L)
#define LCDWIDTH (16)


/* local functions */

void
lcdDelay(unsigned long constant) {
  volatile unsigned long counter;

  for(counter = constant; counter > 0; counter--);
 } // end lcdDelay()


/* Write LCD module in 8-bit mode
 * Inputs:
 *  data: to be written, lower 4 bits are ignored
 *  rs: register select, only bit 0 is significant
 * Handles the shifting into place and the EN pulsing
 * This is only used at the start of the init sequence
 */
void writeLcd8(unsigned char data) {
  unsigned char temp;

  /* shift upper nibble to data bits in port K */
  temp = (data >> 2); // rs is always 0
  /* Now do the EN pulsing */
  PORTK = temp; // write with EN=0
  PORTK = temp | ENBIT; // write with EN=1
  PORTK = temp; // write with EN=0
  /* allow instruction to complete */
  lcdDelay(DELAY40US);
} // end writeLcd8()


/* Write LCD module in 4-bit mode
 * Inputs:
 *  data: to be written, 8 bits are significant
 *  rs: register select, only bit 0 is significant
 * Does two consecutive writes, high nibble, then low
 * Handles the shifting into place and the EN pulsing
 * This is can be used at any time (init and display)
 */
void writeLcd4(unsigned char data, unsigned char rs) {
  unsigned char hi, lo;

  /* split byte into 2 nibbles and shift to line up
   * with data bits in port K
   */
  hi = ((data & 0xf0) >> 2) | (rs & 0x01) ;
  lo = ((data & 0x0f) << 2) | (rs & 0x01) ;

  /* do write pulses for upper, then lower nibbles */
  PORTK = hi; // write with EN=0
  PORTK = hi | ENBIT; // write with EN=1
  PORTK = hi; // write with EN=0
  PORTK = lo; // write with EN=0
  PORTK = lo | ENBIT; // write with EN=1
  PORTK = lo; // write with EN=0
  /* allow instruction to complete */
  lcdDelay(DELAY40US);
} // end writeLcd4()




/* public functions */


/* Initialise LCD module
 * (Here there be dragons. Good luck.)
 */
void
LCD_init(void) {
   /* initialise port */
   DDRK = 0xff;

   writeLcd8(0x30);  // tell it once
   lcdDelay(DELAY4_1MS);
   writeLcd8(0x30);  // tell it twice
   lcdDelay(DELAY100US);
   writeLcd8(0x30);  // tell it thrice

  /* last write in 8-bit mode sets bus to 4 bit mode */
  writeLcd8(0x20);

  /* Now we are in 4 bit mode, write upper/lower nibble */
  writeLcd4(0x28, 0); // last function set: 4-bit mode, 2 lines, 5x7 matrix
  writeLcd4(0x0c, 0); // display on, cursor off, blink off
  writeLcd4(0x01, 0); // display clear
  writeLcd4(0x06, 0); // cursor auto-increment, disable display shift
} // end initLcd()


/* Write a line to the LCD
 * Inputs:
 *  string is a pointer to an array of char to be sent. It must be 16 long.
 *  line tells which line to display (0=top line, 1=bottom line). Defaults to top.
 *
 * If the string is shorter, we will write garbage to the LCD.
 * If it's longer, it gets truncated.
 */
void
writeLine(char *string, int line) {
	int charCount;
	unsigned char instruction;

	/* Set address in LCD module */
	if( 1 == line)
		instruction = 0xc0; // write bottom line
	else
		instruction = 0x80; // write top line
	writeLcd4( instruction, 0);  // rs=0 means command

	/* blast out 16 bytes */
	for( charCount = 0; charCount < LCDWIDTH; charCount++) {
		writeLcd4( string[charCount], 1); // rs=1 means data
	}
} // end writeLine()

