/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : os-cpu.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-07-06 10:00:38 AM
 * 
 * 修改记录1:
 *    修改日期: 2023-07-06
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef __OS_CPU_H__
#define __OS_CPU_H__

#include "os-cpu-types.h"


extern int cpuIntLock(void);
extern int cpuIntUnlock(int level);
extern int intLock(void);
extern int intUnlock(int level);
extern void  cpuContextSwitch(void);
int cpuSysTicksConfig(uint32_t ticks);
extern void highestTaskStart(void); 
void cpuTaskContextSwitchTrig(register void* cur, register void* tcb_high);

#endif /* #ifndef __OS_CPU_H__ */


