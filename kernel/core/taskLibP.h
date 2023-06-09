/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : taskLibP.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-30 09:40:41 AM
 * 
 * 修改记录1:
 *    修改日期: 2023-05-30
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef __TASKLIBP_H__
#define __TASKLIBP_H__
IMPORT STATUS taskQReadyPut(TCB_ID tcb);
IMPORT STATUS taskPendQuePut(LUOS_TCB *tcb, SEM_ID semId);
IMPORT STATUS taskPendQueGet(LUOS_TCB *tcb, SEM_ID semId);
IMPORT const char *taskStatusStr(TCB_ID tcb);

#endif /* #ifndef __TASKLIBP_H__ */


