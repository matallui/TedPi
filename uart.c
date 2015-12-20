#include "uart.h"
#include "kernel.h"
#include "gpio.h"

volatile uart_t* uart = (uart_t*) (REG_BASE + UART_BASE);

static unsigned int enabled = 0;

static unsigned int busy(void);
static void reset(void);
static unsigned int txfull(void);
static unsigned int rxempty(void);
static unsigned int cts(void);

void uart_enable(unsigned int baud, unsigned int wlen)
{
    if (enabled)
        return;

    /* reset UART */
    reset();

    /* setup gpio pins */
    gpio_init(UART_TX, GPFSEL_ALT0);
    gpio_init(UART_RX, GPFSEL_ALT0);
    gpio_pull(UART_TX, GPPULL_DOWN);
    gpio_pull(UART_RX, GPPULL_DOWN);


    /* setup interrupts */
    uart->icr = 0x7FF;
    uart->imsc = 0x000;

    switch (baud) {
        case 9600:
        case 57600:
        case 115200:
            uart->ibrd = (12000000 / baud) >> 6;
            uart->fbrd = (12000000 / baud) & 63;
            break;
        default:
            /* baud = 115200 */
            uart->ibrd = 1;
            uart->fbrd = 40;
    }

    switch (wlen) {
        case 8:
            uart->lcrh = WLEN_8BITS;
            break;
        case 7:
            uart->lcrh = WLEN_7BITS;
            break;
        case 6:
            uart->lcrh = WLEN_6BITS;
            break;
        case 5:
            uart->lcrh = WLEN_5BITS;
            break;
        default:
            uart->lcrh = WLEN_8BITS;
    }

    /* enable fifo */
    uart->lcrh |= FIFO_EN;

    /* enable uart */
    uart->cr = UART_EN | RX_EN | TX_EN;
    enabled = 1;
}

void uart_disable(void)
{
    reset();
    enabled = 0;
}

void uart_putc(char c)
{
    while (txfull() || !cts());
    uart->dr = c;
}

char uart_getc(void)
{
    while (rxempty());
    return (char)uart->dr;
}

static void reset(void)
{
    while (busy());
    uart->lcrh &= ~FIFO_EN;
    uart->cr = 0;
    uart->lcrh = 0;
}

static unsigned int busy(void)
{
    return uart->fr & UART_BUSY;
}

static unsigned int txfull(void)
{
    return uart->fr & TX_FIFO_FULL;
}

static unsigned int rxempty(void)
{
    return uart->fr & RX_FIFO_EMPTY;
}

static unsigned int cts(void)
{
    return uart->fr & CLEAR_TO_SEND;
}

