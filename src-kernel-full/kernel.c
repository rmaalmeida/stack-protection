// -----------------------------------------------------------------------
//   kernel.c -> fun��es para gerenciamento do kernel
//   Autor:  Rodrigo Maximiano Antunes de Almeida
//          rodrigomax at unifei.edu.br
// -----------------------------------------------------------------------

#include "kernel.h"
#include "basico.h"
#include "derivative.h"
#include "crc.h"
#include "hamming.h"

//Defines de debug

//#define DEBUG_LCD

#ifdef DEBUG_LCD
  #include "drvLcd.h"
#endif

#define RTOS_LATE()       PORTB |=  0x30;

#define PROC_LATE()       PORTB |=  0x20;


#define IDLE_LED_ON()     PORTB |=  0x40;
#define IDLE_LED_OFF()    PORTB &= ~0x40;

#define CONTEXT_LED_ON()  PORTB |=  0x80;
#define CONTEXT_LED_OFF() PORTB &= ~0x80;

//**************************
// Defines para configuracao do modo de operacao do kernel
//#define prioRTOS

#define RRS
//#define CTES

#define HAMMING
//#define CRC
//#define MIX

//#define INPUTERROR
//***************************

#ifdef CRC
  #define correcao crc16
#endif

#ifdef HAMMING
  #define correcao hamming
#endif

#ifdef correcao
    #define CHECK
#endif

#ifdef MIX
    #define CHECK
#endif


//inconsistency check


#ifdef RRS
  #ifdef CTES
    #error "Mais de um escalonador escolhido"
  #endif
#endif


//*******************


//this is the Idle process
//the only process on kernel that is banked
void IdleFunc(void) {
  //enable int only when everthing is setted up
  asm CLI;
  //start Real Time Interrupt
  CRGINT |= (unsigned char)0x80U;
  
  for(;;){
    //the perfect place to energy saving
    IDLE_LED_ON(); 
  }
}


#define HEAP_SIZE        15*2
#define NUMBER_OF_SLOTS    2
#define NUMBER_OF_TASKS (NUMBER_OF_SLOTS - 1)
//put the idle task on last position
#define IDLE_PROC_ID    (NUMBER_OF_SLOTS - 1)
#define MIN_TIME                     (-30000)


#pragma CODE_SEG __NEAR_SEG NON_BANKED

  //Last postition available in the stack
  volatile static unsigned int SP;
  //1k vector for task's stack
  volatile unsigned char stack[HEAP_SIZE];
  
  //variaveis do kernel
  volatile process pool[NUMBER_OF_SLOTS];
  volatile int actualTask;
  
  #ifdef RRS
    //variable to define the actual task on round robin scheduler
    volatile int RRactualTask;  
  #endif

void timedLoopStart(signed int valor){
  pool[actualTask].Time = valor;
}

void timedLoopWait(void){
  //if it wants to wait, we mark as needed to restart
  //pool[actualTask].Started = 0;
  pool[actualTask].Status = WAITING;
  while(pool[actualTask].Status == WAITING);
}

void delay(signed int valor){
  timedLoopStart(valor);
  timedLoopWait();
}

void createStack(procFunc func, char* position){
  position--;
  *position = (unsigned char)((long)func>> 8);
  position--;
  *position = (unsigned char)((long)func>> 16);
  position--;
  *position = 0x66;
  position--;
  *position = 0x55;
  position--;
  *position = 0x44;
  position--;
  *position = 0x33;
  position--;
  *position = 0x22;
  position--;
  *position = 0x11;
  position--;
  *position = 0x00;
  position--;
  *position = (unsigned char)((long)func >> 0);
  //stack end point
}

void CreateIdleProc(void){
  //idle creation
  pool[IDLE_PROC_ID].StackInit = SP;
  //pushing function address into the stack (low first)
  pool[IDLE_PROC_ID].Status = READY;
  pool[IDLE_PROC_ID].Function = IdleFunc;
  
  
  //functions pointers use a different addressing sequence
  //ADDR High / ADDR Low / PPage
  //filling the stack as an interrupt would do plus PPAGE value
  //pc (2) y(2) x(2) d(2) CCR(1) PPage(1)
  SP--;
  *((unsigned char*)SP) = (unsigned char)((long)IdleFunc>> 8);
  SP--;
  *((unsigned char*)SP) = (unsigned char)((long)IdleFunc>> 16);
  SP--;
  *((unsigned char*)SP) = 0x66;
  SP--;
  *((unsigned char*)SP) = 0x55;
  SP--;
  *((unsigned char*)SP) = 0x44;
  SP--;
  *((unsigned char*)SP) = 0x33;
  SP--;
  *((unsigned char*)SP) = 0x22;
  SP--;
  *((unsigned char*)SP) = 0x11;
  SP--;
  *((unsigned char*)SP) = 0x00;
  SP--;
  *((unsigned char*)SP) = (unsigned char)((long)IdleFunc >> 0);
  //stack end point
  pool[IDLE_PROC_ID].StackPoint = (unsigned int)SP;
  //jump to the next "stack slot"
  //idle consume only 10 stack bytes but using 50 due serial monitor needs
  SP = pool[IDLE_PROC_ID].StackInit - 50;
  #ifdef DEBUG_LCD
    writeLcd4('i',1);
  #endif
}

char kernelInit(void) {
  unsigned char i;
  #ifdef DEBUG_LCD
    LCD_init();
  #endif
  //starting all positions as empty
  for(i=0;i<NUMBER_OF_TASKS;i++){
    pool[i].Status = EMPTY;
  }
  //nextTask = 0;
  
  //Pointing the SP to the bottom of stack
  SP = (unsigned int)&stack + HEAP_SIZE;
  CreateIdleProc();
  actualTask = IDLE_PROC_ID;

  return FIM_OK;
}
 int restartTask(int task){
  unsigned int spDummy;
  int i;

 
  spDummy = pool[task].StackInit;

  pool[task].Status = READY;
  pool[task].Time = 10;//give a little time to all process to avoid RTOS false warning

 //pushing function address into the stack (low first)
    
  //functions pointers use a different addressing sequence
  //ADDR High / ADDR Low / PPage
  //filling the stack as an interrupt would do plus PPAGE value
  //PC, Y, X, D (B:A), CCR  
  
  //pc
  spDummy--;
  *((unsigned char*)spDummy) = (unsigned char)((long)pool[task].Function >> 8);
  spDummy--;
  *((unsigned char*)spDummy) = (unsigned char)((long)pool[task].Function >> 16);
  //y
  spDummy--;
  *((unsigned char*)spDummy) = 0x66;
  spDummy--;
  *((unsigned char*)spDummy) = 0x55;
  //x
  spDummy--;
  *((unsigned char*)spDummy) = 0x44;
  spDummy--;
  *((unsigned char*)spDummy) = 0x33;
  //d (a:b)
  spDummy--;
  *((unsigned char*)spDummy) = 0x22;
  spDummy--;
  *((unsigned char*)spDummy) = 0x11;
  //CCR
  spDummy--;
  *((unsigned char*)spDummy) = 0x00;
  /////////PPAGE
  spDummy--;
  *((unsigned char*)spDummy) = (unsigned char)((long)pool[task].Function >> 0);
  /////////CRC

#ifdef correcao  
  i = correcao((unsigned char*)spDummy,10);
#endif

#ifdef MIX
  if (pool[task].Prio == RTOS){
    i = hamming((unsigned char*)spDummy,10);
  } else {
    i = crc16((unsigned char*)spDummy,10); 
  }
#endif

#ifdef CHECK
  spDummy--;
  *((unsigned char*)spDummy) = (unsigned char)(i >> 0);
  spDummy--;
  *((unsigned char*)spDummy) = (unsigned char)(i >> 8);
#endif



  //stack end point
  pool[task].StackPoint = (unsigned int)spDummy;
  return spDummy;
}

  
process * kernelAddProc(procFunc func, int stkSize, priorityMode newPrio){
  //volatile to avoid warnings
  volatile process * temp;
  unsigned int i;
  int nextSlot;
  //find next available slot
  for(i=0;i<NUMBER_OF_TASKS;i++){
    if (pool[i].Status == EMPTY){
      nextSlot = i;
      break;
    }
  }
  //there is no space
  if (i == NUMBER_OF_TASKS){
    //return 0; 
  }
 
  temp = &pool[nextSlot];
  temp->StackInit = (unsigned int) SP;
  temp->Prio = newPrio;

  //temp->Started = 0;//the process start in READY not on RUNNING 
  temp->Status = READY;
  temp->Function = func;
  temp->Time = 10; //give a little time to all process to avoid RTOS false warning

  //pushing function address into the stack (low first)
    
  //functions pointers use a different addressing sequence
  //ADDR High / ADDR Low / PPage
  //filling the stack as an interrupt would do plus PPAGE value
  //PC, Y, X, D (B:A), CCR  
  
  //pc
  SP--;
  *((unsigned char*)SP) = (unsigned char)((long)func >> 8);
  SP--;
  *((unsigned char*)SP) = (unsigned char)((long)func >> 16);
  //y
  SP--;
  *((unsigned char*)SP) = 0x66;
  SP--;
  *((unsigned char*)SP) = 0x55;
  //x
  SP--;
  *((unsigned char*)SP) = 0x44;
  SP--;
  *((unsigned char*)SP) = 0x33;
  //d (a:b)
  SP--;
  *((unsigned char*)SP) = 0x22;
  SP--;
  *((unsigned char*)SP) = 0x11;
  //CCR
  SP--;
  *((unsigned char*)SP) = 0x00;
  /////////PPAGE
  SP--;
  *((unsigned char*)SP) = (unsigned char)((long)func >> 0);
  /////////CRC

#ifdef correcao  
  i = correcao((unsigned char*)SP,10);
#endif

#ifdef MIX
  if (temp->Prio == RTOS){
    i = hamming((unsigned char*)SP,10);
  } else {
    i = crc16((unsigned char*)SP,10); 
  }
#endif

#ifdef CHECK

  SP--;
  *((unsigned char*)SP) = (unsigned char)(i >> 0);
  SP--;
  *((unsigned char*)SP) = (unsigned char)(i >> 8);

#endif

  //stack end point
  temp->StackPoint = (unsigned int)SP;

  //jump to the next stack free position
  SP = temp->StackInit - stkSize;
  #ifdef DEBUG_LCD
    writeLcd4((nextSlot+48),1);
  #endif
  return temp;
}

//look for the next task ready to run
int Scheduler(void){
  int i;
  int next = IDLE_PROC_ID;
  #ifdef DEBUG_LCD
    int rt_task = -1;    
  #endif
  
  //+++++++++++++++++++++++++
  //update tick count for each process except the one running
  for (i = 0; i < NUMBER_OF_TASKS; i++){
    if (pool[i].Status != EMPTY){      
      if (pool[i].Time > MIN_TIME){
        pool[i].Time--;
      }
      if (pool[i].Time <= 0){
        pool[i].Status = READY;
        #ifdef DEBUG_LCD
          if(pool[i].Prio == RTOS){
            rt_task = i;
          }
        #endif
      }
      
    }
  }

#ifdef prioRTOS  
  //+++++++++++++++++++++++++
  //RTOS priority check
  for (i = 0; i < NUMBER_OF_TASKS; i++){
    if((pool[i].Prio == RTOS) &&
       (pool[i].Status == READY)) {
      return i;
    }
  }
#endif

#ifdef RRS
  //+++++++++++++++++++++++++
  //RRS: round robbing scheduler
  //start from the next task and go all the way arround
  i = (RRactualTask+1);
  
  if(i>=NUMBER_OF_TASKS) {
    i = 0; 
  }
  
  while( (i != RRactualTask) &&
         (pool[i].Status != READY)
       ){
     i++;
     if(i>=NUMBER_OF_TASKS) {
       i = 0; 
     }
  }

  //if the variable "i" comes back to nextTask
  //there aren't any other task willing to run
  if( (i == RRactualTask) &&
      (pool[i].Status == WAITING) ) {
    return IDLE_PROC_ID;    
  } else {
    RRactualTask=i;
    next = i; 
  }
#endif

#ifdef CTES
  //+++++++++++++++++++++++++
  //CTES: closest to execute scheduler
  //find the next READY task
  i=0;
  //the idle function is the last one and is always ready
  while(pool[i].Status != READY) {
    i++;
  }
  next = i;
  //the loop will iterate until the last process
  //if i has the IDLE_PROC_ID the loop will not run 
  for (i = (next+1); i < NUMBER_OF_TASKS; i++){
    if((pool[i].Status == READY) &&
       (pool[i].Time < pool[next].Time)) {
      next = i;
    }
  }
#endif
  //in the end next has the next task for both scheduler algorithms

#ifdef DEBUG_LCD
  if((rt_task !=-1) &&
     (rt_task != next)) {
    /*writeLcd4(0x8f,0);
    writeLcd4('!',1);*/
    RTOS_LATE();
  } 
  
#endif

  return next;
}






#ifdef INPUTERROR
  //variable to test bit errors
  unsigned char BITERRADO;
#endif


void interrupt kernelClock(void){
  
  //at this point CCR,D,X,Y,SP are already stored on the stack
  
  //the SPdummy and crc_on_stack variables are created on the stack top position
  volatile unsigned int SPdummy;       //used as stack pointer temp value
  #ifdef CHECK
  volatile unsigned int crc_on_stack;  //will point to the crc value on the stack

  //just to avoid removing crc_on_stack when optmizing without CRC check
  crc_on_stack = 1;  
  #endif

  
  //####################
  // Bit set for time evaluation
  CONTEXT_LED_ON();
  //####################  
  
  //led used for IDLE time
  IDLE_LED_OFF();
  
  // crc_on_stack and SPdummy variables are located on the stack
  // in order to save the PPage and CRC values on the right position
  // the two variables must be removed from the stack
  
  //remove SPdummy
  __asm PULD;
  
  //removecrc_on_stack
  #ifdef CHECK
  __asm PULD;
  #endif

  // Storing PPage on the stack
  __asm LDAA 0x30;
  __asm PSHA;
  
  //recreating the space for crc_on_stack
  #ifdef CHECK
  __asm PSHD;
  #endif
  
  //recreating the space for SPdummy
  __asm PSHD;

  //fill SPdummy with actual stack postition to use on
  //crc calculation  ###333###
  __asm TSX;
  __asm STX SPdummy;

#ifdef correcao  
  //SPdummy have its value incremented by 4 to ignore both SPdummy
  //and crc_on_stack from the CRC calculation.
  crc_on_stack = correcao((unsigned char *)SPdummy+4,10);
#endif

#ifdef MIX
  if (pool[actualTask].Prio == RTOS){
    crc_on_stack  = hamming((unsigned char *)SPdummy+4,10);
  } else {
    crc_on_stack  = crc16((unsigned char *)SPdummy+4,10); 
  }
#endif
    
  //####################
  // Context switch start
  //####################
  
  //save SP value on process info for further recover
  //#ifdef CHECK
    //not nedded if there is no CRC, it is already done before at ###333###
  __asm TSX;
  __asm STX SPdummy;
  //#endif

 
  //incrementing by 2 to point to the top of crc_on_stack variable
  #ifdef CHECK
  pool[actualTask].StackPoint = SPdummy+2;
  #else
  pool[actualTask].StackPoint = SPdummy;
  #endif
  //it can be on WAITING or RUNNING state.
  //must change only on RUNNING
  if (pool[actualTask].Status == RUNNING){
    pool[actualTask].Status = READY;
  }

  // Calling the scheduler to get the next task to run
  actualTask = Scheduler();
  
  //setting the next task as running
  pool[actualTask].Status = RUNNING;
  //mark its execution as started
  //pool[actualTask].Started = 1;
  
  //load the next task SP from process info
  SPdummy = pool[actualTask].StackPoint;
 
  __asm LDX SPdummy;
  __asm TXS;
  
  //restore space for crc_on_stack variable
  #ifdef CHECK
  __asm PSHD;
  #endif

#ifdef INPUTERROR
  //********************
  //Error routine
  //Inserting one error on each available bit in task stackframe (one at a time)
  *((unsigned char *)(pool[actualTask].StackPoint+((BITERRADO/8)%12))) =
    *((unsigned char *)(pool[actualTask].StackPoint+((BITERRADO/8)%12))) ^ (1<<BITERRADO%8);  
  BITERRADO++; //grab another bit next time
  //********************
#endif  


//TODO use (unsigned char *)SPdummy+4
//(unsigned char *)(pool[actualTask].StackPoint+2)
//instead of 

#ifdef MIX
  if (pool[actualTask].Prio == RTOS){
    SPdummy  = hamming((unsigned char *)(pool[actualTask].StackPoint+2),10);
    if (crc_on_stack != SPdummy) {
    //making XOR to find bit changed
    //Subtract 136 becouse the hammming routine is implemented differently
      crc_on_stack = (crc_on_stack^SPdummy) - 136;
      //check if the error is on the CRC or the data
      if (crc_on_stack < 80){
        //coorecting the bit
        *((unsigned char *)(pool[actualTask].StackPoint+2+(crc_on_stack/8))) =
          *((unsigned char *)(pool[actualTask].StackPoint+2+(crc_on_stack/8))) ^ (1<<(crc_on_stack%8));
      }
    }
  } else {
    SPdummy= crc16((unsigned char *)(pool[actualTask].StackPoint+2),10); 
    if (crc_on_stack != SPdummy) {
       SPdummy = restartTask(actualTask)-2;
       __asm LDX SPdummy;
       __asm TXS;
    }
  }
#endif

#ifdef correcao  
 //Using SPdummy to calculate the CRC for the new process
  //+2 is needed to exclude crc_on_stack from the calculation
  SPdummy = correcao((unsigned char *)(pool[actualTask].StackPoint+2),10);
  //crc_on_stack has the old value and SPdummy the fresh one
  if (crc_on_stack != SPdummy) {
  #ifdef HAMMING
  //making XOR to find bit changed
  //Subtract 136 becouse the hammming routine is implemented differently
    crc_on_stack = (crc_on_stack^SPdummy) - 136;
    //check if the error is on the CRC or the data
    if (crc_on_stack <80){
      //coorecting the bit
      *((unsigned char *)(pool[actualTask].StackPoint+2+(crc_on_stack/8))) =
        *((unsigned char *)(pool[actualTask].StackPoint+2+(crc_on_stack/8))) ^ (1<<(crc_on_stack%8));
    }
  #endif
  #ifdef CRC
      SPdummy = restartTask(actualTask)-2;
      __asm LDX SPdummy;
      __asm TXS;
  #endif
  }  
#endif

  //In this point the context frame is cheked and good to go
  //Removing the two extra variables, SPdummy and crc_on_stack,
  __asm PULD;
  #ifdef CHECK
  __asm PULD;
  #endif
  // and setting the PPage for the next process
  __asm PULA;
  __asm STAA 0x30;  
  
  //####################
  // Bit cleared for time evaluation
  CONTEXT_LED_OFF();
  //####################
  CRGFLG = 0x80; //clearing the RTI flag
  //All other context loading is done by RTI
  __asm RTI;
}


void kernelLoop(void){
  unsigned int SPdummy;
  #ifdef DEBUG_LCD
    int i;  
  #endif

  pool[IDLE_PROC_ID].Status = RUNNING;
  SPdummy = pool[IDLE_PROC_ID].StackPoint;
  actualTask = IDLE_PROC_ID;
  //writing 2nd line with kernel options
  #ifdef DEBUG_LCD
    writeLcd4(0xC0,0);
    #ifdef RRS
      writeLcd4('R',1);
      writeLcd4('R',1);
    #endif
    #ifdef CTES
      writeLcd4('C',1);
      writeLcd4('E',1);
    #endif
    writeLcd4(' ',1);
    #ifdef HAMMING
      writeLcd4('H',1);
      writeLcd4('a',1);
      writeLcd4('m',1);
    #else
      #ifdef CRC
        writeLcd4('C',1);
        writeLcd4('R',1);
        writeLcd4('C',1);
      #else
        #ifdef MIX
          writeLcd4('M',1);
          writeLcd4('i',1);
          writeLcd4('x',1);
        #else
          writeLcd4('S',1);
          writeLcd4('/',1);
          writeLcd4('C',1);
        #endif //mix
      #endif  //crc
    #endif   //lcd
    writeLcd4(' ',1); 
    #ifdef INPUTERROR
      writeLcd4('C',1);
      writeLcd4('/',1);
      writeLcd4('E',1);
    #else
      writeLcd4('S',1);
      writeLcd4('/',1);
      writeLcd4('E',1);
    #endif
    writeLcd4(' ',1);
    #ifdef prioRTOS
      writeLcd4('R',1);
      writeLcd4('T',1);
    #else
      writeLcd4('N',1);
      writeLcd4('o',1);
    #endif
    writeLcd4(' ',1);
    //the pool must be sequential
    i=0;
    while(pool[i].Status != EMPTY){
      i++;
    }
    writeLcd4((i/10)+48,1);
    writeLcd4((i%10)+48,1);
  #endif
  
  
  __asm LDX SPdummy;
  __asm TXS;
  
  //restore PPAGE
  __asm PULA;
  __asm STAA 0x30;

  //load context is done by RTI
  __asm RTI;

  //should not get here
  for(;;){
 
  }
}
#pragma CODE_SEG DEFAULT
