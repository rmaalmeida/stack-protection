// -----------------------------------------------------------------------
//   timer.c -> fun��es para acesso ao timer 0
//   Autor:  Rodrigo Maximiano Antunes de Almeida
//          rodrigomax at unifei.edu.br
// -----------------------------------------------------------------------
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; version 2 of the License.
// 
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
// -----------------------------------------------------------------------

#include "basico.h"
#include "derivative.h" 
#include "drvTimer.h"

static driver thisDriver;
static driver_functions this_functions[TMR_END];


char startTimer(void *parameters) {
  parameters = parameters;
      /* CRGINT: RTIE=1 */
  CRGINT |= (unsigned char)0x80U; 
    return FIM_OK;
}

char initTimer(void *parameters) {

  thisDriver.drv_id = *(char*) parameters;
  this_functions[TMR_START].func_ptr = startTimer;

  /* ### Init_RTI init code */
  /* CRGINT: RTIE=0 */
  CRGINT &= (unsigned char)~(unsigned char)0x80U;                     
  /* CRGFLG: RTIF=1,PORF=0,LOCKIF=0,LOCK=0,TRACK=0,SCMIF=0,SCM=0 */
  CRGFLG = 0x80U;                                      
  /* PLLCTL: PRE=0 */
  PLLCTL &= (unsigned char)~(unsigned char)0x04U;                     
  /* CLKSEL: RTIWAI=0 */
  CLKSEL &= (unsigned char)~(unsigned char)0x02U;                     
                     
  /* RTICTL: RTR6=1,RTR5=1,RTR4=1,RTR3=1,RTR2=1,RTR1=1,RTR0=1 */
  RTICTL = 0x27U;
  //first nible is power 1 = 2^10, 2 = 2^11
  //second nible is multiplier factor, 7 = 7 and on.
  /* ### */
    
    return FIM_OK;
}

driver* getTimerDriver(void) {
    //to ensure that at least the init function will be known
    thisDriver.drv_init = initTimer;
    thisDriver.func_ptr = (driver_functions*) & this_functions;
    return &thisDriver;
}
