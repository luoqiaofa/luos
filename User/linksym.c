/* This file is generated by specific tool, please don't modify it */
#include "libSym.h"

extern int console_buffer[];
extern int semMuxprint[];
extern int cpuStatusCnt[];
extern int dbg_print[];
extern int msg_loop_cnt[];
extern int nsec_freq[];
extern int numTocksQWork[];
extern int tickQWorkRdIdx[];
extern int tickQworkWrIdx[];
extern int tmr_keep[];
extern int tmr_nsec_dly[];
extern void * BSP_CPU_ClkFreq(void);
extern void * cli_readline(void);
extern void * cli_readline_into_buffer(void);
extern void * coreContextHook(void);
extern void * coreIntEnter(void);
extern void * coreIntExit(void);
extern void * coreLibInit(void);
extern void * coreTickDoing(void);
extern void * coreTrySchedule(void);
extern void * cpuCntLeadZeros(void);
extern void * cpuIntLock(void);
extern void * cpuIntUnlock(void);
extern void * cpuRunningTaskStkGet(void);
extern void * cpuStackInit(void);
extern void * cpuSysTicksConfig(void);
extern void * cpuTaskContextSwitchTrig(void);
extern void * getc(void);
extern void * GPIO_Init(void);
extern void * GPIO_ReadOutputDataBit(void);
extern void * GPIO_ResetBits(void);
extern void * GPIO_SetBits(void);
extern void * i(void);
extern void * intLock(void);
extern void * intUnlock(void);
extern void * isValidNumber(void);
extern void * ledToggle(void);
extern void * LED_Init(void);
extern void * luosStart(void);
extern void * memPartAlloc(void);
extern void * memPartFree(void);
extern void * memPartInit(void);
extern void * memPartLibInit(void);
extern void * msgQCreate(void);
extern void * msgQInit(void);
extern void * msgQReceive(void);
extern void * msgQSend(void);
extern void * NVIC_Init(void);
extern void * osMemAlloc(void);
extern void * osMemFree(void);
extern void * Printf(void);
extern void * putc(void);
extern void * puts(void);
extern void * RCC_APB2PeriphClockCmd(void);
extern void * RCC_GetClocksFreq(void);
extern void * readline(void);
extern void * semBGive(void);
extern void * semBLibInit(void);
extern void * semBTake(void);
extern void * semCCreate(void);
extern void * semCGive(void);
extern void * semCInit(void);
extern void * semCLibInit(void);
extern void * semCTake(void);
extern void * semFlush(void);
extern void * semFlushDefer(void);
extern void * semGive(void);
extern void * semGiveDefer(void);
extern void * semInvalid(void);
extern void * semLibInit(void);
extern void * semMGive(void);
extern void * semMInit(void);
extern void * semMLibInit(void);
extern void * semMTake(void);
extern void * semQInit(void);
extern void * semTake(void);
extern void * semTypeInit(void);
extern void * sprintf(void);
extern void * strStrip(void);
extern void * strToInt(void);
extern void * sysClkRateGet(void);
extern void * sysClkRateSet(void);
extern void * sysClkTickGet(void);
extern void * SysReset(void);
extern void * SystemInit(void);
extern void * taskActivate(void);
extern void * taskCreate(void);
extern void * taskDelay(void);
extern void * taskDelete(void);
extern void * taskIdSelf(void);
extern void * taskInit(void);
extern void * taskLibInit(void);
extern void * taskLock(void);
extern void * taskName(void);
extern void * taskPendQueGet(void);
extern void * taskPendQuePut(void);
extern void * taskPrioritySet(void);
extern void * taskResume(void);
extern void * taskSpawn(void);
extern void * taskStatusStr(void);
extern void * taskSuspend(void);
extern void * taskUnlock(void);
extern void * tickAnnounce(void);
extern void * tickQWorkDoing(void);
extern void * timerAdd(void);
extern void * timerInit(void);
extern void * timerLibInit(void);
extern void * timerListDing(void);
extern void * timerModify(void);
extern void * timer_add_test(void);
extern void * tolower(void);
extern void * tstc(void);
extern void * UART_Receive(void);
extern void * USARTx_Config(void);
extern void * USART_ClearITPendingBit(void);
extern void * USART_Cmd(void);
extern void * USART_GetFlagStatus(void);
extern void * USART_GetITStatus(void);
extern void * USART_Init(void);
extern void * USART_ITConfig(void);
extern void * USART_ReceiveData(void);
extern void * USART_SendData(void);
extern void * version(void);
extern void * vscnprintf(void);

static const TsymPara g_symTbl[] =
{
    {"BSP_CPU_ClkFreq"               , SYM_TYPE_T, BSP_CPU_ClkFreq},
    {"GPIO_Init"                     , SYM_TYPE_T, GPIO_Init},
    {"GPIO_ReadOutputDataBit"        , SYM_TYPE_T, GPIO_ReadOutputDataBit},
    {"GPIO_ResetBits"                , SYM_TYPE_T, GPIO_ResetBits},
    {"GPIO_SetBits"                  , SYM_TYPE_T, GPIO_SetBits},
    {"LED_Init"                      , SYM_TYPE_T, LED_Init},
    {"NVIC_Init"                     , SYM_TYPE_T, NVIC_Init},
    {"Printf"                        , SYM_TYPE_T, Printf},
    {"RCC_APB2PeriphClockCmd"        , SYM_TYPE_T, RCC_APB2PeriphClockCmd},
    {"RCC_GetClocksFreq"             , SYM_TYPE_T, RCC_GetClocksFreq},
    {"SysReset"                      , SYM_TYPE_T, SysReset},
    {"SystemInit"                    , SYM_TYPE_T, SystemInit},
    {"UART_Receive"                  , SYM_TYPE_T, UART_Receive},
    {"USART_ClearITPendingBit"       , SYM_TYPE_T, USART_ClearITPendingBit},
    {"USART_Cmd"                     , SYM_TYPE_T, USART_Cmd},
    {"USART_GetFlagStatus"           , SYM_TYPE_T, USART_GetFlagStatus},
    {"USART_GetITStatus"             , SYM_TYPE_T, USART_GetITStatus},
    {"USART_ITConfig"                , SYM_TYPE_T, USART_ITConfig},
    {"USART_Init"                    , SYM_TYPE_T, USART_Init},
    {"USART_ReceiveData"             , SYM_TYPE_T, USART_ReceiveData},
    {"USART_SendData"                , SYM_TYPE_T, USART_SendData},
    {"USARTx_Config"                 , SYM_TYPE_T, USARTx_Config},
    {"cli_readline"                  , SYM_TYPE_T, cli_readline},
    {"cli_readline_into_buffer"      , SYM_TYPE_T, cli_readline_into_buffer},
    {"console_buffer"                , SYM_TYPE_B, console_buffer},
    {"coreContextHook"               , SYM_TYPE_T, coreContextHook},
    {"coreIntEnter"                  , SYM_TYPE_T, coreIntEnter},
    {"coreIntExit"                   , SYM_TYPE_T, coreIntExit},
    {"coreLibInit"                   , SYM_TYPE_T, coreLibInit},
    {"coreTickDoing"                 , SYM_TYPE_T, coreTickDoing},
    {"coreTrySchedule"               , SYM_TYPE_T, coreTrySchedule},
    {"cpuCntLeadZeros"               , SYM_TYPE_T, cpuCntLeadZeros},
    {"cpuIntLock"                    , SYM_TYPE_T, cpuIntLock},
    {"cpuIntUnlock"                  , SYM_TYPE_T, cpuIntUnlock},
    {"cpuRunningTaskStkGet"          , SYM_TYPE_T, cpuRunningTaskStkGet},
    {"cpuStackInit"                  , SYM_TYPE_T, cpuStackInit},
    {"cpuStatusCnt"                  , SYM_TYPE_D, cpuStatusCnt},
    {"cpuSysTicksConfig"             , SYM_TYPE_T, cpuSysTicksConfig},
    {"cpuTaskContextSwitchTrig"      , SYM_TYPE_T, cpuTaskContextSwitchTrig},
    {"dbg_print"                     , SYM_TYPE_D, dbg_print},
    {"getc"                          , SYM_TYPE_T, getc},
    {"i"                             , SYM_TYPE_T, i},
    {"intLock"                       , SYM_TYPE_T, intLock},
    {"intUnlock"                     , SYM_TYPE_T, intUnlock},
    {"isValidNumber"                 , SYM_TYPE_T, isValidNumber},
    {"ledToggle"                     , SYM_TYPE_T, ledToggle},
    {"luosStart"                     , SYM_TYPE_T, luosStart},
    {"memPartAlloc"                  , SYM_TYPE_T, memPartAlloc},
    {"memPartFree"                   , SYM_TYPE_T, memPartFree},
    {"memPartInit"                   , SYM_TYPE_T, memPartInit},
    {"memPartLibInit"                , SYM_TYPE_T, memPartLibInit},
    {"msgQCreate"                    , SYM_TYPE_T, msgQCreate},
    {"msgQInit"                      , SYM_TYPE_T, msgQInit},
    {"msgQReceive"                   , SYM_TYPE_T, msgQReceive},
    {"msgQSend"                      , SYM_TYPE_T, msgQSend},
    {"msg_loop_cnt"                  , SYM_TYPE_D, msg_loop_cnt},
    {"nsec_freq"                     , SYM_TYPE_D, nsec_freq},
    {"numTocksQWork"                 , SYM_TYPE_D, numTocksQWork},
    {"osMemAlloc"                    , SYM_TYPE_T, osMemAlloc},
    {"osMemFree"                     , SYM_TYPE_T, osMemFree},
    {"putc"                          , SYM_TYPE_T, putc},
    {"puts"                          , SYM_TYPE_T, puts},
    {"readline"                      , SYM_TYPE_T, readline},
    {"semBGive"                      , SYM_TYPE_T, semBGive},
    {"semBLibInit"                   , SYM_TYPE_T, semBLibInit},
    {"semBTake"                      , SYM_TYPE_T, semBTake},
    {"semCCreate"                    , SYM_TYPE_T, semCCreate},
    {"semCGive"                      , SYM_TYPE_T, semCGive},
    {"semCInit"                      , SYM_TYPE_T, semCInit},
    {"semCLibInit"                   , SYM_TYPE_T, semCLibInit},
    {"semCTake"                      , SYM_TYPE_T, semCTake},
    {"semFlush"                      , SYM_TYPE_T, semFlush},
    {"semFlushDefer"                 , SYM_TYPE_T, semFlushDefer},
    {"semGive"                       , SYM_TYPE_T, semGive},
    {"semGiveDefer"                  , SYM_TYPE_T, semGiveDefer},
    {"semInvalid"                    , SYM_TYPE_T, semInvalid},
    {"semLibInit"                    , SYM_TYPE_T, semLibInit},
    {"semMGive"                      , SYM_TYPE_T, semMGive},
    {"semMInit"                      , SYM_TYPE_T, semMInit},
    {"semMLibInit"                   , SYM_TYPE_T, semMLibInit},
    {"semMTake"                      , SYM_TYPE_T, semMTake},
    {"semMuxprint"                   , SYM_TYPE_B, semMuxprint},
    {"semQInit"                      , SYM_TYPE_T, semQInit},
    {"semTake"                       , SYM_TYPE_T, semTake},
    {"semTypeInit"                   , SYM_TYPE_T, semTypeInit},
    {"sprintf"                       , SYM_TYPE_T, sprintf},
    {"strStrip"                      , SYM_TYPE_T, strStrip},
    {"strToInt"                      , SYM_TYPE_T, strToInt},
    {"sysClkRateGet"                 , SYM_TYPE_T, sysClkRateGet},
    {"sysClkRateSet"                 , SYM_TYPE_T, sysClkRateSet},
    {"sysClkTickGet"                 , SYM_TYPE_T, sysClkTickGet},
    {"taskActivate"                  , SYM_TYPE_T, taskActivate},
    {"taskCreate"                    , SYM_TYPE_T, taskCreate},
    {"taskDelay"                     , SYM_TYPE_T, taskDelay},
    {"taskDelete"                    , SYM_TYPE_T, taskDelete},
    {"taskIdSelf"                    , SYM_TYPE_T, taskIdSelf},
    {"taskInit"                      , SYM_TYPE_T, taskInit},
    {"taskLibInit"                   , SYM_TYPE_T, taskLibInit},
    {"taskLock"                      , SYM_TYPE_T, taskLock},
    {"taskName"                      , SYM_TYPE_T, taskName},
    {"taskPendQueGet"                , SYM_TYPE_T, taskPendQueGet},
    {"taskPendQuePut"                , SYM_TYPE_T, taskPendQuePut},
    {"taskPrioritySet"               , SYM_TYPE_T, taskPrioritySet},
    {"taskResume"                    , SYM_TYPE_T, taskResume},
    {"taskSpawn"                     , SYM_TYPE_T, taskSpawn},
    {"taskStatusStr"                 , SYM_TYPE_T, taskStatusStr},
    {"taskSuspend"                   , SYM_TYPE_T, taskSuspend},
    {"taskUnlock"                    , SYM_TYPE_T, taskUnlock},
    {"tickAnnounce"                  , SYM_TYPE_T, tickAnnounce},
    {"tickQWorkDoing"                , SYM_TYPE_T, tickQWorkDoing},
    {"tickQWorkRdIdx"                , SYM_TYPE_D, tickQWorkRdIdx},
    {"tickQworkWrIdx"                , SYM_TYPE_D, tickQworkWrIdx},
    {"timerAdd"                      , SYM_TYPE_T, timerAdd},
    {"timerInit"                     , SYM_TYPE_T, timerInit},
    {"timerLibInit"                  , SYM_TYPE_T, timerLibInit},
    {"timerListDing"                 , SYM_TYPE_T, timerListDing},
    {"timerModify"                   , SYM_TYPE_T, timerModify},
    {"timer_add_test"                , SYM_TYPE_T, timer_add_test},
    {"tmr_keep"                      , SYM_TYPE_D, tmr_keep},
    {"tmr_nsec_dly"                  , SYM_TYPE_D, tmr_nsec_dly},
    {"tolower"                       , SYM_TYPE_T, tolower},
    {"tstc"                          , SYM_TYPE_T, tstc},
    {"version"                       , SYM_TYPE_T, version},
    {"vscnprintf"                    , SYM_TYPE_T, vscnprintf},
};


int usrSymInit(void)
{
    return sysSymTblAdd(g_symTbl, ARRAY_SIZE(g_symTbl));
}
