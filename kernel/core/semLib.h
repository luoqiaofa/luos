/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : semLib.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-26 02:22:05 PM
 * 
 * 修改记录1:
 *    修改日期: 2023-05-26
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef __SEMLIB_H__
#define __SEMLIB_H__

#include "semLibP.h"

#define NO_WAIT         0
#define WAIT_FOREVER    (-1)
    

extern STATUS semGive (SEM_ID semId);
extern STATUS semTake (SEM_ID semId, int timeout);
extern STATUS semFlush (SEM_ID semId);
extern STATUS semDelete (SEM_ID semId);
extern STATUS semTerminate (SEM_ID semId);
extern STATUS semDestroy (SEM_ID semId, BOOL dealloc);
extern STATUS semGiveDefer (SEM_ID semId);
extern STATUS semFlushDefer (SEM_ID semId);
extern STATUS semInvalid (SEM_ID semId);
extern STATUS semInfo (SEM_ID semId, int idList[], int maxTasks);
extern STATUS semBLibInit (void);
extern SEM_ID semBCreate (int options, SEM_B_STATE initialState);
extern STATUS semCLibInit (void);
extern SEM_ID semCCreate (int options, int initialCount);
extern STATUS semMLibInit (void);
extern SEM_ID semMCreate (int options);
extern STATUS semMGiveForce (SEM_ID semId);
extern STATUS semOLibInit (void);
extern SEM_ID semCreate (void);
extern void   semShowInit (void);
extern STATUS semShow (SEM_ID semId, int level);

#endif /* #ifndef __SEMLIB_H__ */


