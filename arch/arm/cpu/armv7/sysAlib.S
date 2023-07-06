.extern __osinfo__
.global cpuIntLock
.global cpuContextSwitch
.global cpuIntUnlock
.global highestTaskStart
.type   cpuIntLock, %function
.type   cpuIntUnlock, %function
.type   cpuContextSwitch, %function
.type   highestTaskStart, %function

.align      4
/* FIQ中断 */
cpuIntLock: 
	mrs r0, cpsr
    cpsid if
	bx lr									

cpuIntUnlock: 
	msr cpsr,r0
	bx lr									

highestTaskStart: 
    dsb     ish
    clrex
    cps     #0x13                   /* 进入SVC模式，允许其他中断再次进去 */
    movw    r2, #:lower16:__osinfo__
    movt    r2, #:upper16:__osinfo__ 
    ldr     r0, [r2, #0] @currentTcb
    ldr     sp, [r0, #0x10]
    ldmfd   sp!, {r0}
	msr     SPSR_cxsf,r0
    ldmfd   sp!, {r0-r12, lr}
    ldmfd   sp!, {pc}^

cpuContextSwitch: 
    stmfd   sp!, {lr} @saved pc
    stmfd   sp!, {lr} @saved lr
    stmfd   sp!, {r0-r12}
	mrs     r0, cpsr
    stmfd   sp!, {r0}
    movw    r2, #:lower16:__osinfo__
    movt    r2, #:upper16:__osinfo__ 
    ldr     r0, [r2, #0] @currentTcb
    ldr     r1, [r2, #4] @highestTcb
    str     sp, [r0, #0x10] @currentTcb->stack
    str     r1, [r2, #0] @ currentTcb = highestTcb
    ldr     r0, [r2, #0] @ reload currentTcb
    ldr     sp, [r0, #0x10]
    ldmfd   sp!, {r0}
	msr     SPSR_cxsf,r0
    ldmfd   sp!, {r0-r12, lr,pc}^


