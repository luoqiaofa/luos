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
SEMAPHORE  semMuxprint;

static void *taskRtn2(void *arg);
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
int tmr_keep = 1;
uint32_t tmr_nsec_dly = 1;

static int tmr_loop = 0;

LOCAL int timer_expire(void *arg)
{
    cputime_t expires;

    if (tmr_keep & 0x02) {
        Printf("[%s]Enter..., loop=%d\n", __func__, tmr_loop++);
    }

    if (tmr_keep) {
        expires = sysClkTickGet();
        expires += tmr_nsec_dly * sysClkRateGet();
        timerModify((timerid_t)&timerL, expires);
    }
    return 0;
}

int timer_add_test(void)
{
    cputime_t expires = sysClkTickGet();

    expires += sysClkRateGet();
    timerInit((timerid_t)&timerL, expires, timer_expire, NULL);
    timerAdd((timerid_t)&timerL);
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

    while (1) {
        rc = msgQReceive(msgQId, buf, maxNBytes, /* WAIT_FOREVER */sysClkRateGet());
        if (rc > 0) {
            buf[rc] = '\0';
            Printf("[msgQReceive]len=%d, msg: %s\n", rc, buf);
            usage = cpuUsageGet();
            Printf("CPU Usage: %u.%03u%%\n", usage/1000, usage%1000);
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

extern void LED_Init ( void );
extern void ledToggle(void);
int dbg_print = 0;
static void *taskRtn1(void *arg)
{
    tid_t tid;
    TCB_ID tcb;
    int cnt = 0;
    char tname[20] = "tOne";

    timer_add_test();

    sysClkRateSet(CONFIG_HZ);

    flagInit(&semFlags, 0, 0);

    taskSpawn("t2",     10, 0, 4096, taskRtn2, semId);
    taskSpawn("tStat", CONFIG_NUM_PRIORITY-4, 0, 512,  taskStatus, NULL);
    taskSpawn("tMsgTx", 12, 0, 2048,  taskMsgTx, NULL);
    taskSpawn("tMsgRx", 11, 0, 2048,  taskMsgRx, NULL);

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
        if (dbg_print) {
		    Printf("[%s]cnt=%d\n", taskName(taskIdSelf()), cnt);
		}
        semGive(semId);
        taskDelay(sysClkRateGet());
    }
    return NULL;
}

extern void SysReset(void);
static void *taskRtn2(void *arg)
{
    char *cmdline;
    size_t len = 0;
    int cnt = 0;
    int retVal;

    sysSymTblInit();
    usrSymInit();
    while(true) {
        cnt++;
		// printf("[%s]cnt=%d\n", taskName(taskIdSelf()), cnt);
        cmdline = readline("luos->");
        if (NULL == cmdline) {
            continue;
        }
        len = strlen(cmdline);
        if (len > 0) {
            if (dbg_print) {
                Printf("len=%d,cmdline=%s\n", len, cmdline);
            }
            if (!strcmp("SysReset", cmdline)) {
                SysReset();
            }
            retVal = shellCmdlineProcess(cmdline);
            Printf("retVal=%d(0x%0x)\n", retVal, retVal);
        }
        // semTake(id, WAIT_FOREVER);
        // taskDelay(2);
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
    int rc;

    USARTx_Config();
	version();
    // cpu_sizeof();

    rc = coreLibInit();
    rc = memPartLibInit();
    rc = taskLibInit();
    semCLibInit();
    semMLibInit();
    semBLibInit();
    semMInit(&semMuxprint, 0);

    semId = semCCreate(SEM_Q_PRIORITY, 0);
    if (NULL == semId) {
        return -1;
    }

    msgQId = msgQCreate(10, 128, 0);

    if (OK != rc) {
        Printf("taskLibInit failed\n");
    }

    luosStart(taskRtn1, NULL, 4096);
    while (true) {
    }
    return 0;
}                /* ----------  end of function main  ---------- */


#ifndef CONFIG_SYS_PBSIZE
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + 128)
#endif

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int i;

	i = vsnprintf(buf, size, fmt, args);

	if (i < size)
		return i;
	if (size != 0)
		return size - 1;
	return 0;
}


int Printf(const char *fmt, ...)
{
	va_list args;
	uint32_t i;

	static char printbuffer[CONFIG_SYS_PBSIZE];

    semTake(&semMuxprint, WAIT_FOREVER);
	va_start(args, fmt);

	/*
	 * For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	printbuffer[0] = '\0';
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	/* Print the string */
	puts(printbuffer);
	semGive(&semMuxprint);
	return i;
}

int cpu_sizeof(void)
{
    printf(
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
    printf(
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

    printf("sizeof(\"abcd\")    =%u\n",(unsigned int)sizeof("abcd"));
    return 0;
}				/* ----------  end of function main  ---------- */
