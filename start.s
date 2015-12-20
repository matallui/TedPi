.section ".text.startup"

.equ    CPSR_MODE_USER,         0x10
.equ    CPSR_MODE_FIQ,          0x11
.equ    CPSR_MODE_IRQ,          0x12
.equ    CPSR_MODE_SVR,          0x13
.equ    CPSR_MODE_ABORT,        0x17
.equ    CPSR_MODE_UNDEFINED,    0x1B
.equ    CPSR_MODE_SYSTEM,       0x1F

.equ    CPSR_IRQ_INHIBIT,       0x80
.equ    CPSR_FIQ_INHIBIT,       0x40
.equ    CPSR_THUMB,             0x20


.global _start
.global _inf_loop
.global _get_stack_pointer


_start:
    ldr     pc, _reset_vec
    ldr     pc, _undef_vec
    ldr     pc, _swi_vec
    ldr     pc, _prefetch_abort_vec
    ldr     pc, _data_abort_vec
    ldr     pc, _unused_vec
    ldr     pc, _irq_vec
    ldr     pc, _fiq_vec

_reset_vec:             .word   _reset
_undef_vec:             .word   undef_vector
_swi_vec:               .word   swi_vector
_prefetch_abort_vec:    .word   abort_vector
_data_abort_vec:        .word   abort_vector
_unused_vec:            .word   _reset
_irq_vec:               .word   irq_vector
_fiq_vec:               .word   fiq_vector


_reset:
    /* setup interrupt vector table */
    bl      _setup_irq_vec_table

    /* setup stack pointers */
    mov r0, #(CPSR_MODE_IRQ | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    mov sp, #(63 * 1024 * 1024)

    mov r0, #(CPSR_MODE_SVR | CPSR_IRQ_INHIBIT | CPSR_FIQ_INHIBIT )
    msr cpsr_c, r0
    mov sp, #(64 * (1024 * 1024))
    
    /* enable VFP */
    mrc p15, #0, r1, c1, c0, #2
    orr r1, r1, #(0xf << 20)
    mcr p15, #0, r1, c1, c0, #2
    mov r1, #0
    mcr p15, #0, r1, c7, c5, #4
    mov r0,#0x40000000
    fmxr fpexc, r0
    
    bl      _cstartup
    bl      _inf_loop

_setup_irq_vec_table:
    mov     r0, #0x0000
    ldr     r1, =_start
    ldmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    mov     pc, lr


.global _enable_interrupts
_enable_interrupts:
    mrs     r0, cpsr
    bic     r0, r0, #0x80
    msr     cpsr_c, r0
    mov     pc, lr


.global _disable_interrupts
_disable_interrupts:
    mrs     r0, cpsr
    orr     r0, #0x80
    msr     cpsr_c, r0
    mov     pc, lr
    

_get_stack_pointer:
    str     sp, [sp]
    ldr     r0, [sp]
    mov     pc, lr


_inf_loop:
    b       _inf_loop



