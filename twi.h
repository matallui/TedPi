#ifndef __TWI__
#define __TWI__

struct twi_packet {
    unsigned int    addr;
    unsigned int    offset;
    unsigned int    olen;
    void            *data;
    unsigned int    dlen;
};


void twi_enable(void);
void twi_disable(void);
int twi_read(struct twi_packet *packet);
int twi_write(struct twi_packet *packet);

#endif /* __TWI__ */
