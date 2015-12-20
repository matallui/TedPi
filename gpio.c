#include "gpio.h"
#include "kernel.h"

#define GPIO_BASE 0x00200000UL

typedef struct gpio_t {
    unsigned int fsel[6];
    unsigned int reserved0;
    unsigned int set[2];
    unsigned int reserved1;
    unsigned int clear[2];
    unsigned int reserved2;
    unsigned int level[2];
    unsigned int reserved3;
    unsigned int event_status[2];
    unsigned int reserved4;
    unsigned int rising_detect[2];
    unsigned int reserved5;
    unsigned int falling_detect[2];
    unsigned int reserved6;
    unsigned int high_detect[2];
    unsigned int reserved7;
    unsigned int low_detect[2];
    unsigned int reserved8;
    unsigned int async_rising_detect[2];
    unsigned int reserved9;
    unsigned int async_falling_detect[2];
    unsigned int reserved10;
    unsigned int pud_enable;
    unsigned int pud_clock[2];
} gpio_t;

volatile gpio_t *gpio = (gpio_t*) (GPIO_BASE + REG_BASE);

void gpio_init(unsigned int pin, gpio_fsel_t fn)
{
    int bank;

    if ((pin > 53) || (fn > 7))
        return;

    bank = pin / 10;
    pin = (pin % 10) * 3;

    gpio->fsel[bank] &= ~(7 << pin);
    gpio->fsel[bank] |= fn << pin;
}

void gpio_set(unsigned int pin)
{
    if (pin <= 53)
        gpio->set[(pin >> 5)] = 1 << (pin & 0x1F);
}

void gpio_clear(unsigned int pin)
{
    if (pin <= 53)
        gpio->clear[(pin >> 5)] = 1 << (pin & 0x1F);
}

int gpio_level(unsigned int pin)
{
    if (pin > 53)
        return -1;

    return gpio->level[(pin >> 5)] & (1 << (pin & 0x1F));
}

int gpio_evt_status_check(unsigned int pin)
{
    if (pin > 53)
        return 0;

    return gpio->event_status[(pin >> 5)] & (1 << (pin & 0x1F));
}

void gpio_evt_status_clear(unsigned int pin)
{
    if (pin <= 53)
        gpio->event_status[(pin >> 5)] |= (1 << (pin & 0x1F));
}

void gpio_evt_set(unsigned int pin, gpio_evt_t event)
{
    unsigned int bank;

    if (pin > 53)
        return;

    bank = (pin >> 5);
    pin = (1 << (pin & 0x1F));

    switch(event) {
        case GPEVT_RISING:
            gpio->rising_detect[bank] |= pin;
            break;
        case GPEVT_FALLING:
            gpio->falling_detect[bank] |= pin;
            break;
        case GPEVT_ASYNC_RISING:
            gpio->async_rising_detect[bank] |= pin;
            break;
        case GPEVT_ASYNC_FALLING:
            gpio->async_falling_detect[bank] |= pin;
            break;
        case GPEVT_HIGH:
            gpio->high_detect[bank] |= pin;
            break;
        case GPEVT_LOW:
            gpio->low_detect[bank] |= pin;
            break;
        case GPEVT_NONE:
            gpio->rising_detect[bank] &= ~pin;
            gpio->falling_detect[bank] &= ~pin;
            gpio->async_rising_detect[bank] &= ~pin;
            gpio->async_falling_detect[bank] &= ~pin;
            gpio->high_detect[bank] &= ~pin;
            gpio->low_detect[bank] &= ~pin;
            break;
    }
}

void gpio_pull(unsigned int pin, gpio_pull_t pull)
{
    int i;

    if (pin > 53)
        return;

    gpio->pud_enable = pull;
    for (i = 0; i < 150; i++);
    gpio->pud_clock[(pin >> 5)] = 1 << (pin & 0x1F);
    for (i = 0; i < 150; i++);
    gpio->pud_enable = 0;
    gpio->pud_clock[0] = 0;
    gpio->pud_clock[1] = 0;
}

