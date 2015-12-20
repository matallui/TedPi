#ifndef __IRQ_H__
#define __IRQ_H__

extern void _enable_interrupts(void);
extern void _disable_interrupts(void);

void enable_interrupts(void);
void disable_interrupts(void);
void enable_interrupt(unsigned int n);
void disable_interrupt(unsigned int n);
int irq_pending(unsigned int n);

#endif /*__IRQ_H__*/

