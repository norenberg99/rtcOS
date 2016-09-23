/******************************************************************************
    Filename: boards.h
    Description: 
        This file contains include mechanism for all the different board files.
		These files define the parameters for the os on a particular board.
    
    Written: Jon Norenberg, 2013 - 2016
    Copyright: My code is your code.  No warranties. 
******************************************************************************/

#ifndef BOARDS_H
#define BOARDS_H

//******************************************************************************
// Include the appropriate board file depending on the build
//******************************************************************************

#if BOARD_MSP430_LAUNCHPAD
#include "board_msp430_launchpad.h"

#elif BOARD_ARDUINO_XXXX
warning Copy the entire arduino_uno directory to your Arduino projects directory

#else
	
warning No board file has been specified for this build

#endif


#endif
