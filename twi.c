#include "clib.h"
#include "kernel.h"
#include "gpio.h"
#include "systimer.h"
#include "twi.h"

#define TWI_BASE    0x00804000UL

#define TWI_SDA     2
#define TWI_SCL     3

#define TWI_C_I2CEN (1 << 15)
#define TWI_C_INTR  (1 << 10)
#define TWI_C_INTT  (1 << 9)
#define TWI_C_INTD  (1 << 8)
#define TWI_C_ST    (1 << 7)
#define TWI_C_CLEAR (3 << 4)
#define TWI_C_READ  (1 << 0)

#define TWI_S_CLKT  (1 << 9)
#define TWI_S_ERR   (1 << 8)
#define TWI_S_RXF   (1 << 7)
#define TWI_S_TXE   (1 << 6)
#define TWI_S_RXD   (1 << 5)
#define TWI_S_TXD   (1 << 4)
#define TWI_S_RXR   (1 << 3)
#define TWI_S_TXW   (1 << 2)
#define TWI_S_DONE  (1 << 1)
#define TWI_S_TA    (1 << 0)

#define TWI_SUCCESS     0
#define TWI_EINIT       1
#define TWI_ENODEV      2
#define TWI_ENAK        3
#define TWI_ETIME       4

typedef struct twi_t {
    unsigned int control;
    unsigned int status;
    unsigned int data_len;
    unsigned int slave_addr;
    unsigned int fifo;
    unsigned int clk_div;
    unsigned int data_delay;
    unsigned int clk_stretch_timeout;
} twi_t;

volatile twi_t *twi = (twi_t*) (TWI_BASE + REG_BASE);

static int enabled = 0;


static void clear_status(void);
static void clear_fifo(void);
static void twi_reset(void);
static int parse_error(unsigned int error);
static void start_read(void);
static void start_write(void);


/*
 * TODO: receive options parameter (e.g., interrupts, clk)
 */
void twi_enable(void)
{
    if (enabled)
        return;

    gpio_init(TWI_SDA, GPFSEL_ALT0);
    gpio_init(TWI_SCL, GPFSEL_ALT0);
    //gpio_pull(TWI_SDA, GPPULL_UP);
    //gpio_pull(TWI_SCL, GPPULL_UP);
    twi_reset();
    enabled = 1;
}

void twi_disable(void)
{
    if (!enabled)
        return;

    gpio_init(TWI_SDA, GPFSEL_IN);
    gpio_init(TWI_SCL, GPFSEL_IN);
    twi_reset();
    enabled = 0;
}

static void clear_status(void)
{
    twi->status = (TWI_S_CLKT | TWI_S_ERR | TWI_S_DONE);
}

static void clear_fifo(void)
{
    twi->control |= TWI_C_CLEAR;
}

static void twi_reset(void)
{
    clear_status();
    clear_fifo();
    twi->control = 0;
}

static int parse_error(unsigned int error)
{
    if (error & TWI_S_ERR)
        return -TWI_ENAK;
    if (error & TWI_S_CLKT)
        return -TWI_ETIME;

    return 0;
}

static void start_read(void)
{
    twi->control = TWI_C_I2CEN | TWI_C_ST | TWI_C_READ; 
}

static void start_write(void)
{
    twi->control = TWI_C_I2CEN | TWI_C_ST;
}

int twi_read(struct twi_packet *packet)
{
    int i;
    unsigned char *p;
    int ret;
    unsigned int offset;

    if (!enabled)
        return -TWI_ENODEV;

    twi_reset();

    offset = packet->offset;
    twi->slave_addr = packet->addr;
    
    if (packet->olen) {
        if (packet->olen > sizeof(packet->offset))
            packet->olen = sizeof(packet->offset);
        twi->data_len = packet->olen;
        start_write();
        /* wait for start */
        while (!(twi->status & (TWI_S_TA | TWI_S_ERR)));
        
        if (twi->status & TWI_S_ERR)
            return -TWI_ENAK;
    }

    /* setup (repeated) start condition */
    twi->data_len = packet->dlen;
    start_read();

    if (packet->olen) {
        /* send offset */
        for (i = 0; i < packet->olen; i++) {
            twi->fifo = (offset & 0xFF);
            offset = (offset >> 8);
        }
        /* make sure offset was sent */
        while (twi->status & TWI_S_TA);
        /* check for errors */
        ret = twi->status & (TWI_S_ERR | TWI_S_CLKT);
        if (ret) {
            twi_reset();
            return parse_error(ret);
        }
    }
    
    /* read data */
    p = (unsigned char *)packet->data;
    i = 0;
    while (!(twi->status & (TWI_S_DONE | TWI_S_CLKT | TWI_S_ERR))) {
        while ((i < packet->dlen) && (twi->status & TWI_S_RXD)) {
            p[i++] = (unsigned char)(twi->fifo & 0xFF);
        }
    }

    while ((i < packet->dlen) && (twi->status & TWI_S_RXD)) {
        p[i++] = (unsigned char)(twi->fifo & 0xFF);
    }

    if (i < packet->dlen)
        printf("* Did not receive all data (%d in %d)\n", i, packet->dlen);

    ret = (int)(twi->status & (TWI_S_CLKT | TWI_S_ERR));
    twi_reset();
    
    return parse_error(ret);
}

int twi_write(struct twi_packet *packet)
{
    int i;
    unsigned char *p;
    int ret;
    unsigned int offset;
    int total_len = 0;

    if (!enabled)
        return -TWI_ENODEV;

    twi_reset();

    offset = packet->offset;
    twi->slave_addr = packet->addr;
    
    if (packet->olen) {

        if (packet->olen > sizeof(packet->offset))
            packet->olen = sizeof(packet->offset);

        total_len += (int)packet->olen;
    }

    total_len += (int)packet->dlen;
    twi->data_len = total_len;

    start_write();

    /* wait for start */
    while (!(twi->status & (TWI_S_TA | TWI_S_ERR)));

    if (twi->status & TWI_S_ERR)
        return -TWI_ENAK;

   
    if (packet->olen) {
        /* send offset */
        for (i = 0; i < packet->olen; i++) {
            twi->fifo = (offset & 0xFF);
            offset = (offset >> 8);
        }
    }
    
    /* write data */
    p = (unsigned char *)packet->data;
    i = 0;
    while (!(twi->status & (TWI_S_DONE | TWI_S_CLKT | TWI_S_ERR))) {
        while ((i < packet->dlen) && (twi->status & TWI_S_TXD)) {
            twi->fifo = (unsigned int)p[i++];
        }
    }

    ret = (int)(twi->status & (TWI_S_CLKT | TWI_S_ERR));
    twi_reset();
    
    return parse_error(ret);
}

