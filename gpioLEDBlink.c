/**
 * \file   gpioLEDBlink.c
 *
 *  \brief  This application uses a GPIO pin to blink the LED.
 *
 *          Application Configurations:
 *
 *              Modules Used:
 *                  GPIO1
 *
 *              Configuration Parameters:
 *                  None
 *
 *          Application Use Case:
 *              1) The GPIO pin GPIO0[13] is used as an output pin.
 *              2) This pin is alternately driven HIGH and LOW. A finite delay
 *                 is given to retain the pin in its current state.
 *
 *          Running the example:
 *              On running the example, the LED on beaglebone would be seen 
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
#include "beaglebone.h"
#include "gpio_v2.h"
#include "dmtimer.h"

/******************************************************************************
**                      INTERNAL MACRO DEFINITIONS
*******************************************************************************/
#define TIMER_INITIAL_COUNT             (0x80000000u)
#define TIMER_RLD_COUNT                 (0xC0000000u)


/*****************************************************************************
**                INTERNAL MACRO DEFINITIONS
*****************************************************************************/
#define GPIO_INSTANCE_ADDRESS           (SOC_GPIO_0_REGS)
#define GPIO_INSTANCE_PIN_NUMBER        (13)

/*****************************************************************************
**                INTERNAL FUNCTION PROTOTYPES
*****************************************************************************/
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
    /* Enabling functional clocks for GPIO1 instance. */
    GPIO1ModuleClkConfig();
    /* This function will enable clocks for the DMTimer2 instance */
    DMTimer2ModuleClkConfig();

    /* Setup and start the DMTimer2 to free run. */
    DMTimerDisable(SOC_DMTIMER_2_REGS);
    DMTimer2SetUp();

    /* Selecting GPIO0[13] pin for use. */
    GPIO0Pin13PinMuxSetup();
    
    /* Enabling the GPIO module. */
    GPIOModuleEnable(GPIO_INSTANCE_ADDRESS);

    /* Resetting the GPIO module. */
    GPIOModuleReset(GPIO_INSTANCE_ADDRESS);

    /* Setting the GPIO pin as an output pin. */
    GPIODirModeSet(GPIO_INSTANCE_ADDRESS,
                   GPIO_INSTANCE_PIN_NUMBER,
                   GPIO_DIR_OUTPUT);

    DMTimerEnable(SOC_DMTIMER_2_REGS);

    while(1)
    {
        /* Driving a logic HIGH on the GPIO pin. LED OFF */
        GPIOPinWrite(GPIO_INSTANCE_ADDRESS,
                     GPIO_INSTANCE_PIN_NUMBER,
                     GPIO_PIN_HIGH);

        Delay(0x5FFFF);

        /* Driving a logic LOW on the GPIO pin. LED ON */
        GPIOPinWrite(GPIO_INSTANCE_ADDRESS,
                     GPIO_INSTANCE_PIN_NUMBER,
                     GPIO_PIN_LOW);

        Delay(0x2FFFF);
    }

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
}


/******************************* End of file *********************************/
