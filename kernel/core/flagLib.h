/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : flagLib.h
 * 内容摘要   : 
 * 其它说明   : 
 * 版本       : 
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-06-19 10:41:50 AM
 * 
 * 修改记录1:
 *    修改日期: 2023-06-19
 *    版 本 号: 
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容: 
 * ===========================================================================
 */

#ifndef __FLAGLIB_H__
#define __FLAGLIB_H__

/* flags option for SEM_TYPE_FLAGS */
#define FLAG_OPT_SET_ALL     (0)
#define FLAG_OPT_CLR_ALL     (1)
#define FLAG_OPT_SET_ANY     (2)
#define FLAG_OPT_CLR_ANY     (3)
#define FLAG_OPT_SETCLR_MSK  (0x03)
#define FLAG_OPT_CONSUME     (1 << 4)


STATUS flagLibInit(void);
STATUS flagInit(SEM_ID semId, int options, UINT flags);
SEM_ID flagCreate(int options, UINT flags);
STATUS flagGive(SEM_ID semId, UINT flags, int flgOptions);
STATUS flagTake(SEM_ID semId, UINT flags, int flgOptions, int timeout);
STATUS flagFlush(SEM_ID id);

#endif /* #ifndef __FLAGLIB_H__ */


