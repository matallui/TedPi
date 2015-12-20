#ifndef __UART__
#define __UART__

#define UART_BASE 0x00201000

#define UART_TX 14
#define UART_RX 15

#ifdef RPI2
#define UART_CTS 16
#else
#define UART_CTS 30
#endif

typedef struct uart_t {
    unsigned int dr;
    unsigned int rsrecr;
    unsigned int pad0[4];
    unsigned int fr;
    unsigned int pad1;
    unsigned int ilpr;
    unsigned int ibrd;
    unsigned int fbrd;
    unsigned int lcrh;
    unsigned int cr;
    unsigned int ifls;
    unsigned int imsc;
    unsigned int ris;
    unsigned int mis;
    unsigned int icr;
    unsigned int dmacr;
    unsigned int pad2[13];
    unsigned int itcr;
    unsigned int itip;
    unsigned int itop;
    unsigned int tdr;
} uart_t;

/*
 * DR (Data Register)
 */
#define OVERRUN_ERROR   (1 << 11)
#define BREAK_ERROR     (1 << 10)
#define PARITY_ERROR    (1 << 9)
#define FRAMING_ERROR   (1 << 8)

/*
 * FR (Flags Register)
 */
#define TX_FIFO_EMPTY   (1 << 7)
#define RX_FIFO_FULL    (1 << 6)
#define TX_FIFO_FULL    (1 << 5)
#define RX_FIFO_EMPTY   (1 << 4)
#define UART_BUSY       (1 << 3)
#define CLEAR_TO_SEND   (1 << 0)

/*
 * LCRH (Line Control Register)
 */
#define STICK_PARITY_EN (1 << 7)
#define WLEN_8BITS      (0b11 << 5)
#define WLEN_7BITS      (0b10 << 5)
#define WLEN_6BITS      (0b01 << 5)
#define WLEN_5BITS      (0b00 << 5)
#define FIFO_EN         (1 << 4)
#define TWO_STOP_BITS   (1 << 3)
#define EVEN_PARITY     (1 << 2)
#define PARITY_ENABLE   (1 << 1)
#define SEND_BREAK      (1 << 0)

/*
 * CR (Control Register)
 */
#define CTS_EN          (1 << 15)
#define RTS_EN          (1 << 14)
#define RX_EN           (1 << 9)
#define TX_EN           (1 << 8)
#define LOOPBACK_EN     (1 << 7)
#define UART_EN         (1 << 0)

/*
 * IFLS (Interrupt FIFO Level Select)
 */
#define RX_IFL_1_8      (0b000 << 3)
#define RX_IFL_1_4      (0b001 << 3)
#define RX_IFL_1_2      (0b010 << 3)
#define RX_IFL_3_4      (0b011 << 3)
#define RX_IFL_7_8      (0b100 << 3)
#define TX_IFL_1_8      (0b000 << 0)
#define TX_IFL_1_4      (0b001 << 0)
#define TX_IFL_1_2      (0b010 << 0)
#define TX_IFL_3_4      (0b011 << 0)
#define TX_IFL_7_8      (0b100 << 0)

/*
 * IMSC (Interrupt Mask Set/Clear)
 */
#define OEIM            (1 << 10)   /* Overrun Error */
#define BEIM            (1 << 9)    /* Break Error */
#define PEIM            (1 << 8)    /* Parity Error */
#define FEIM            (1 << 6)    /* Faming Error */
#define RTIM            (1 << 5)    /* Receive Timeout */
#define TXIM            (1 << 4)    /* Transmit */
#define RXIM            (1 << 3)    /* Receive */

/*
 * RIS (Raw Interrupt Status)
 */
#define OERIS           (1 << 10)   /* Overrun Error */
#define BERIS           (1 << 9)    /* Break Error */
#define PERIS           (1 << 8)    /* Parity Error */
#define FERIS           (1 << 6)    /* Faming Error */
#define RTRIS           (1 << 5)    /* Receive Timeout */
#define TXRIS           (1 << 4)    /* Transmit */
#define RXRIS           (1 << 3)    /* Receive */

/*
 * MIS (Masked Interrupt Status)
 */
#define OEMIS           (1 << 10)   /* Overrun Error */
#define BEMIS           (1 << 9)    /* Break Error */
#define PEMIS           (1 << 8)    /* Parity Error */
#define FEMIS           (1 << 6)    /* Faming Error */
#define RTMIS           (1 << 5)    /* Receive Timeout */
#define TXMIS           (1 << 4)    /* Transmit */
#define RXMIS           (1 << 3)    /* Receive */

/*
 * ICR (Interrupt Clear Register)
 */
#define OEIC            (1 << 10)   /* Overrun Error */
#define BEIC            (1 << 9)    /* Break Error */
#define PEIC            (1 << 8)    /* Parity Error */
#define FEIC            (1 << 6)    /* Faming Error */
#define RTIC            (1 << 5)    /* Receive Timeout */
#define TXIC            (1 << 4)    /* Transmit */
#define RXIC            (1 << 3)    /* Receive */


/*
 * Function Headers
 */
void uart_enable(unsigned int baud, unsigned int wlen);
void uart_disable(void);

void uart_putc(char c);
char uart_getc(void);

#endif /* __UART__ */

