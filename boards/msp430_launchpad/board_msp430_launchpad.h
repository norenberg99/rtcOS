/******************************************************************************
    Filename: board_msp430_launchpad.h
    Description: 
        This file contains defines for the OS for this board.
    
    Written: Jon Norenberg, 2013 - 2016
    Copyright: My code is your code.  No warranties. 
******************************************************************************/

#ifndef MSP430_LAUNCHPAD_H
#define MSP430_LAUNCHPAD_H

//******************************************************************************
// Some generic constants
//******************************************************************************

#ifndef TRUE
#define TRUE                            1
#endif

#ifndef FALSE
#define FALSE                           0
#endif

//******************************************************************************
// Set these to the appropriate sizes
//******************************************************************************
typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;

//******************************************************************************
// Change the size of these if needed
//******************************************************************************
typedef int8 osStatus_t;
typedef uint8 osTaskID_t;
typedef uint32 osEvents_t;
typedef uint32 osTick_t;
typedef uint32 osMsg_t;
typedef uint32 osTaskParam_t;
typedef uint8 osMsgCount_t;
typedef uint16 osIndex_t;
typedef uint8 bool;

//******************************************************************************
// Define how this system handles critical sections
//******************************************************************************
#if 1
#define ENTER_CRITICAL_SECTION		__enable_interrupt()	
#define EXIT_CRITICAL_SECTION		__disable_interrupt()
#define CRITICAL_SECTION_VARIABLE	
#else
#define ENTER_CRITICAL_SECTION		__sr = _get_interrupt_state(); __disable_interrupt()
#define EXIT_CRITICAL_SECTION		__set_interrupt_state(__sr)
#define CRITICAL_SECTION_VARIABLE	uint16 __sr
#endif
//******************************************************************************
// Set the max number of tasks in the system
//******************************************************************************
#define MAX_TASKS_IN_SYSTEM             5

//******************************************************************************
// Defines how many future events that can be scheduled.
// This is for the entire system, not per task
//******************************************************************************
#define MAX_FUTURE_EVENTS               4

//******************************************************************************
// Set the max number of messages that can be sent to each task
// This is for each task in the system
// If you do not want to use messages in your system, set this to 0
//******************************************************************************
#define MAX_MESSAGES_IN_SYSTEM          3


#endif
