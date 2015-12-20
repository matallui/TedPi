#include "clib.h"
#include "kernel.h"
#include "irq.h"
#include "systimer.h"

#define SYSTIMER_BASE   0x00003000

#define SYSTIMER_HZ     1000000
#define IRQ_GP          3
#define IRQ_SCHED       1

typedef struct {
    unsigned int status;
    unsigned int counter_lo;
    unsigned int counter_hi;
    unsigned int compare[4]; /* 1,3 -> CPU; 0,2 -> GPU */
} systimer_t;

volatile systimer_t *systimer = (systimer_t*)(REG_BASE + SYSTIMER_BASE);

static unsigned int alarm_set = 0;
static void (*callback)(void) = NULL;



void systimer_irq_handler(unsigned int irq)
{
    switch (irq) {
        case 0x01:
        case 0x04:
            printf("systimer: GPU irq detected. No action taken.\n");
            break;
        case 0x02:
            /* TODO: scheduler tick */
            systimer->status = 0x02;
            printf("systimer: scheduler tick!!!\n");
            break;
        case 0x08:
            systimer->status = 0x08;
            disable_interrupt(IRQ_GP);
            alarm_set = 0;
            callback(); 
            break;
        default:
            printf("systimer: unknown irq detected!\n");
    
    }
}


void alarm(unsigned int ms, void (*fn)(void))
{
    unsigned int counter = systimer->counter_lo;
    unsigned int target = counter + (ms * 1000);
    
    if (alarm_set || fn == NULL)
        return;

    alarm_set = 1;
    callback = fn;
    systimer->compare[IRQ_GP] = target;
    enable_interrupt(IRQ_GP);
}

void wait_ms(unsigned int ms)
{
    unsigned long long counter = (unsigned long long)systimer->counter_lo | 
                            ((unsigned long long)systimer->counter_hi << 32);
    unsigned long long target = counter + ms * 1000;

    while (counter < target) {
        counter = (unsigned long long )systimer->counter_lo | 
                ((unsigned long long)systimer->counter_hi << 32);
    }
    
}

void wait(unsigned int sec)
{
    wait_ms(sec * 1000);
}

