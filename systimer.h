#ifndef __SYSTIMER_H__
#define __SYSTIMER_H__

void systimer_irq_handler(unsigned int irq);
void alarm(unsigned int ms, void (*fn)(void));
void wait(unsigned int sec);
void wait_ms(unsigned int ms);


#endif /* __SYSTIMER_H__*/
