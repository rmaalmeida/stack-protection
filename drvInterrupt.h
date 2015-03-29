// -----------------------------------------------------------------------
//   int.c -> fun��es para gerenciamento das interrup��es
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

#ifndef INT_H
    #define INT_H
    #include "ddCtr_types.h"

enum {
    INT_ADC_SET, INT_SERIAL_SET, INT_TIMER_SET, INT_ENABLE, INT_END
};

//apenas retorna o "objeto" do driver
driver* getInterruptDriver(void);

  #pragma CODE_SEG DEFAULT
    void MCU_init(void); /* Device initialization function declaration */
    //     Method      :  MCU_init (component MC9S12DG256_112)

  #pragma CODE_SEG __NEAR_SEG NON_BANKED
    __interrupt void isr_default(void);
    //     Interrupt handler : isr_default
  #pragma CODE_SEG DEFAULT


  #pragma CODE_SEG __NEAR_SEG NON_BANKED
    __interrupt void isrVrti(void);
    //     Interrupt handler : isr_default
  #pragma CODE_SEG DEFAULT

#endif //INT_H