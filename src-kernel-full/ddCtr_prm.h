/*
   File:   ctrdrv_prm.h
   Author: rmaax

   Created on 14 de Abril de 2011, 09:56
 */

#ifndef ctrdrv_prm_h
    #define ctrdrv_prm_h
//quantidade máxima de drivers disponíveis ao mesmo tempo
    #define QNTD_DRV 4

    #include "drvGenerico.h"
    #include "drvInterrupt.h"
    #include "drvTimer.h"

enum {
    DRV_GEN,
    DRV_INTERRUPT,
    DRV_TIMER,
    DRV_END /*DRV_END should always be the last*/
};
//the functions to get the drivers should be put in the same order as in the enum
static ptrGetDrv drvGetFunc[DRV_END] = {
    getGenericoDriver,
    getInterruptDriver,
    getTimerDriver
};

#endif // ctrdrv_prm_h
