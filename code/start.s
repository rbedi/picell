.section .text
.globl _start
_start:
    mov r0, #0xD2    @ IRQ mode
    msr cpsr_c, r0   @ Put in IRQ mode, don't clear C bits
    mov sp, #0x8000  @ Set IRQ stack pointer
    mov r0, #0xD3    @ SVC mode
    msr cpsr_c, r0   @ Put in SVC mode, don't clear C bits
    mov sp, #0x7000  @ Set SVC stack pointer
    bl _cstart       @ Jump to C start routine

.globl GETR0
GETR0:
    bx lr

.globl GETR3
GETR3:
    mov r0, r3
    bx lr


.globl SAVEREG
SAVEREG:
    str r1, [r0, #4]
    str r2, [r0, #8]
    str r4, [r0,#16]
    str r5, [r0,#20]
    str r6, [r0,#24]
    str r7, [r0,#28]
    str r8, [r0,#32]
    str r9, [r0,#36]
    str r10, [r0,#40]
    str r11, [r0,#44]
    str r12, [r0,#48]
    str r13, [r0,#52]
    str r14, [r0,#56]
    str r15, [r0,#60]

    bx lr


hang:
    b hang


