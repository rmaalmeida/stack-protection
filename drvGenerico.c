/*
   File:   drvGenerico.c
   Author: rmaax

   Created on 13 de Abril de 2011, 10:14
 */
#include "basico.h"
#include "derivative.h"
#include "drvGenerico.h"
//quantidade de funções neste driver

static driver thisDriver;
static driver_functions this_functions[GEN_END];

//funcionalidades implementadas
char changePORTB(void *parameters) {
    PORTB = *((char*) parameters);
    return FIM_OK;
}

//inicialização do driver
char initGenerico(void *parameters) {
    DDRB = 0xFF;
    
    thisDriver.drv_id = *((char*) parameters);
    this_functions[GEN_PORTB].func_ptr = changePORTB;

    return FIM_OK;
}

driver* getGenericoDriver(void) {
    //to ensure that at least the init function will be known
    thisDriver.drv_init = initGenerico;
    thisDriver.func_ptr = (driver_functions*) & this_functions;
    return &thisDriver;
}




