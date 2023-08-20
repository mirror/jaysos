#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include "kernel.h"
#include "cond_lock.h"

//i bet you thought this would be in ui_mgr.h! ha!
struct ui_window_t {
    int x;
    int y;
    int width;
    int height;

};


enum msg_type { MSG_NO_MSG, MSG_BUTTON_PRESS, MSG_GOT_FOCUS, MSG_LOST_FOCUS,
                MSG_RESIZE, MSG_PAUSE, MSG_ALARM, MSG_QUIT, MSG_UART_BYTE };


struct msg_t {
    enum msg_type     type;
    union {
        char button;
        char byte;
        struct ui_window_t window;

    } m;

};


struct msg_queue_t {
    struct msg_t*  array;
    const char* name;
    int max_index;
    volatile int end_index;
    volatile int start_index;

    struct cond_lock_t not_empty_cond;
    struct cond_lock_t not_full_cond;
};


void
msg_queue_create(struct msg_queue_t* queue, const char* name, struct msg_t* array, int size);

bool
msg_queue_add(struct msg_queue_t* queue, struct msg_t msg, bool blocking);



struct msg_t
msg_queue_remove(struct msg_queue_t* queue, bool blocking);


#endif
