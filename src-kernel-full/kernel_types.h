/*
   File:   kernel_types.h
   Author: rmaax

   Created on 14 de Abril de 2011, 10:03
 */

#ifndef kernel_types_h
    #define kernel_types_h
    
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Context Tasks Structure Prototypes          /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


    
typedef enum {
  EMPTY, RUNNING, READY, WAITING
} processState;

typedef enum {
  NORMAL, RTOS
} priorityMode;


typedef void (* procFunc) (void);


/**
* \struct ContextType
* Context Task Structure
* Used by the task control block
*/
typedef struct
{
   //const CHAR8 * TaskName;   ///< Task name
   //volatile to avoid warnings
   volatile unsigned int StackPoint;          ///< Current position of virtual stack pointer
   volatile unsigned int StackInit;           ///< Virtual stack pointer init
   volatile processState Status;           ///< Actual process state
   volatile signed int Time;
   volatile priorityMode Prio;
   volatile procFunc Function;
   //volatile int Started;
}process;


//funcoes do kernel





#endif // kernel_types_h
