#define ASM
#include <board/armv7m.h>

	/* Global symbols */
	.globl	_start
	.type	_start, %function
	.globl	_vector_table_begin
	.globl	_vector_table_end
	.globl	_vector_table_size

	.extern _system_stack_top

    .extern _armv7m_exception_handler
    .extern _armv7m_interrupt_dispatch
	.extern _armv7m_clock_handler
	.extern _armv7m_crt_init
    .extern PendSV_Handler

	.section ".vectors", "ax"
	.syntax	  unified
	.thumb

_vector_table_begin:
	.word	_system_stack_top
	.word	_start /* Reset */
	.word	_armv7m_exception_handler /* NMI */
	.word	_armv7m_exception_handler /* Hard Fault */
	.word	_armv7m_exception_handler /* MPU Fault */
	.word	_armv7m_exception_handler /* Bus Fault */
	.word	_armv7m_exception_handler /* Usage Fault */
	.word	_armv7m_exception_handler /* Reserved */
	.word	_armv7m_exception_handler /* Reserved */
	.word	_armv7m_exception_handler /* Reserved */
	.word	_armv7m_exception_handler /* Reserved */
	.word	_armv7m_exception_handler /* SVC */
	.word	_armv7m_exception_handler /* Debug Monitor */
	.word	_armv7m_exception_handler /* Reserved */
	.word	PendSV_Handler /* PendSV */
	.word	_armv7m_clock_handler /* SysTick */
	.rept	ARMV7M_SOC_IRQCNT
	.word	_armv7m_interrupt_dispatch /* IRQ */
	.endr

_vector_table_end:
	.thumb_func

_start:

    /* Disable interrupt  */
    cpsid   i

	ldr r0, =_system_stack_top

	/*
	 * Set the Main Stack Pointer (MSP) to the ISR stack area end just in
	 * case we run using the Process Stack Pointer (PSP).
	 */
	msr	msp, r0
    bl  _armv7m_crt_init
    b  .

	.set	_vector_table_size, _vector_table_end - _vector_table_begin
