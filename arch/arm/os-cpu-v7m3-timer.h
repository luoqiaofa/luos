/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : os-cpu-v7m3-timer.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-25 01:36:56 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-05-25
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef __OS-CPU-V7M3-TIMER_H__
#define __OS-CPU-V7M3-TIMER_H__

IMPORT int sysClkRateGet (void);
IMPORT STATUS sysClkRateSet(int ticksPerSecond);

#endif /* #ifndef __OS-CPU-V7M3-TIMER_H__ */


