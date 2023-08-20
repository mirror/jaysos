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


#include "msg_queue.h"
#include "cond_lock.h"


const struct msg_t s_empty_msg = {MSG_NO_MSG, {'#'}};


void
msg_queue_create(struct msg_queue_t* queue, const char* name, struct msg_t* array, int size) {
    queue->array = array;
    queue->max_index = size;
    queue->end_index = 0;
    queue->start_index = 0;
    queue->name = name;

    cond_lock_create(&queue->not_empty_cond);
    cond_lock_create(&queue->not_full_cond);
}


bool
msg_queue_add(struct msg_queue_t* queue, struct msg_t msg, bool blocking) {
    if (blocking) {
        while ((queue->start_index + 1) % queue->max_index == queue->end_index)
            cond_lock_wait(&queue->not_full_cond);
    }
    else {
        if ((queue->start_index + 1) % queue->max_index == queue->end_index)
            return FALSE;
    }

    DISABLE_INTERRUPTS();
    queue->array[queue->start_index] = msg;

    queue->start_index++;
    if (queue->start_index > queue->max_index) {
        queue->start_index = 0;
    }
    cond_lock_signal(&queue->not_empty_cond);

    ENABLE_INTERRUPTS();
    return TRUE;
}


struct msg_t
msg_queue_remove(struct msg_queue_t* queue, bool blocking) {
    struct msg_t msg;
    if (blocking) {
        while (queue->start_index == queue->end_index) {
            cond_lock_wait(&queue->not_empty_cond);
        }
    }
    else {
        if (queue->start_index == queue->end_index)
            return s_empty_msg;
    }
    DISABLE_INTERRUPTS();
    msg = queue->array[queue->end_index];

    queue->end_index++;
    if (queue->end_index > queue->max_index) {
        queue->end_index = 0;
    }

    cond_lock_signal(&queue->not_full_cond);
    ENABLE_INTERRUPTS();
    return msg;
}
