/*
 * Copyright (c) 2002 Justin Armstrong
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/*
 * this is a bit inconsistent, because for reading bytes over the UART
 * we have a proper read queue that is filled up by an ISR, but for
 * writing we just do a cheapass blocking thing.
 */



#include "uart.h"
#include "kernel.h"
#include "msg_queue.h"
#include "ui_mgr.h"

static struct msg_t s_read_queue_array[10];
struct msg_queue_t uart_read_queue;

static struct cond_lock_t s_data_sent_cond;
static bool s_waiting_to_send = FALSE;

void init_uart()
{
    REG_RCNT = 0;
    REG_SIOCNT = 0;
    REG_SIOCNT = SIO_BAUD_9600 | SIO_CTS | SIO_LENGTH_8 | SIO_SEND_ENABLE
             | SIO_RECV_ENABLE | SIO_USE_UART | SIO_REQUEST_IRQ;

    msg_queue_create(&uart_read_queue, "uart_read_queue", s_read_queue_array, 2);
    cond_lock_create(&s_data_sent_cond);

    REG_SIOCNT |= SIO_RECV_DATA | SIO_SEND_DATA;
    REG_RCNT = REG_RCNT & (0x0020 ^ 0xFFFF);

}



void
uart_isr() {
    struct msg_t msg;
    if ((REG_SIOCNT & ~SIO_RECV_DATA) && !s_waiting_to_send) {

        //read char into read buffer
        msg.type = MSG_UART_BYTE;
        msg.m.byte = REG_SIODATA8;

        msg_queue_add(&uart_read_queue, msg, FALSE);
        REG_RCNT = REG_RCNT & (0x0020 ^ 0xFFFF);
        REG_SIOCNT |= SIO_RECV_DATA | SIO_SEND_DATA;
    }

    if (REG_SIOCNT & ~SIO_SEND_DATA) {
        s_waiting_to_send = FALSE;
        cond_lock_signal(&s_data_sent_cond);
    }
}


void
send_byte(char c) {
    REG_RCNT = 0;   //not CTS
    REG_SIODATA8 = c;

    s_waiting_to_send = TRUE;
    cond_lock_wait(&s_data_sent_cond);

    REG_SIOCNT |= SIO_RECV_DATA | SIO_SEND_DATA;
    REG_RCNT = REG_RCNT & (0x0020 ^ 0xFFFF);    //set CTS back again
}


void
send_string(const char* str) {
    while(*str) {
        send_byte(*str);
        str++;
    }
}
