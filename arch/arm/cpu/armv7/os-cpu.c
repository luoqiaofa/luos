/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : os-cpu.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-07-05 06:07:08 PM
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
#include "coreLib.h"

static inline cpureg_t cpsrGet(void)
{
    cpureg_t val;
    __asm("mrs %0, cpsr\n"
            :"=r"(val)
            ::
         );
    return val;
}

void cpuStackInit(LUOS_TCB *tcb, FUNCPTR exitRtn)
{
    STK_REGS *stk;

    stk = (STK_REGS *)tcb->stack;
    stk--;
    stk->R0  = (uint32_t)tcb->param;
    stk->R1  = (uint32_t)tcb->stkLimit;
    stk->R2  = 0x02020202;
    stk->R3  = 0x03030303;
    stk->R4  = 0x04040404;
    stk->R5  = 0x05050505;
    stk->R6  = 0x06060606;
    stk->R7  = 0x07070707;
    stk->R8  = 0x08080808;
    stk->R9  = 0x09090909;
    stk->R10 = 0x10101010;
    stk->R11 = 0x11111111;
    stk->R12 = 0x12121212;
    stk->LR  = (uint32_t)exitRtn; /* R14 */
    stk->PC  = (uint32_t)tcb->taskEntry; /* R15 */
    stk->SPSR = 0x40000173; /* cpsrGet()*/;
    tcb->stack = stk;
}

