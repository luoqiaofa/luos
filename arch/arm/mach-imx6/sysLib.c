/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : sysLib.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-07-05 05:30:00 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-07-05
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "os-cpu.h"
#include "bsp_clk.h"
#include "bsp_led.h"
#include "bsp_int.h"
#include "bsp_uart.h"
#include "bsp_epittimer.h"

#define _STR(x) #x
#define TO_STR(x) _STR(x)

int intLock(void)
{
    return cpuIntLock();
}

int intUnlock (int level)
{
    return cpuIntUnlock(level);
}

void LED_Init(void)
{
}

void ledToggle(void)
{
    static int state = 0;

    led_switch(LED0, state);
    state = !state;
}

#if defined(__CC_ARM)
int __asm cpuCntLeadZeros(cpudata_t val)
{
    clz     r0, r0
    bx      lr
}
#elif defined(__GNUC__)
int cpuCntLeadZeros(cpudata_t val)
{
    __asm__ __volatile__(
            "clz     %0, %0\n"
            : "+r"(val)
            : "r"(val)
            :
            );
    return val;
}
#else
#error "Unsuppoted compiler!"
#endif

int sysHwInit(void)
{
    GIC_Type *gic = (GIC_Type *)(__get_CBAR() & 0xFFFF0000UL);

	int_init(); 				/* 初始化中断(一定要最先调用！) */
	imx6u_clkinit();			/* 初始化系统时钟 			*/
	clk_enable();				/* 使能所有的时钟 1		*/
	led_init();					/* 初始化led 			*/
	// beep_init();				/* 初始化beep	 		*/
#if 0
    epit1_init(0, 66000000/2); 
    epit1_irqhandler_cb_setup(ledToggle);
#else
    // epit1_init(0, 66000000/1000); 
    // epit1_init(0, 66000000/2); 
    // epit1_irqhandler_cb_setup(tickAnnounce);
#endif


	uart_init();				/* 初始化串口，波特率115200 */
	
    printf("Version: %s %s\n", TO_STR(BUILD_DATE), TO_STR(BUILD_TIME));
    printf("CBAR=%p\n", gic);
    return 0;
}

void cpuTaskContextSwitchTrig(register void* cur, register void* tcb_high)
{
    cpuContextSwitch();
}

int cpuSysTicksConfig(uint32_t ticks)
{
    return 0;
}

