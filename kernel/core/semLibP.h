/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : semLibP.h
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-29 04:30:36 PM
 *
 * 修改记录1:
 *    修改日期: 2023-05-29
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

#ifndef __SEMLIBP_H__
#define __SEMLIBP_H__

#define SEM_TYPE_BINARY      0 /* binary semaphore */
#define SEM_TYPE_MUTEX       1 /* mutex semaphore */
#define SEM_TYPE_COUNT       2 /* counting semaphore */
#define SEM_TYPE_FLAGS       3 /* counting semaphore */
#define SEM_TYPE_MAX         4

#define SEM_Q_MASK               0x03   /*  q-type mask */
#define SEM_Q_FIFO               0x00   /*  first in first out queue */
#define SEM_Q_PRIORITY           0x01   /*  priority sorted queue */
#define SEM_DELETE_SAFE          0x04   /*  owner delete safe (mutex opt.) */
#define SEM_INVERSION_SAFE       0x08   /*  no priority inversion (mutex opt.) */
#define SEM_EVENTSEND_ERR_NOTIFY 0x10   /*  notify when eventRsrcSend fails */

#define  SEM_METUX_RECURSE_MAX   65535
typedef struct semaphore {
    UINT8    semType;    /* semaphore type */
    UINT8    options;    /* semaphore options */
    UINT16   recurse;    /* mutex recursive take count */
    int      oriPriority;/* origin owner priority */
    TLIST    qPendHead;  /* blocked task queue head */
    union {
        struct luosTcb  *ownerTcb;   /* origin owner tcb */
        int32_t  count;
        cpureg_t flags;    /* events flags */
    } obj;
} SEMAPHORE;

typedef SEMAPHORE* SEM_ID;

#define semCount  obj.count
#define semOwner  obj.ownerTcb
#define semEvents obj.flags

typedef enum        /*  SEM_TYPE_BINARY */
{
    SEM_EMPTY,      /*  0: semaphore not available */
    SEM_FULL        /*  1: semaphore available */
} SEM_B_STATE;


typedef int (*semGive_t)(SEM_ID id);
typedef int (*semTake_t)(SEM_ID id, int timeout);
typedef int (*semFlush_t)(SEM_ID id);
typedef int (*semGiveDefer_t)(SEM_ID id);
typedef int (*semFlushDefer_t)(SEM_ID id);
typedef struct sem_ops {
    semGive_t        psemGive;
    semTake_t        psemTake;
    semFlush_t       psemFlush;
    semGiveDefer_t   psemGiveDefer;
    semFlushDefer_t  psemFlushDefer;
} SEM_OPS;

STATUS semCGive(SEM_ID semId);
STATUS semCTake(SEM_ID semId, int timeout);
STATUS semFlush(SEM_ID id);
STATUS semGiveDefer(SEM_ID id);
STATUS semFlushDefer(SEM_ID id);

STATUS semQInit(SEM_ID id, int options);
STATUS semLibInit(void);
STATUS semMGive(SEM_ID semId);
STATUS semMTake(SEM_ID semId, int timeout);
STATUS semBGive(SEM_ID semId);
STATUS semBTake(SEM_ID semId, int timeout);
STATUS semTypeInit(int semtype, SEM_OPS *ops);

#endif /* #ifndef __SEMLIBP_H__ */


