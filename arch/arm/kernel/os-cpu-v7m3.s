    PRESERVE8
    THUMB
    AREA    |.text|, CODE, READONLY
OS_CPU_PendSVHandler  PROC                                        ;// Modified by fire £¨Ô­ÊÇ PendSV Handler£©
        EXPORT  OS_CPU_PendSVHandler
        IMPORT  __osinfo__
        CPSID   I                                                   ; Prevent interruption during context switch
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
        CPSIE   I
        BX      LR                                                  ; Exception return will restore remaining context
    ENDP
END
