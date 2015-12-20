#include "clib.h"
#include "kernel.h"
#include "systimer.h"
#include "led.h"
#include "irq.h"

#define IRQ_BASE      0x0000B200

typedef struct irq {
    unsigned int basic_pending;
    unsigned int irq_pending[2];
    unsigned int fiq_ctrl;
    unsigned int irq_enable[2];
    unsigned int basic_enable;
    unsigned int irq_disable[2];
    unsigned int basic_disable;
} irq_t;

volatile irq_t *irq = (irq_t*)(REG_BASE + IRQ_BASE);


void __attribute__((interrupt("SWI"))) swi_vector(void)
{
    printf("* SWI Interrupt!\n"); 
    while(1);
}

void __attribute__((interrupt("UNDEF"))) undef_vector(void)
{
    printf("* UNDEF Interrupt!\n"); 
    while(1);
}

void __attribute__((interrupt("ABORT"))) abort_vector(void)
{
    printf("* ABORT Interrupt!\n"); 
    while(1);
}


void __attribute__((interrupt("IRQ"))) irq_vector(void)
{
    unsigned int basic_pending, pending_lo;
    basic_pending = irq->basic_pending;

    if (basic_pending & 0x100) {
        pending_lo = irq->irq_pending[0];
        if (pending_lo & 0x0000000F)
            systimer_irq_handler(pending_lo & 0x0F);
        if (pending_lo & ~0x0000000F)
            printf("irq: unknown low irq detected!\n");
    } else {
        printf("irq: unknown high irq detected!\n");
    }
}

void __attribute__((interrupt("FIQ"))) fiq_vector(void)
{
    printf("* FIQ Interrupt!\n");
    while(1);
}

void enable_interrupts(void)
{
    _enable_interrupts();
}

void disable_interrupts(void)
{
    _disable_interrupts();
}

void enable_interrupt(unsigned int n)
{
    if (n > 63)
        return;

    irq->irq_enable[n >> 5] = 1 << (n & 0x1F);
}

void disable_interrupt(unsigned int n)
{
    if (n > 63)
        return;

    irq->irq_disable[n >> 5] = 1 << (n & 0x1F);
}

int irq_pending(unsigned int n)
{
    if (n > 63)
        return 0;

    if (irq->irq_pending[n >> 5] & (1 << (n & 0x1F)))
        return 1;

    return 0;
}
