/***************************************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 start.s
作者	   : 左忠凯
版本	   : V1.0
描述	   : ZERO-I.MX6UL/I.MX6ULL开发板启动文件，完成C环境初始化，
		 C环境初始化完成以后跳转到C代码。
其他	   : 无
论坛 	   : www.wtmembed.com
日志	   : 初版V1.0 2019/1/3 左忠凯修改
		 V2.0 2019/1/4 左忠凯修改
		 添加中断相关定义
**************************************************************/
.equ ARM_EXCEPT_RESET,              0x00
.equ ARM_EXCEPT_UNDEF_INSTR,        0x01
.equ ARM_EXCEPT_SWI,                0x02
.equ ARM_EXCEPT_PREFETCH_ABORT,     0x03
.equ ARM_EXCEPT_DATA_ABORT,         0x04
.equ ARM_EXCEPT_ADDR_ABORT,         0x05
.equ ARM_EXCEPT_IRQ,                0x06
.equ ARM_EXCEPT_FIQ,                0x07

.equ CPU_ARM_CONTROL_INT_DIS,       0xC0                     @ Disable both FIQ and IRQ.
.equ CPU_ARM_CONTROL_FIQ_DIS,       0x40                     @ Disable FIQ.
.equ CPU_ARM_CONTROL_IRQ_DIS,       0x80                     @ Disable IRQ.
.equ CPU_ARM_CONTROL_THUMB,         0x20                     @ Set THUMB mode.
.equ CPU_ARM_CONTROL_ARM,           0x00                     @ Set ARM mode.

.equ CPU_ARM_MODE_MASK,             0x1F
.equ CPU_ARM_MODE_USR,              0x10
.equ CPU_ARM_MODE_FIQ,              0x11
.equ CPU_ARM_MODE_IRQ,              0x12
.equ CPU_ARM_MODE_SVC,              0x13
.equ CPU_ARM_MODE_ABT,              0x17
.equ CPU_ARM_MODE_UND,              0x1B
.equ CPU_ARM_MODE_SYS,              0x1F


.extern __osinfo__
.extern coreIntEnter
.extern coreIntExit
.extern system_irqhandler
.extern coreContextHook
.global _start  				/* 全局标号 */
.type _start, %function
.global Reset_Handler		/* 复位中断 					*/	
.global Undefined_Handler	/* 未定义中断 					*/
.global SVC_Handler		/* SVC(Supervisor)中断 		*/
.global PrefAbort_Handler	/* 预取终止中断 					*/
.global DataAbort_Handler	/* 数据终止中断 					*/
.global NotUsed_Handler	/* 未使用中断					*/
.global IRQ_Handler		/* IRQ中断 					*/
.global FIQ_Handler		/* FIQ(快速中断)未定义中断 			*/

.type Reset_Handler, %function		/* 复位中断 					*/	
.type Undefined_Handler, %function	/* 未定义中断 					*/
.type SVC_Handler, %function		/* SVC(Supervisor)中断 		*/
.type PrefAbort_Handler, %function	/* 预取终止中断 					*/
.type DataAbort_Handler, %function	/* 数据终止中断 					*/
.type NotUsed_Handler, %function	/* 未使用中断					*/
.type IRQ_Handler, %function		/* IRQ中断 					*/
.type FIQ_Handler, %function		/* FIQ(快速中断)未定义中断 			*/

/*
 * 描述：	_start函数，首先是中断向量表的创建
 * 参考文档:ARM Cortex-A(armV7)编程手册V4.0.pdf P42，3 ARM Processor Modes and Registers（ARM处理器模型和寄存器）
 * 		 	ARM Cortex-A(armV7)编程手册V4.0.pdf P165 11.1.1 Exception priorities(异常)
 */
_start:
	ldr pc, =Reset_Handler		/* 复位中断 					*/	
	ldr pc, =Undefined_Handler	/* 未定义中断 					*/
	ldr pc, =SVC_Handler		/* SVC(Supervisor)中断 		*/
	ldr pc, =PrefAbort_Handler	/* 预取终止中断 					*/
	ldr pc, =DataAbort_Handler	/* 数据终止中断 					*/
	ldr	pc, =NotUsed_Handler	/* 未使用中断					*/
	ldr pc, =IRQ_Handler		/* IRQ中断 					*/
	ldr pc, =FIQ_Handler		/* FIQ(快速中断)未定义中断 			*/

/* 复位中断 */	
Reset_Handler:

	cpsid i						/* 关闭全局中断 */

	/* 关闭I,DCache和MMU 
	 * 采取读-改-写的方式。
	 */
	mrc     p15, 0, r0, c1, c0, 0     /* 读取CP15的C1寄存器到R0中       		        	*/
    bic     r0,  r0, #(0x1 << 12)     /* 清除C1寄存器的bit12位(I位)，关闭I Cache            	*/
    bic     r0,  r0, #(0x1 <<  2)     /* 清除C1寄存器的bit2(C位)，关闭D Cache    				*/
    bic     r0,  r0, #0x2             /* 清除C1寄存器的bit1(A位)，关闭对齐						*/
    bic     r0,  r0, #(0x1 << 11)     /* 清除C1寄存器的bit11(Z位)，关闭分支预测					*/
    bic     r0,  r0, #0x1             /* 清除C1寄存器的bit0(M位)，关闭MMU				       	*/
    mcr     p15, 0, r0, c1, c0, 0     /* 将r0寄存器中的值写入到CP15的C1寄存器中	 				*/

	
#if 0
	/* 汇编版本设置中断向量表偏移 */
	ldr r0, =0X87800000

	dsb
	isb
	mcr p15, 0, r0, c12, c0, 0
	dsb
	isb
#endif
    
	/* 设置各个模式下的栈指针，
	 * 注意：IMX6UL的堆栈是向下增长的！
	 * 堆栈指针地址一定要是4字节地址对齐的！！！
	 * DDR范围:0X80000000~0X9FFFFFFF
	 */
	/* 进入IRQ模式 */
	mrs r0, cpsr
	bic r0, r0, #0x1f 	/* 将r0寄存器中的低5位清零，也就是cpsr的M0~M4 	*/
	orr r0, r0, #0x12 	/* r0或上0x13,表示使用IRQ模式					*/
	msr cpsr, r0		/* 将r0 的数据写入到cpsr_c中 					*/
	ldr sp, =0x80600000	/* 设置IRQ模式下的栈首地址为0X80600000,大小为2MB */

	/* 进入SYS模式 */
	mrs r0, cpsr
	bic r0, r0, #0x1f 	/* 将r0寄存器中的低5位清零，也就是cpsr的M0~M4 	*/
	orr r0, r0, #0x1f 	/* r0或上0x13,表示使用SYS模式					*/
	msr cpsr, r0		/* 将r0 的数据写入到cpsr_c中 					*/
	ldr sp, =0x80400000	/* 设置SYS模式下的栈首地址为0X80400000,大小为2MB */

	/* 进入SVC模式 */
	mrs r0, cpsr
	bic r0, r0, #0x1f 	/* 将r0寄存器中的低5位清零，也就是cpsr的M0~M4 	*/
	orr r0, r0, #0x13 	/* r0或上0x13,表示使用SVC模式					*/
	msr cpsr, r0		/* 将r0 的数据写入到cpsr_c中 					*/
	ldr sp, =0X80200000	/* 设置SVC模式下的栈首地址为0X80200000,大小为2MB */

	cpsie i				/* 打开全局中断 */
#if 0
	/* 使能IRQ中断 */
	mrs r0, cpsr		/* 读取cpsr寄存器值到r0中 			*/
	bic r0, r0, #0x80	/* 将r0寄存器中bit7清零，也就是CPSR中的I位清零，表示允许IRQ中断 */
	msr cpsr, r0		/* 将r0重新写入到cpsr中 			*/
#endif

	b main				/* 跳转到main函数 			 	*/

/* 未定义中断 */
Undefined_Handler:
	ldr r0, =Undefined_Handler
	bx r0

/* SVC中断 */
SVC_Handler:
    push   {r0-r12, lr}
    mrs    r0, spsr
    push   {r0}
    cps    0x12
    cps    0x13
    ldmfd  sp!, {r0}
    msr    SPSR_cxsf, r0
    pop    {r0-r12, lr}
    movs   pc, lr
    /* b      SVC_Handler */

/* 预取终止中断 */
PrefAbort_Handler:
	ldr r0, =PrefAbort_Handler	
	bx r0

/* 数据终止中断 */
DataAbort_Handler:
	ldr r0, =DataAbort_Handler
	bx r0

/* 未使用的中断 */
NotUsed_Handler:

	ldr r0, =NotUsed_Handler
	bx r0

/* IRQ中断！重点！！！！！ */
@ Register Usage:  R0     Exception Type
@                  R1     Exception's SPSR
@                  R2     Return PC
@                  R3     Exception's SP
IRQ_Handler:
    sub  lr, lr, #4
	push {r0-r12,lr}			/* 保存r0-r3，r12寄存器 */
    mov  r0, #CPU_ARM_MODE_IRQ
	mrs  r1, spsr				/* 读取spsr寄存器 */
    push {r1}
    mov  r2, lr
    mov  r3, sp

	msr  cpsr, #(CPU_ARM_CONTROL_INT_DIS | CPU_ARM_MODE_SVC)
    clrex
    push {r2}
    push {r4-r12, lr}
    ldmfd r3!, {r4-r8} /* r3-r0, spsr to svc sp */
    push  {r4-r8}

    ldr r2, =coreIntEnter
    blx r2

	ldr r2, =system_irqhandler	/* 加载C语言中断处理函数到r2寄存器中*/
	blx r2						/* 运行C语言中断处理函数，带有一个参数，保存在R0寄存器中 */

    ldr r2, =coreIntExit
    blx r2
    
    movw r2, #:lower16:__osinfo__ 
    movt r2, #:upper16:__osinfo__ 
    ldr  r0, [r2, #0] @currentTcb
    ldr  r1, [r2, #4] @highestTcb
    cmp  r0, r1
    bne  IntContextSwitchSave
    pop  {r0}
	msr  SPSR_cxsf, r0			/* 恢复spsr */
    pop  {r0-r12,lr}
    pop  {r0} /* discard pc */
switch_to_irq_mode: 
	msr  cpsr, #(CPU_ARM_CONTROL_INT_DIS | CPU_ARM_MODE_IRQ)
    clrex
	pop {r0}						
	msr SPSR_cxsf, r0			/* 恢复spsr */
	pop {r0-r12}			    /* r0-r3,r12出栈 */
	ldmfd sp!, {pc}^			/* lr出栈 */

IntContextSwitchSave :
    str     sp, [r0, #0x10]     /* save currentTcb->stack */
IntContextSwitch:
    str     r1, [r2, #0] @ currentTcb = highestTcb
    ldr     r0, [r2, #0] @ reload currentTcb
    ldr     sp, [r0, #0x10] /* load currentTcb->stack */

    push    {r0-r7}
    ldr     r0, =coreContextHook
    blx     r0
    pop     {r0-r7}

    ldmfd   sp!, {r0}
	msr     SPSR_cxsf,r0
    ldmfd   sp!, {r0-r12, lr}
    ldmfd   sp!, {pc}^

/* FIQ中断 */
FIQ_Handler:

	ldr r0, =FIQ_Handler	
	bx r0									

