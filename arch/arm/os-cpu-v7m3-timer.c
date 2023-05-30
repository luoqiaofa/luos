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

LOCAL int cpuTimerRate = CONFIG_HZ;
int sysClkRateGet(void)
{
    return cpuTimerRate;
}

STATUS sysClkRateSet(int ticksPerSecond)
{
    cpuTimerRate = ticksPerSecond;
    return 0;
}


