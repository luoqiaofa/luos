/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : os-cpu-types.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-07-05 06:03:44 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-07-05
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef __OS_CPU_TYPES_H__
#define __OS_CPU_TYPES_H__
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t tid_t;
typedef uint32_t cpureg_t;
typedef uint32_t cpudata_t;
typedef uint32_t cputime_t;

typedef struct armv7_stk {
    cpureg_t SPSR;
    cpureg_t R0;
    cpureg_t R1;
    cpureg_t R2;
    cpureg_t R3;
    cpureg_t R4;
    cpureg_t R5;
    cpureg_t R6;
    cpureg_t R7;
    cpureg_t R8;
    cpureg_t R9;
    cpureg_t R10;
    cpureg_t R11;
    cpureg_t R12;
    /* cpureg_t R13;*/ /* is SP */
    cpureg_t LR; /* R14 */
    cpureg_t PC; /* R15 */
} STK_REGS;

typedef struct armv7_regs {
    STK_REGS stkRegs;
    cpureg_t spsr;
} REG_SET;

#endif /* #ifndef __OS_CPU_TYPES_H__ */


