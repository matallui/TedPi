
#define TIMER_BASE 0x00203400 // errado

typedef struct {
    unsigned int load;
    unsigned int value;
    unsigned int control;
    unsigned int irq_clear;
    unsigned int raw_irq;
    unsigned int masked_irq;
    unsigned int reload;
    unsigned int predivider;
    unsigned int counter;
} timer_t;

