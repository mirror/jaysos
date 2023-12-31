/*
 * based on uart.h by Adrian O'Grady
 */


#ifndef UART_H
#define UART_H

// Register base address
#define REG_BASE          0x4000000

// Serial IO Registers
#define REG_SIOCNT        *(volatile unsigned short int *)(REG_BASE + 0x128)  // Serial control
#define REG_SIODATA8      *(volatile unsigned short int *)(REG_BASE + 0x12a)  // Serial data
#define REG_RCNT          *(volatile unsigned short int *)(REG_BASE + 0x134)  // General IO


// UART settings
#define SIO_USE_UART      0x3000

// Baud Rate
#define SIO_BAUD_9600     0x0000
#define SIO_BAUD_38400    0x0001
#define SIO_BAUD_57600    0x0002
#define SIO_BAUD_115200   0x0003

#define SIO_CTS           0x0004
#define SIO_PARITY_ODD    0x0008
#define SIO_SEND_DATA     0x0010
#define SIO_RECV_DATA     0x0020
#define SIO_ERROR         0x0040
#define SIO_LENGTH_8      0x0080
#define SIO_USE_FIFO      0x0100
#define SIO_USE_PARITY    0x0200
#define SIO_SEND_ENABLE   0x0400
#define SIO_RECV_ENABLE   0x0800
#define SIO_REQUEST_IRQ   0x4000


void
init_uart();

void
send_string(const char* str);

void
send_byte(char c);


#endif
