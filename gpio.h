#ifndef __GPIO__
#define __GPIO__


/* GPIO Function Select */
typedef enum {
    GPFSEL_IN = 0,
    GPFSEL_OUT,
    GPFSEL_ALT5,
    GPFSEL_ALT4,
    GPFSEL_ALT0,
    GPFSEL_ALT1,
    GPFSEL_ALT2,
    GPFSEL_ALT3
} gpio_fsel_t;

/* GPIO Events */
typedef enum {
    GPEVT_NONE,
    GPEVT_RISING,
    GPEVT_FALLING,
    GPEVT_ASYNC_RISING,
    GPEVT_ASYNC_FALLING,
    GPEVT_HIGH,
    GPEVT_LOW
} gpio_evt_t;

/* GPIO PULL UP/DOWN*/
typedef enum {
    GPPULL_NONE = 0,
    GPPULL_DOWN,
    GPPULL_UP
} gpio_pull_t;


void gpio_init(unsigned int pin, gpio_fsel_t fn);
void gpio_set(unsigned int pin);
void gpio_clear(unsigned int pin);
int gpio_level(unsigned int pin);
int gpio_evt_status_check(unsigned int pin);
void gpio_evt_status_clear(unsigned int pin);
void gpio_evt_set(unsigned int pin, gpio_evt_t event);
void gpio_pull(unsigned int pin, gpio_pull_t pull);

#endif /* __GPIO__ */
