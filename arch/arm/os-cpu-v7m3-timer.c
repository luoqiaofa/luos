/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : os-cpu-v7m3-timer.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-25 01:36:41 PM
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
#include "coreLib.h"

#define  OS_CPU_CFG_SYSTICK_PRIO           0u
#define  CPU_REG_NVIC_ST_CTRL_COUNTFLAG           0x00010000
#define  CPU_REG_NVIC_ST_CTRL_CLKSOURCE           0x00000004
#define  CPU_REG_NVIC_ST_CTRL_TICKINT             0x00000002
#define  CPU_REG_NVIC_ST_CTRL_ENABLE              0x00000001

#define  CPU_REG_NVIC_ST_CTRL        (*((cpureg_t *)(0xE000E010)))             /* SysTick Ctrl & Status Reg.           */
#define  CPU_REG_NVIC_ST_RELOAD      (*((cpureg_t *)(0xE000E014)))             /* SysTick Reload      Value Reg.       */
#define  CPU_REG_NVIC_SHPRI3         (*((cpureg_t *)(0xE000ED20)))             /* System Handlers 12 to 15 Prio.       */

#define  DEF_BIT_MASK(bit_mask, bit_shift)   ((bit_mask) << (bit_shift))
#define  DEF_INT_CPU_U_MAX_VAL               4294967295u
#define  DEF_INT_CPU_NBR_BITS                32
#define  DEF_BIT(bit)                        (1u << (bit))
#define  DEF_BIT_FIELD(bit_field, bit_shift) ((((bit_field) >= DEF_INT_CPU_NBR_BITS) ? (DEF_INT_CPU_U_MAX_VAL) \
                                             : (DEF_BIT(bit_field) - 1uL)) << (bit_shift))

void  OS_CPU_SysTickInit (cpureg_t  cnts);

LOCAL cpureg_t cpuTimerRate = CONFIG_HZ;
int sysClkRateGet(void)
{
    return cpuTimerRate;
}

extern cpudata_t  BSP_CPU_ClkFreq (void);
STATUS sysClkRateSet(int ticksPerSecond)
{
    cpureg_t  cnts;
    cpudata_t sysfreq = BSP_CPU_ClkFreq();

    cpuTimerRate = ticksPerSecond;
    cnts = sysfreq / cpuTimerRate;
    OS_CPU_SysTickInit (cnts);
    return 0;
}

void  OS_CPU_SysTickInit (cpureg_t  cnts)
{
    cpureg_t  prio;

    /* 填写 SysTick 的重载计数值 */
    CPU_REG_NVIC_ST_RELOAD = cnts - 1u;                     // SysTick 以该计数值为周期循环计数定时

    /* 设置 SysTick 中断优先级 */
    prio  = CPU_REG_NVIC_SHPRI3;
    prio &= DEF_BIT_FIELD(24, 0);
    prio |= DEF_BIT_MASK(OS_CPU_CFG_SYSTICK_PRIO, 24);      //设置为默认的最高优先级0，在裸机例程中该优先级默认为最低

    CPU_REG_NVIC_SHPRI3 = prio;

    /* 使能 SysTick 的时钟源和启动计数器 */
    CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_CLKSOURCE |
                            CPU_REG_NVIC_ST_CTRL_ENABLE;
    /* 使能 SysTick 的定时中断 */
    CPU_REG_NVIC_ST_CTRL |= CPU_REG_NVIC_ST_CTRL_TICKINT;
}


