/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : arm-regs.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-16 03:46:16 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-05-16
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef __REGSARM_H__
#define __REGSARM_H__

#ifndef  NVIC_INT_CTRL
#define  NVIC_INT_CTRL      *((uint32_t *)0xE000ED04)
#endif

#ifndef  NVIC_PENDSVSET
#define  NVIC_PENDSVSET     0x10000000
#endif
static inline void cpuTaskContextSwitchTrig()
{
    NVIC_INT_CTRL = NVIC_PENDSVSET;
}

static inline void cpuIntContextSwitchTrig()
{
    NVIC_INT_CTRL = NVIC_PENDSVSET;
}

typedef struct armv7m3_stkregs {
    /* R0-R15 and xPSR need save in stack, 
     * R0-R3,R12,LR,PC,xPSR is auto pushed to stack when interrupt occur 
     * R4-R11,R12,LR,PC,xPSR is auto pushed to stack when interrupt occur 
     */
    cpureg_t R4;
    cpureg_t R5;
    cpureg_t R6;
    cpureg_t R7;
    cpureg_t R8;
    cpureg_t R9;
    cpureg_t R10;
    cpureg_t R11;
    /* follow regs are auto stored while interrupt occur, R13 is SP */
    cpureg_t R0;
    cpureg_t R1;
    cpureg_t R2;
    cpureg_t R3;
    cpureg_t R12;
    cpureg_t LR; /* R14 */
    cpureg_t PC; /* R15 */
    cpureg_t PSR;
} V7M3_STK_REGS;

typedef struct coretex_v7m3_regs {
    V7M3_STK_REGS stkRegs;
    cpureg_t      MSP;
    cpureg_t      PSP;
    cpureg_t      BASEPRI;
    cpureg_t      PRIMASK;
    cpureg_t      FAULTMASK;
    cpureg_t      CONTROL;
} REG_SET;

extern void cpuStackInit(LUOS_TCB *tcb, FUNCPTR exitRtn);
IMPORT int intLock(void);
IMPORT int intUnlock(int oldSR);
#endif /* #ifndef __REGSARM_H__ */


