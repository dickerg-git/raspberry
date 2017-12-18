/**
 * \file   gpioLEDBlink.c
 *
 *  \brief  This application uses a GPIO pin to blink the LED.
 *
 *          Application Configurations:
 *
 *              Modules Used:
 *                  GPIO0
 *                  GPIO1
 *
 *              Configuration Parameters:
 *                  None
 *
 *          Application Use Case:
 *              1) The GPIO pin GPIO0[13] is used as an output pin.
 *              2) This pin is alternately driven HIGH and LOW. A finite delay
 *                 is given to retain the pin in its current state.
 *              3) The GPIO pin GPIO1[09] is used as an input pin, to read the switch.
 *              4) The pin is normally pulled HIGH, the switch connects to GROUND.
 *              5) The GPIO pin GPIO1[08] is used as an input pin, currently unused.
 *
 *          Running the example:
 *              On running the example, the LED on PROTO CARD would be seen
 *              turning ON and OFF alternatively.
 *
 */

/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
*/
/* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


#include "soc_AM335x.h"
#include "cache.h"
#include "beaglebone.h"
#include "gpio_v2.h"
#include "dmtimer.h"
#include "interrupt.h"

/******************************************************************************
**                      INTERNAL MACRO DEFINITIONS
*******************************************************************************/
#define TIMER_INITIAL_COUNT             (0x80000000u)
#define TIMER_RLD_COUNT                 (0xC0000000u)


/*****************************************************************************
**                INTERNAL FUNCTION PROTOTYPES
*****************************************************************************/
void HW_Setup_GPIO_AM335x(void);
void HW_GPIO_BlinkLED(int turnOff);
unsigned int HW_GPIO_Reset_switch(void);
unsigned int HW_GPIO_Debug_switch(void);
void BootFromFlash(void);

static void Delay(unsigned int count);
static void DMTimer2SetUp(void);

/*****************************************************************************
**                INTERNAL FUNCTION DEFINITIONS
*****************************************************************************/
/*
** The main function. Application starts here.
*/
int main()
{
    unsigned int N_Reset = 1;               /* State of the N_Reset pin. */
    unsigned int blinks = 10000;

    CacheEnable(CACHE_ALL);

    /* Set up the AM335x GPIO pins as connected to this prototype board. */
    /* Set up pins for LED and Reset switch.                             */
    /* ALSO, init the interrupt controller Aintc.                        */
    HW_Setup_GPIO_AM335x();

    /* Setup and start the DMTimer2 to free run, generate interrupt.     */
    /* This function will enable clocks for the DMTimer2 instance */
    DMTimer2ModuleClkConfig();
    DMTimerDisable(SOC_DMTIMER_2_REGS);
    DMTimer2SetUp();

    /* Enable IRQ in CPSR */
    IntMasterIRQEnable();

    /* Enable the DMTimer2 counting function. */
    DMTimerEnable(SOC_DMTIMER_2_REGS);

    while(blinks--)
    {
        HW_GPIO_BlinkLED(0);

        /* Look at the RESET switch state. */
        N_Reset = HW_GPIO_Reset_switch(); /* Equals 0 or 1 */

        /* Press the switch to stop blinking, perform some tasks. */
        if( N_Reset == 0 ) break;
    }

    /* Do some real work now... */
    if( HW_GPIO_Debug_switch() == 0 )
       BootFromFlash();

    HW_GPIO_BlinkLED(1);
    while(1);

} 

/*
** A function which is used to generate a delay.
*/
static void Delay(volatile unsigned int count)
{
    while(count--);
}


/*
** Setup the timer for one-shot and compare mode.
*/
static void DMTimer2SetUp(void)
{
    /* Load the load register with the reload count value */
    DMTimerReloadSet(SOC_DMTIMER_2_REGS, TIMER_RLD_COUNT);

    /* Load the counter with the initial count value */
    DMTimerCounterSet(SOC_DMTIMER_2_REGS, TIMER_INITIAL_COUNT);

    /* Configure the DMTimer for Auto-reload and compare mode */
    DMTimerModeConfigure(SOC_DMTIMER_2_REGS, DMTIMER_AUTORLD_NOCMP_ENABLE);

    /* Enable the DMTimer interrupts */
    DMTimerIntEnable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);

}


#define GPIO_BANK0_ADDRESS              (SOC_GPIO_0_REGS)
#define GPIO_BANK1_ADDRESS              (SOC_GPIO_1_REGS)

/*
** A function to blink the LED OFF, then ON.
** If requested, exit with the LED off again.
** Wired to the GPIO0-13 pin.
*/
void HW_GPIO_BlinkLED(int turnOff)
{
#define GPIO_LED_PIN_NUMBER             (13)

	unsigned int delay;

	/* Driving a logic HIGH on the GPIO pin. LED OFF */
	GPIOPinWrite(GPIO_BANK0_ADDRESS,
                 GPIO_LED_PIN_NUMBER,
                 GPIO_PIN_HIGH);

	delay = 0x5FFFF;
	while( delay--);

	/* Driving a logic LOW on the GPIO pin. LED ON */
	GPIOPinWrite(GPIO_BANK0_ADDRESS,
                 GPIO_LED_PIN_NUMBER,
                 GPIO_PIN_LOW);

    delay = 0x2FFFF;
    while( delay--);

    if (turnOff) {
        /* Exit the routine with the LED OFF. */
        GPIOPinWrite(GPIO_BANK0_ADDRESS,
                     GPIO_LED_PIN_NUMBER,
                     GPIO_PIN_HIGH);
    }
}


/*
** A function to return the state of the PROTO Reset button.
** Reads the pin wired to GPIO1-09, returns 0 or 1.
*/
unsigned int HW_GPIO_Reset_switch(void)
{
#define GPIO_RESET_PIN_NUMBER           (9)

    return(
	   GPIOPinRead(GPIO_BANK1_ADDRESS,
                   GPIO_RESET_PIN_NUMBER)
       >> GPIO_RESET_PIN_NUMBER
    );

}


/*
** A function to return the state of the PROTO Debug button.
** Reads the pin wired to GPIO1-08, returns 0 or 1.
*/
unsigned int HW_GPIO_Debug_switch(void)
{
#define GPIO_RESET_PIN_NUMBER           (8)

    return(
	   GPIOPinRead(GPIO_BANK1_ADDRESS,
                   GPIO_RESET_PIN_NUMBER)
       >> GPIO_RESET_PIN_NUMBER
    );

}


/* Attempt to boot the PROTO card from it's Flash */
void BootFromFlash(void)
{
	unsigned long volatile * from_ptr = (unsigned long volatile *) 0x08000000;
	unsigned long volatile * to_ptr   = (unsigned long volatile *) 0x80000000;

	/* Read the Flash code header length and address. */
    unsigned long code_length  = *((long*)from_ptr++);
    unsigned long code_address = *((long*)from_ptr++);
    unsigned long code_bytes = 0;

    /* Turn off the D-Cache, to get Write-Thru into the DDR. */
    CacheDataCleanAll();
    CacheDisable(CACHE_DCACHE);

    /* Verify the code address in the image points to DDR space. */
    if (code_address != 0x80000000) return;
    if (code_length > 10000000) return;

    while( code_length > 0 ) {
    	*to_ptr = *from_ptr;
        if(*to_ptr != *from_ptr ) while(1); /* Data Miscompare, HALT! */

        to_ptr++;
		from_ptr++;

		code_length -= 4;
        code_bytes  += 4;
    }

}


void HW_Setup_GPIO_AM335x(void)
{
    /* Enabling functional clocks for the GPIO instances.         */
    GPIO0ModuleClkConfig();
    GPIO1ModuleClkConfig();

    /* Selecting GPIO0[13] GPIO1[09] pin for use.       */
    /* These routines need Supervisor mode privileges.  */
    GPIO0Pin13PinMuxSetup();  /* GPIO0-13 OUTPUT to LED */
    GPIO1Pin09PinMuxSetup();  /* GPIO1-09 INPUT/PULL-UP */

    /* Enabling the GPIO modules. */
    GPIOModuleEnable(GPIO_BANK0_ADDRESS);
    GPIOModuleEnable(GPIO_BANK1_ADDRESS);

    /* Resetting the GPIO modules. */
    GPIOModuleReset(GPIO_BANK0_ADDRESS);
    GPIOModuleReset(GPIO_BANK1_ADDRESS);

    /* Setting the GPIO-0 LED pin as an output pin. */
    GPIODirModeSet(GPIO_BANK0_ADDRESS,
                   GPIO_LED_PIN_NUMBER,
                   GPIO_DIR_OUTPUT);
    /* Setting the GPIO-1 RESET pin as an input pin. */
    GPIODirModeSet(GPIO_BANK1_ADDRESS,
                   GPIO_RESET_PIN_NUMBER,
                    GPIO_DIR_INPUT);

    /* Initialize the ARM interrupt controller system. */
    IntAINTCInit();

}
/******************************* End of file *********************************/
