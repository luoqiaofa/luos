/*
 * ===========================================================================
 * 版权所有 (C)2010, MrLuo股份有限公司
 * 文件名称   : main.c
 * 内容摘要   :
 * 其它说明   :
 * 版本       :
 * 作    者   : Luoqiaofa (Luo), luoqiaofa@163.com
 * 创建时间   : 2023-05-30 04:36:11 PM
 *
 * 修改记录1:
 *    修改日期: 2023-05-30
 *    版 本 号:
 *    修 改 人: Luoqiaofa (Luo), luoqiaofa@163.com
 *    修改内容:
 * ===========================================================================
 */

/******************************************************************************
 *                #include (依次为标准库头文件、非标准库头文件)               *
 ******************************************************************************/
#include <stdio.h>
#include <stdarg.h>

#include "coreLib.h"

extern int usrSymInit(void);
extern int sysSymTblInit(void);
extern int shellCmdlineProcess(char *cmdline);
int Printf(const char *fmt, ...);
int cpu_sizeof(void);


extern int version(void);
LOCAL SEM_ID semId;

extern void USARTx_Config(void);
extern char * readline(const char *const prompt);

static SEM_ID semIds[9];
volatile UINT cpuStatusCnt = 0;
static void *taskStatus(void *arg) {
    // i();
    Printf("[%s] enter\n", taskName(taskIdSelf()));
    semTake(semIds[0], WAIT_FOREVER);
    Printf("[%s]wait someone task exit ok\n", taskName(taskIdSelf()));
    while (true) {
        cpuStatusCnt++;
        taskDelay(2);
    };
    return NULL;
}

static void *taskOneshort(void *arg)
{
    int nsec = (int)arg;

    nsec = (nsec <= 0 ? 1 : nsec);
    // Printf("[%s] enter!\n", taskName(taskIdSelf()));
    taskDelay(nsec * sysClkRateGet());
    // Printf("[%s] exit!\n", taskName(taskIdSelf()));
    return NULL;
}

LOCAL timerList_t timerL;
int dbg_print = 1;
uint32_t tmr_nsec_dly = 1;

static int tmr_loop = 0;

LOCAL int timer_expire(void *arg)
{
    cputime_t expires;

    if (dbg_print & BIT(1)) {
        Printf("[%s]Enter..., loop=%d\n", __func__, tmr_loop++);
    }

    if (dbg_print & BIT(0)) {
        expires = tmr_nsec_dly * sysClkRateGet();
        timerModify((timerid_t)&timerL, expires);
    }
    return 0;
}

int timer_add_test(void)
{
    ULONG ticksDefer;

    ticksDefer = sysClkRateGet();
    timerInit((timerid_t)&timerL, timer_expire, NULL);
    timerAdd((timerid_t)&timerL, ticksDefer);
    tmr_loop = 0;
    return 0;
}

LOCAL MSG_Q_ID msgQId = NULL;
static void *taskMsgRx(void *arg)
{
    UINT usage;
    int rc;
    char buf[101];
    UINT maxNBytes = 100;
    char *tname = NULL;

    char *pbuf = buf;

    while (1) {
        rc = msgQReceive(msgQId, &pbuf, maxNBytes, /* WAIT_FOREVER */sysClkRateGet());
        if (rc > 0) {
            buf[rc] = '\0';
            if (dbg_print & BIT(2)) {
                tname = taskName(taskIdSelf());
                Printf("[%s]msgQReceivelen=%d, msg: %s\n", tname, rc, pbuf);
                usage = cpuUsageGet();
                Printf("[%s]CPU Usage: %u.%03u%%\n", tname, usage/1000, usage%1000);
            }
        }
    }
    return NULL;
}

int nsec_freq = 2;
int msg_loop_cnt = 0;
static void *taskMsgTx(void *arg)
{
    int rc;
    char buf[101];
    UINT nBytes;

    while (1) {
        msg_loop_cnt++;
        nBytes = sprintf(buf, "message id=%d", msg_loop_cnt);
        buf[nBytes] = '\0';
        rc = msgQSend(msgQId, buf, nBytes, WAIT_FOREVER, 0);
        if (OK != rc) {
            Printf("rc=%d\n", rc);
        }
        taskDelay(nsec_freq * sysClkRateGet());
    }
    return NULL;
}

LOCAL MSG_Q_ID msgQId2 = NULL;
static void *taskMsgRx2(void *arg)
{
    int rc;
    char *buf;
    UINT maxNBytes = 100;

    while (1) {
        rc = msgQReceive(msgQId2, &buf, maxNBytes, WAIT_FOREVER);
        if (rc > 0) {
            buf[rc] = '\0';
            if (dbg_print & BIT(2)) {
                Printf("[%s]msgQReceivelen=%d, msg: %s\n", taskName(taskIdSelf()), rc, buf);
            }
        }
    }
    return NULL;
}

LOCAL char msgArray[3][128];
static void *taskMsgTx2(void *arg)
{
    int rc;
    char *buf;
    UINT nBytes;
    int loop = 0;

    while (1) {
        buf = msgArray[loop % ARRAY_SIZE(msgArray)];
        nBytes = sprintf(buf, "message id=%d", loop + 1);
        loop++;
        buf[nBytes] = '\0';
        rc = msgQSend(msgQId2, buf, nBytes, WAIT_FOREVER, 0);
        if (OK != rc) {
            Printf("rc=%d\n", rc);
        }
        taskDelay(nsec_freq * sysClkRateGet());
    }
    return NULL;
}

static SEMAPHORE semFlags;
static void *taskFlagsGive(void *arg)
{
    UINT idx = 0;
    UINT flags = ~0;
    while (true) {
        taskDelay(sysClkRateGet());
        flagGive(&semFlags, flags, FLAG_OPT_SET_ALL);
        idx++;
        if (0 == idx % (8 * sizeof(flags))) {
            flags = ~0;
        }
    }
    return NULL;
}

static void *taskFlagsTake(void *arg)
{
    UINT idx = 0;
    UINT flags = ~0;
    while (true) {
        flags = 1 << idx;
        flagTake(&semFlags, flags, FLAG_OPT_SET_ALL | FLAG_OPT_CONSUME, WAIT_FOREVER);
        idx = (idx + 1) % (8 * sizeof(flags));
    }
    return NULL;
}

extern int usrSymInit(void);
extern void LED_Init ( void );
extern void ledToggle(void);

static void *usrRoot(void *arg)
{
    int rc;
    tid_t tid;
    TCB_ID tcb;
    int cnt = 0;
    char tname[20] = "tOne";

    rc = sysHwInit();
    if (OK != rc) {
        Printf("sysHwInit failed, rc=%d\n", rc);
        return NULL;
    }
	version();
    // cpu_sizeof();

    usrSymInit();

    semId = semCCreate(SEM_Q_PRIORITY, 0);
    if (NULL == semId) {
        return NULL;
    }

    timer_add_test();

    sysClkRateSet(CONFIG_HZ);

    flagInit(&semFlags, 0, 0);

    taskSpawn("tStat", CONFIG_NUM_PRIORITY-4, 0, 512,  taskStatus, NULL);

    msgQId = msgQCreate(3, 128, 0);
    if (NULL != msgQId) {
        taskSpawn("tMsgTx", 12, 0, 2048,  taskMsgTx, NULL);
        taskSpawn("tMsgRx", 11, 0, 2048,  taskMsgRx, NULL);
    }

    msgQId2 = msgQCreate(3, 128, MSG_Q_ZERO_COPY);
    if (NULL != msgQId2) {
        taskSpawn("tMsgTx2", 12, 0, 1024,  taskMsgTx2, NULL);
        taskSpawn("tMsgRx2", 11, 0, 1024,  taskMsgRx2, NULL);
    }

    taskSpawn("tFlgGive", 17, 0, 1024,  taskFlagsGive, NULL);
    taskSpawn("tFlgTake", 18, 0, 1024,  taskFlagsTake, NULL);
#if 1
    for (cnt = 0; cnt < 9; cnt++) {
        tname[4] = '1' + cnt;
        tid = taskSpawn(tname,  CONFIG_NUM_PRIORITY-3, 0, 512,  taskOneshort, (void *)(13 - cnt));
        tcb = (TCB_ID)tid;
        semIds[cnt] = &(tcb->semJoinExit);
        if (tid == (tid_t)0) {
            Printf("task %s created failed\n", tname);
        }
    }
#endif
    cnt = 0;
	LED_Init();
    while(true) {
        cnt++;
        ledToggle();
        if (dbg_print & BIT(3)) {
		    Printf("[%s]cnt=%d\n", taskName(taskIdSelf()), cnt);
		}
        semGive(semId);
        taskDelay(sysClkRateGet());
    }
    return NULL;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:
 * =====================================================================================
 */
int main (int argc, char *argv[])
{
    luosStart(usrRoot, NULL, 4096);
    while (true) {;}
    return 0;
}                /* ----------  end of function main  ---------- */


int cpu_sizeof(void)
{
    Printf(
            "sizeof(char)      =%u\n"
            "sizeof(short)     =%u\n"
            "sizeof(int)       =%u\n"
            "sizeof(long)      =%u\n"
            "sizeof(float)     =%u\n"
            "sizeof(double)    =%u\n"
            "sizeof(void)      =%u\n"
            "sizeof(UINT)      =%u\n"
            "sizeof(ULONG)     =%u\n",
            sizeof(char)  ,
            sizeof(short) ,
            sizeof(int)  ,
            sizeof(long)  ,
            sizeof(float) ,
            sizeof(double),
            sizeof(void),
            sizeof(UINT),
            sizeof(ULONG)
          );
    Printf(
            "sizeof(char *)    =%d\n"
            "sizeof(short *)   =%d\n"
            "sizeof(int *)     =%d\n"
            "sizeof(long *)    =%d\n"
            "sizeof(float *)   =%d\n"
            "sizeof(double *)  =%d\n"
            "sizeof(void *)    =%d\n",
            (int)sizeof(char *)  ,
            (int)sizeof(short *) ,
            (int)sizeof(int *)  ,
            (int)sizeof(long *)  ,
            (int)sizeof(float *) ,
            (int)sizeof(double *) ,
            (int)sizeof(void *)
          );

    Printf("sizeof(\"abcd\")    =%u\n",(unsigned int)sizeof("abcd"));
    return 0;
}				/* ----------  end of function main  ---------- */
