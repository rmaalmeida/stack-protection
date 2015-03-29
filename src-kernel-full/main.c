#include "drvInterrupt.h"
#include "drvTimer.h"
#include "kernel.h"
#include "ddCtr.h"
#include "derivative.h"


/*
v	tempo	
 200 	 7,2	0,036
 300 	11,1	0,037
 400 	15,0	0,0375
 500 	19,0	0,038
 600 	22,8	0,038
 700 	26,7	0,0381428571
 800 	30,6	0,03825
 900 	34,8	0,0386666667
1000	38,0	0,038
m�dia 0,037728836
		
tempo	valor	
10	265	
20	530	
30	795	
40	1060	
50	1325	
60	1590	
70	1855	
80	2120	
90	2385	
100	2650
*/

//multiplica todo mundo por 15,15
#define t10 265	
#define t20 530	
#define t30 795	
#define t40 1060	
#define t50 20075 //*ja ta multiplicado*/	
#define t60 1590	
#define t70 1855	
#define t80 2120	
#define t90 2385	
#define t100 2650	


void proc0(void) {
  volatile long int f;
  for(;;){
    timedLoopStart(1000);//2s
    PORTB|=0x01;
    for(f=0;f<t50;f++);
    PORTB&=~0x01;
    timedLoopWait();
  }
}

void proc1(void) {
  volatile long int f;
  for(;;){
    timedLoopStart(125);
    for(f=0;f<t50;f++);
    PORTB^=0x02;
    timedLoopWait();
  }
}

void proc2(void) {
  volatile long int f;
  for(;;){
    timedLoopStart(125);
    for(f=0;f<t50;f++);
    PORTB^=0x04;
    timedLoopWait();
  }
}

void proc3(void) {
  volatile long int f;
  for(;;){
    timedLoopStart(125);
    for(f=0;f<t50;f++);
    PORTB^=0x08;
    timedLoopWait();
  }
}

void proc4(void) {
  volatile long int f;
  for(;;){
    timedLoopStart(125);
    for(f=0;f<t50;f++);
    timedLoopWait();
  }
}

void proc5(void) {
  volatile long int f;
  for(;;){
    timedLoopStart(125);
    for(f=0;f<t50;f++);
    timedLoopWait();
  }
}

void main(void) {
    int NPROC = 4;
    
    MCU_init(); /* call Device Initialization */
    
    //portb is exclusive for debug info.
    DDRB = 0xFF;
    PORTB = 0x00;

    //inicialização do sistema
  
    kernelInit();
    initCtrDrv();
    
    
    initDriver(DRV_TIMER);
    initDriver(DRV_INTERRUPT);
   
    
    
    //callDriver(DRV_INTERRUPT, INT_TIMER_SET, kernelClock);
    
    
    
    //insercao dos processos no kernel
    kernelAddProc(proc0, 50, RTOS);
   if(NPROC>=1) kernelAddProc(proc1, 50, NORMAL);
   if(NPROC>=2) kernelAddProc(proc2, 50, NORMAL);
   if(NPROC>=3) kernelAddProc(proc3, 50, NORMAL);
   if(NPROC>=4) kernelAddProc(proc4, 50, NORMAL);
   if(NPROC>=5) kernelAddProc(proc5, 50, NORMAL);    
    
    //Start do kernel
    kernelLoop();
}

//implementação dos processos

