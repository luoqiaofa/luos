/*
*********************************************************************************************************
*                                     BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2013; BigLuo Technology Co., Ltd
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Eval-STM32F103
*                                     Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : EHS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_MODULE
#include <bsp.h>

/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint32_t  BSP_CPU_ClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;


    RCC_GetClocksFreq(&rcc_clocks);

    return ((int32_t)rcc_clocks.HCLK_Frequency);
}

int cpuSysTicksConfig(uint32_t ticks)
{
    uint32_t  cnts;
    uint32_t sysfreq = BSP_CPU_ClkFreq();

    cnts = sysfreq/ticks;
	if (0 == SysTick_Config(cnts)) {
	    return 0;
	}
	return -1;
}

void ledToggle(void)
{
	macLED2_TOGGLE();
}
