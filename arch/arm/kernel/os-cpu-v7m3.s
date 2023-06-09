    PRESERVE8
    THUMB
    AREA    |.text|, CODE, READONLY

OS_CPU_SysTickHandler  PROC
    IMPORT  __osinfo__
    IMPORT  coreIntEnter
    IMPORT  coreIntExit
    IMPORT  tickAnnounce
    EXPORT  OS_CPU_SysTickHandler
    CPSID  I
    PUSH    {R4-R11,LR}
    LDR     R0, =coreIntEnter
    BLX     R0
    LDR     R2, =tickAnnounce
    BLX     R2
    LDR     R0, =coreIntExit
    BLX     R0
    LDR     R2, =__osinfo__
    LDR     R0, [R2]
    LDR     R1, [R2, #0x04]
    POP     {R4-R11,LR}
    CMP     R0, R1
    BNE     OS_CPU_PendSVHandler
    CPSIE   I
    BX      LR
    ENDP

OS_CPU_PendSVHandler  PROC                                        ;// Modified by fire £¨Ô­ÊÇ PendSV Handler£©
    EXPORT  OS_CPU_PendSVHandler
    IMPORT  __osinfo__
    IMPORT  coreContextHook
    CPSID   I                                                   ; Prevent interruption during context switch
    LDR     R2, =__osinfo__
    LDR     R0, [R2]
    LDR     R1, [R2, #0x04]
    MRS     R2, PSP                                             ; PSP is process stack pointer
    CBZ     R2, OS_CPU_PendSVHandler_nosave                     ; Skip register save the first time
    SUBS    R2, R2, #0x20                                       ; Save remaining regs r4-11 on process stack
    STM     R2, {R4-R11}
    ADDS    R3, R0, #0x10                                       ; Save SP to currentTask()->stack
    STR     R2, [R3]                                            ; R2 is SP of process being switched out
OS_CPU_PendSVHandler_nosave
    LDR     R2, =__osinfo__
    ADDS    R2, R2, #0x00                                       ; &osInfo->currentTcb
    STR     R1, [R2]
    ADDS    R2, R1, #0x10
    LDR     R2, [R2]                                            ; R0 is new process SP; SP = OSTCBHighRdyPtr->StkPtr;
    LDM     R2, {R4-R11}                                        ; Restore r4-11 from new process stack
    ADDS    R2, R2, #0x20
    MSR     PSP, R2                                             ; Load PSP with new process SP
    ORR     LR, LR, #0x04                                       ; Ensure exception return uses process stack
    PUSH    {LR}
    LDR     R0, =coreContextHook
    BLX     R0
    POP     {LR}
    CPSIE   I
    BX      LR                                                  ; Exception return will restore remaining context
    ENDP

cpuIntLock PROC
    EXPORT  cpuIntLock
    MRS     R0, PRIMASK
    CPSID   I
    BX  LR
    NOP
    ENDP

cpuIntUnlock PROC
    EXPORT cpuIntUnlock
    MSR PRIMASK, R0
    BX  LR
    ENDP

; USART1_IRQHandler PROC
;     EXPORT USART1_IRQHandler
;     IMPORT  __osinfo__
;     IMPORT  coreIntEnter
;     IMPORT  coreIntExit
;     IMPORT  UART_Receive
;     CPSID  I
;     PUSH    {R4-R11,LR}
;     LDR     R0, =coreIntEnter
;     BLX     R0
;     LDR     R2, =UART_Receive
;     BLX     R2
;     LDR     R0, =coreIntExit
;     BLX     R0
;     LDR     R2, =__osinfo__
;     LDR     R0, [R2]
;     LDR     R1, [R2, #0x04]
;     POP     {R4-R11,LR}
;     CMP     R0, R1
;     BNE     OS_CPU_PendSVHandler
;     CPSIE   I
;     BX      LR
;     ENDP

cpuRunningTaskStkGet PROC
    EXPORT cpuRunningTaskStkGet
    MRS R0, PSP
    BX  LR
    ENDP


END
