#include "gpio.h"
#include "systimer.h"
#include "led.h"

#if (RPI2 || RPI0)
#define STATUS_LED  47
#else
#define STATUS_LED  16
#endif

static unsigned int enabled = 0;
static unsigned int on = 0;
static unsigned int blink_ms = 1000;
static unsigned int blink_set = 0;

static void led_init(void)
{
    gpio_init(STATUS_LED, GPFSEL_OUT);
    enabled = 1;
}

void led_on(void)
{
    if (!enabled)
        led_init();
    gpio_set(STATUS_LED);
    on = 1;
}

void led_off(void)
{
    if (!enabled)
        led_init();
    gpio_clear(STATUS_LED);
    on = 0;
}

void led_callback(void)
{
    if (!blink_set)
        return;

    if (on)
        led_off();
    else
        led_on();
    
    alarm(blink_ms, led_callback);
} 

void led_blink(int ms)
{
    blink_ms = ms;
    if (!blink_set) {
        blink_set = 1;
        alarm(blink_ms, led_callback);
    }
}

void led_blink_disable(void)
{
    blink_set = 0;
}

