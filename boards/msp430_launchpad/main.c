/******************************************************************************
    Filename: main.c
    Description: 
        Test code for the RTC OS, running on an msp430 g2553 launchpad.
		Normally the tasks might be in separate files for clarity,
		but here they are all in this one file.
    
    Written: Jon Norenberg, 2013 - 2016
    Copyright: My code is your code.  No warranties. 
******************************************************************************/
 
#include  "msp430g2553.h"
#include  "rtcOS.h"

//******************************************************************************
// Constants
//******************************************************************************

// board gpio definitions
#define     LED1                  BIT0
#define     LED2                  BIT6
#define     LED_DIR               P1DIR
#define     LED_OUT               P1OUT

#define     BUTTON                BIT3
#define     BUTTON_OUT            P1OUT
#define     BUTTON_DIR            P1DIR
#define     BUTTON_IN             P1IN
#define     BUTTON_IE             P1IE
#define     BUTTON_IES            P1IES
#define     BUTTON_IFG            P1IFG
#define     BUTTON_REN            P1REN


// task constants
#define SEND_EVENT_NOW          0
#define DO_NOT_RELOAD_DELAY     FALSE
#define RELOAD_DELAY            TRUE

// 0 is the highest priority, task numbers must be consecutive, no gaps
#define TASK1_ID            0
#define TASK2_ID            1
#define TASK3_ID            4		// kind of the idle task, so put it at lowest priority
#define TASK4_ID            3
#define TASK5_ID            2

// events for task one
#define T1_EVT_TEST1        0x01
#define T1_EVT_TEST2        0x02

// events for task two
#define T2_EVT_TEST1        0x01
#define T2_EVT_TEST2        0x02

// events for task three
#define T3_EVT_TEST1        0x01
#define T3_EVT_TEST2        0x02

// events for task four
#define T4_EVT_TEST1        0x01

//******************************************************************************
// Globals
//******************************************************************************
osTimer_t theTimer;


//******************************************************************************
// Protypes
//******************************************************************************

void InitializeLeds(void);
void InitializeButton(void);
void PreApplicationMode(void);                     
void ConfigureAdcTempSensor(void);
void ConfigureTimerPwm(void);
void ConfigureTimerUart(void);
void Transmit(void);
void InitializeClocks(void);

/******************************************************************************
    @fn     TaskOne

    @brief  This task when it receives an event will turn on the led.  It
			will then send an event to task 2 to turn off the led.

    @param	eventFlags - 1 bit flags indicating events
			msgCount - number of messages waiting for this task
			taskParam - value that was given at task creation time
            
    @return 
******************************************************************************/
osEvents_t TaskOne( osEvents_t eventFlags, osMsgCount_t msgCount, osTaskParam_t taskParam )
{
    
    if ( eventFlags & T1_EVT_TEST1 )
    {   // this event would be the highest priority for this task
        // since it is handled first
        
		// turn the LED on (HIGH is the voltage level)
		LED_OUT |= LED1;                          
		
		// send event to task 2 to turn off the led
        osSendEvent( TASK2_ID, T2_EVT_TEST1, 10, DO_NOT_RELOAD_DELAY );
        
        // then return the events that have NOT been handled
        return eventFlags & ~T1_EVT_TEST1;
    }
        
    // maybe there are some events that we do not care about
    // this will throw those events away
    return 0;
}


/******************************************************************************
    @fn     TaskTwo

    @brief  This task when it receives an event will turn off the led.  It
			will then send an event to task 1 to turn on the led.

    @param	eventFlags - 1 bit flags indicating events
			msgCount - number of messages waiting for this task
			taskParam - value that was given at task creation time
            
    @return 
******************************************************************************/
osEvents_t TaskTwo( osEvents_t eventFlags, osMsgCount_t msgCount, osTaskParam_t taskParam )
{

    if ( eventFlags & T2_EVT_TEST1 )
    {   // this event would be the highest priority for this task
        // since it is handled first
        
		// turn the LED off by making the voltage LOW
		LED_OUT &= ~LED1;                          

		// send event to task 1 to turn on the led, event will be delivered in x ms
        osSendEvent( TASK1_ID, T1_EVT_TEST1, 10, DO_NOT_RELOAD_DELAY );
        
        return eventFlags & ~T2_EVT_TEST1;
    }
    
    if ( eventFlags & T2_EVT_TEST2 )
    {
        // do something
        // then return the events that have NOT been handled
        return eventFlags & ~T2_EVT_TEST2;
    }
    
    // maybe there are some events that we do not care about
    // this will throw those events away
    return 0;
}

/******************************************************************************
    @fn     TaskThree

    @brief  This is setup as the idle task so that it gets called whenever
			there is nothing to do in the system.  This happens because it
			is the lowest priority task and it keeps sending an event to
			itself.

    @param	eventFlags - 1 bit flags indicating events
			msgCount - number of messages waiting for this task
			taskParam - value that was given at task creation time
            
    @return 
******************************************************************************/
osEvents_t TaskThree( osEvents_t eventFlags, osMsgCount_t msgCount, osTaskParam_t taskParam )
{
    if ( eventFlags & T3_EVT_TEST1 )
    {   
		// need to create a timer, that will be checked all the time
		osTimerCreate( 40, &theTimer );

		// send an event to ourselves to test the timer
		osSendEvent( TASK3_ID, T3_EVT_TEST2, SEND_EVENT_NOW, DO_NOT_RELOAD_DELAY );
        
        return eventFlags & ~T3_EVT_TEST1;
    }
        
    if ( eventFlags & T3_EVT_TEST2 )
    {	// this will be called every time the os done not have a higher priority task 
		// notice that we did not clear the event when the return is called
        
		if ( osTimerExpired( &theTimer ))
		{
			//Serial.println( osGetTickCount(), HEX );
			//Serial.println( "Timer Expired, creating another!" );
			osTimerCreate( 40, &theTimer );
		}
		
        return eventFlags;
    }
        
    // maybe there are some events that we do not care about
    // this will throw those events away
    return 0;
}

/******************************************************************************
    @fn     TaskFour

    @brief  This is the event processing for the task.  It is the 
            task entry point.  It will only be called if it has an
            event pending.  That event could come from another task,
            isr, or itself.

    @param	eventFlags - 1 bit flags indicating events
			msgCount - number of messages waiting for this task
			taskParam - value that was given at task creation time
            
    @return 
******************************************************************************/
osEvents_t TaskFour( osEvents_t eventFlags, osMsgCount_t msgCount, osTaskParam_t taskParam )
{
    static osMsg_t messageData = 0;
    
    if ( eventFlags & T4_EVT_TEST1 )
    {   // this event would be the highest priority for this task
        // since it is handled first
        
        if ( OS_ERR_NONE != osSendMessage( TASK5_ID, messageData ))
        {
			//Serial.println( "T4: error sending message" );
		}

        // then return the events that have NOT been handled
        return eventFlags & ~T4_EVT_TEST1;
    }
	
	if ( msgCount )
	{
		if ( OS_ERR_NONE == osGetMessage( &messageData ))
		{ 	// somebody sent us a message
			//Serial.println( "T4: message received" );
			//Serial.println( messageData, DEC );
			// now that we sent a message, then received a message, schedule delay event
			// to start it all over again
			osSendEvent( TASK4_ID, T4_EVT_TEST1, 30, DO_NOT_RELOAD_DELAY );
		}
	}
    
    // maybe there are some events that we do not care about
    // this will throw those events away
    return 0;
}

/******************************************************************************
    @fn     TaskFour

    @brief  Task 4 is going to send us a message and then we need to 
			add 1 to the msg and send it back.  

    @param	eventFlags - 1 bit flags indicating events
			msgCount - number of messages waiting for this task
			taskParam - value that was given at task creation time
            
    @return 
******************************************************************************/
osEvents_t TaskFive( osEvents_t eventFlags, osMsgCount_t msgCount, osTaskParam_t taskParam )
{
    osMsg_t newMessage;
    		
	if ( msgCount )
	{	// just handle one message then return, for higher priority tasks
		if ( OS_ERR_NONE == osGetMessage( &newMessage ))
		{ 	// somebody sent us a message
			//Serial.println( "T5: message received" );
			//Serial.println( newMessage, DEC );
			
			// increment and then send back
			++newMessage;
			if ( OS_ERR_NONE != osSendMessage( TASK4_ID, newMessage ))
			{
				//Serial.println( "T4: error sending message" );
			}
		}
		else
		{	// if the task was invoked, then we should have a message
			//Serial.println( "T5: error, no message" );
		}
	}
    
    // maybe there are some events that we do not care about
    // this will throw those events away
    return 0;
}


/******************************************************************************
    @fn     SystemSleepHandler

    @brief  This routine will get called when there are no events for any 
            task in the system.  Put the processor to sleep and wake on
            an interrupt.  The interrupt will then send an event to 
            some task.

    @param  
            
    @return 
******************************************************************************/
void SystemSleepHandler( void )
{
    // possibly stop the processor until an interrupt occurs
    
}

/******************************************************************************
    @fn     tick_isr

    @brief  This routine is called because the hw timer (TIMER_A) fired.  
			Turn around and call the os to update the system.

    @param  
            
    @return 
******************************************************************************/
#pragma vector=TIMER0_A1_VECTOR
__interrupt void tick_isr(void)
{
	// clear the interrupt
	TACCTL1 &= ~CCIFG;
 
	// call the os to update the system
	osUpdateTick();
 
}

/******************************************************************************
    @fn     hwInit

    @brief  This routine init's the hardware.

    @param  
            
    @return 
******************************************************************************/
void hwInit( void )
{
	// stop the watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	// INIT THE CLOCKS
	// Set range
	BCSCTL1 = CALBC1_1MHZ;                    
	DCOCTL = CALDCO_1MHZ;
	// SMCLK = DCO = 1MHz
	BCSCTL2 &= ~(DIVS_3);                     

	// INIT THE BUTTONS
	BUTTON_DIR &= ~BUTTON;
	BUTTON_OUT |= BUTTON;
	BUTTON_REN |= BUTTON;
	BUTTON_IES |= BUTTON;
	BUTTON_IFG &= ~BUTTON;
	//BUTTON_IE |= BUTTON;

	// INIT THE LEDS
	// set to outputs and clear both
	LED_DIR |= LED1 + LED2;
	LED_OUT &= ~(LED1 + LED2);

}

/******************************************************************************
    @fn     startTickInterrupt

    @brief  Program the timer to interrupt every x ms.

    @param  
            
    @return 
******************************************************************************/
void startTickInterrupt( void )
{
	
	// TACLK = SMCLK, Up mode, divide by 4.	
	// SMCLK = 1,000,000 Hz / 4 = 250,000 Hz = 4 us
	// we want tick to be every 100 ms, 100,000 us, so 4 x 25,000 should get 100 ms
	TACCR0 = 25000;
	TACTL = TASSEL_2 | MC_1 | ID_2;
	//TACTL = TASSEL_1 | MC_1;
	// TACCTL1 Capture Compare	
	TACCTL1 = CCIE;
}

/******************************************************************************
    @fn     main

    @brief  Loop function runs over and over again forever, but with the 
			RTC OS, once osRun is called it will never return.

    @param  
            
    @return 
******************************************************************************/
void main( void )
{
	hwInit();

	osInit();

	// register tasks so that they can receive events
    osRegisterTaskEventHandler( TaskOne, 	TASK1_ID, 0 );
    osRegisterTaskEventHandler( TaskTwo, 	TASK2_ID, 0 ); 
    osRegisterTaskEventHandler( TaskThree, 	TASK3_ID, 0 ); 
    osRegisterTaskEventHandler( TaskFour, 	TASK4_ID, 0 ); 
    osRegisterTaskEventHandler( TaskFive, 	TASK5_ID, 0 ); 
    
    // register a routine that will get called if there are no events for any tasks
    osRegisterSystemSleepHandler( SystemSleepHandler );
 
    // send event to turn on led
    osSendEvent( TASK1_ID, T1_EVT_TEST1, SEND_EVENT_NOW, DO_NOT_RELOAD_DELAY );
	// send event to create the timer
    osSendEvent( TASK3_ID, T3_EVT_TEST1, SEND_EVENT_NOW, DO_NOT_RELOAD_DELAY );
	// start the messaging exchange with an event
	osSendEvent( TASK4_ID, T4_EVT_TEST1, SEND_EVENT_NOW, DO_NOT_RELOAD_DELAY );
    
	__enable_interrupt(); 

	startTickInterrupt();
  
    // start the system running, it will never return from this routine
    osRun();
}
 
