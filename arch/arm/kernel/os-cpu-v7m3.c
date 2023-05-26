/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : arm-v7m3.c
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-25 10:15:55 AM
 * 
 * 修改记录1:
 *    修改日期: 2023-05-25
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
#include "taskLib.h"

void cpuStackInit(LUOS_TCB *tcb, FUNCPTR exitRtn)
{
    V7M3_STK_REGS *stk;
    
    stk = (V7M3_STK_REGS *)tcb->stack;
    stk--;
    stk->R4  = 0x04040404;
    stk->R5  = 0x05050505;
    stk->R6  = 0x06060606;
    stk->R7  = 0x07070707;
    stk->R8  = 0x08080808;
    stk->R9  = 0x09090909;
    stk->R10 = 0x10101010;
    stk->R11 = 0x11111111;
    stk->R0  = (uint32_t)tcb->param;
    stk->R1  = (uint32_t)tcb->stkLimit;
    stk->R2  = 0x02020202;
    stk->R3  = 0x03030303;
    stk->R12 = 0x12121212;
    stk->LR  = (uint32_t)exitRtn; /* R14 */
    stk->PC  = (uint32_t)tcb->taskEntry; /* R15 */
    stk->PSR = 0x01000000u;
}

int intLock(void)
{
    return 0;
}

int intUnlock (int oldSR)
{
    return 0;
}

#if defined(__CC_ARM)
int __asm cpuCntLeadZeros(ULONG val)
{
    clz     r0, r0
    bx      lr
}
#else /* gnu gcc */
int cpuCntLeadZeros(ULONG val)
{
    __asm__ __volatile__(
            "clz     %0, %0\n"
            : "+r"(val)
            : "r"(val)
            : 
            ); 
    return val;
}
#endif

