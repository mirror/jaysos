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



#include "ui_mgr.h"
#include "msg_queue.h"
#include "semaphore.h"
#include "util.h"
#include "zxfont.c"

#define WINDOW_BORDER 4


static u32 s_redraw_thread_stack[DEFAULT_STACK_SIZE];


static struct semaphore_t s_client_sem;   //controls access to the following 4 arrays

static struct msg_queue_t* s_ui_msg_queues[MAX_WINDOWS];
static struct window_t s_windows[MAX_WINDOWS];
static draw_callback_t  s_draw_callbacks[MAX_WINDOWS];
static void* s_draw_callback_data[MAX_WINDOWS];
static u32  s_alarm_intervals[MAX_WINDOWS];

static int s_clients = 0;
static volatile int s_active_window = -1;


static struct window_t s_root_window = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, VideoBuffer, SCREEN_WIDTH, SCREEN_HEIGHT };


static volatile bool s_need_redraw_all;


static void
draw_borders();


static const struct msg_t s_lost_focus_msg = {MSG_LOST_FOCUS, {0}};
static const struct msg_t s_got_focus_msg = {MSG_GOT_FOCUS, {0}};
static const struct msg_t s_resize_msg = {MSG_RESIZE, {0}};
static const struct msg_t s_alarm_msg = {MSG_ALARM, {0}};


//-----------------------------------------------------------

/*
 * thread that asks each window to redraw itself
 */
void
redraw_thread_start() {
    while(1) {
        int i;
        semaphore_wait(&s_client_sem);
        for (i=0; i < MAX_WINDOWS; i++) {
            if (s_draw_callbacks[i])
                s_draw_callbacks[i](s_windows[i], s_need_redraw_all, s_draw_callback_data[i]);

        }
        s_need_redraw_all = FALSE;
        semaphore_signal(&s_client_sem);
        thread_sleep_self(2);
    }
}

//-----------------------------------------------------------

static void
_update_active_window(int direction) {
    int i = s_active_window;
    int step = (direction > 0) ? 1 : -1;

    if (s_clients < 0) return;

    do {
        i += step;

        if (i >= MAX_WINDOWS)
            i = 0;
        else if (i < 0)
            i = MAX_WINDOWS-1;

        if (s_ui_msg_queues[i]){

            if ((s_active_window > -1) && (s_ui_msg_queues[s_active_window]))
                msg_queue_add(s_ui_msg_queues[s_active_window], s_lost_focus_msg, FALSE);

            s_active_window = i;

            msg_queue_add(s_ui_msg_queues[s_active_window], s_got_focus_msg, FALSE);
            return;
        }


    }
    while (i != s_active_window);
}



//-----------------------------------------------------------
// functions called from interrupts
//-----------------------------------------------------------

static u32 s_last_button_irq_time = 0;


void
button_press_isr() {
    struct msg_t msg;

    if (s_active_window < 0) return;

    if (kern_uptime_centisecs - s_last_button_irq_time < 20)
        return;

    s_last_button_irq_time = kern_uptime_centisecs;
    if (~REG_KEY & BUTTON_SHOULDER_LEFT) {
        _update_active_window(-1);
        draw_borders();
    }
    else if (~REG_KEY & BUTTON_SHOULDER_RIGHT) {
        _update_active_window(1);
        draw_borders();
    }
    else if (~REG_KEY & BUTTON_START) {
        msg.type = MSG_PAUSE;
        msg_queue_add(s_ui_msg_queues[s_active_window], msg, FALSE);
    }
    else {
        msg.type = MSG_BUTTON_PRESS;
        msg.m.button = REG_KEY;
        msg_queue_add(s_ui_msg_queues[s_active_window], msg, FALSE);
    }
}


//-----------------------------------------------------------
// functions called from supervisor mode (at startup)
//-----------------------------------------------------------
void
init_uimgr() {
    int i;
    REG_DISPCNT = 3 | (1<<10); // set to mode 3
    for (i=0; i < MAX_WINDOWS; i++) {
        s_ui_msg_queues[i] =  NULL;
        s_draw_callbacks[i] = NULL;
        s_draw_callback_data[i] = NULL;
        s_alarm_intervals[i] = 0;
    }
    semaphore_create(&s_client_sem, 1);
    thread_create("redraw", redraw_thread_start, s_redraw_thread_stack, DEFAULT_STACK_SIZE, NULL);
};


//-----------------------------------------------------------
// functions called from threads (user mode)
//-----------------------------------------------------------

static void
draw_borders() {
    int i;
    for (i=0; i < MAX_WINDOWS; i++) {
        if (s_ui_msg_queues[i] != NULL) {
            draw_rect(&s_root_window, s_windows[i].origin_x-WINDOW_BORDER, s_windows[i].origin_y-WINDOW_BORDER,
                    s_windows[i].width+WINDOW_BORDER, s_windows[i].height+WINDOW_BORDER, WINDOW_BORDER,
                    (i == s_active_window) ? 0xFFEE : 0xFFFF);
        }
    }
}

/*
 * note as there may be gaps in s_windows,
 * 'n' is not a direct index into s_windows, it is the "n-th non-null window"
 */
static void
resize_window(int n, int x, int y, int width, int height) {
    int i;
    int found = -1;
    for (i=0; i < MAX_WINDOWS; i++) {
        if (s_ui_msg_queues[i] != NULL)
            found++;

        if (found == n) {
            s_windows[i].origin_x = x+WINDOW_BORDER;
            s_windows[i].origin_y = y+WINDOW_BORDER;
            s_windows[i].width = width-(WINDOW_BORDER*2);
            s_windows[i].height = height-(WINDOW_BORDER*2);

            msg_queue_add(s_ui_msg_queues[i], s_resize_msg, FALSE);
            return;
        }
    }
}

static void
clients_changed() {
    switch (s_clients) {
        case 0: break;
        case 1:
            resize_window(0, 0,0,240,160);
            break;
        case 2: {
            resize_window(0, 0,0,120,160);
            resize_window(1, 120,0,120,160);
            break;
        }
        case 3: {
            resize_window(0, 0,0,120,80);
            resize_window(1, 120,0,120,80);
            resize_window(2, 0,80,120,80);
            break;
        }
        case 4: {
           resize_window(0, 0,0,120,80);
           resize_window(1, 120,0,120,80);
           resize_window(2, 0,80,120,80);
           resize_window(3, 120,80,120,80);
        }
    }
    draw_borders();
    s_need_redraw_all = TRUE;
}

const struct window_t*
register_with_uimgr(struct msg_queue_t* queue, draw_callback_t draw_cb, void* cb_data) {
    int i = 0;
    semaphore_wait(&s_client_sem);
    if (s_clients >= MAX_WINDOWS) {
        panic("register_with_uimgr: no free slots");
        semaphore_signal(&s_client_sem);
        return NULL;
    }
    s_clients++;
    while (i < MAX_WINDOWS) {
        if (s_ui_msg_queues[i] == NULL) {
            s_ui_msg_queues[i] = queue;
            s_draw_callbacks[i] = draw_cb;
            s_draw_callback_data[i] = cb_data;
            s_windows[i].video_buffer = VideoBuffer;
            s_windows[i].buffer_width = SCREEN_WIDTH;
            s_windows[i].buffer_height = SCREEN_HEIGHT;
            if ((s_active_window > -1) && (s_ui_msg_queues[s_active_window]))
                msg_queue_add(s_ui_msg_queues[s_active_window], s_lost_focus_msg, FALSE);
            s_active_window = i;
            msg_queue_add(s_ui_msg_queues[s_active_window], s_got_focus_msg, FALSE);
            break;
        }
        i++;
    }

    //clear the screen
    fill_rect(&s_root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    clients_changed();
    semaphore_signal(&s_client_sem);

    return &s_windows[i];
}


void
unregister_with_uimgr(struct msg_queue_t* queue) {
    int i;
    semaphore_wait(&s_client_sem);
    for(i = 0; i < MAX_WINDOWS; i++) {
        if (s_ui_msg_queues[i] == queue) {
            s_ui_msg_queues[i] = NULL;
            s_draw_callbacks[i] = NULL;
            s_draw_callback_data[i] = NULL;
            s_alarm_intervals[i] = 0;

            s_clients--;
            if (s_clients < 1)
                s_active_window = -1;
            else
                _update_active_window(1);

            fill_rect(&s_root_window, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
            clients_changed();

            semaphore_signal(&s_client_sem);
            return;
        }
    }

    semaphore_signal(&s_client_sem);
    panic("unregister_with_uimgr: unknown queue");
}

struct msg_queue_t*
get_ui_queue_by_name(const char* name) {
    int i;
    semaphore_wait(&s_client_sem);

    for(i = 0; i < MAX_WINDOWS; i++) {
        if (s_ui_msg_queues[i] &&
            (strncmp(name, s_ui_msg_queues[i]->name, strlen(s_ui_msg_queues[i]->name)) == 0)) {

            semaphore_signal(&s_client_sem);
            return s_ui_msg_queues[i];
        }
    }

    semaphore_signal(&s_client_sem);
    //panic("get_ui_queue_by_name: unknown queue");
    return NULL;
}

//note - does not hold client sem
static int
get_queue_id(struct msg_queue_t* queue) {
    int i;
    for (i = 0; i < MAX_WINDOWS; i++) {
        if (s_ui_msg_queues[i] == queue) {
            return i;
        }
    }

    panic("get_queue_id: unknown queue");
    return -1;

}

/*
 * prints all the ui queue names over the serial link
 * only used by the shell app
 */
void
print_ui_queue_names() {
    int i;

    semaphore_wait(&s_client_sem);
    printf("ui msg queues:\r\n");
    for(i = 0; i < MAX_WINDOWS; i++) {

        if (s_ui_msg_queues[i])
            printf("  %s\r\n", s_ui_msg_queues[i]->name);
    }
    semaphore_signal(&s_client_sem);
}


void
ui_alarm_cb(void* cb_data) {
    int id = (int)cb_data;
    //note we must use "try" not "wait" as we are called from an ISR
    if (semaphore_try(&s_client_sem)) {
        if (s_ui_msg_queues[id]) {
            msg_queue_add(s_ui_msg_queues[id], s_alarm_msg, FALSE);
            //reschedule it to run again
            if (s_alarm_intervals[id] > 0) {
                alarm_add(ui_alarm_cb, s_alarm_intervals[id], (void*)id);
            }

        }

        semaphore_signal(&s_client_sem);
    }
    else {
        //try again!
        alarm_add(ui_alarm_cb, 1, cb_data);
        //panic("!!!");

    }
}

/*
 * causes the ui queue to get a MSG_ALARM event in centisecs_from_now,
 * each queue can only have one alarm registered at at time.
 * reoccurring means the event will be reposted every centisecs_from_now,
 * otherwise its a once off alarm.
 */

bool
ui_alarm_add(struct msg_queue_t* queue, u32 centisecs_from_now, bool reocurring) {
    bool ok;
    int id;

    semaphore_wait(&s_client_sem);
    id = get_queue_id(queue);
    if (id < 0) return FALSE;

    ok = alarm_add(ui_alarm_cb, centisecs_from_now, (void*)id);
    if (ok && reocurring) {
        s_alarm_intervals[id] = centisecs_from_now;
    }
    semaphore_signal(&s_client_sem);
    return ok;

}

bool
post_ui_msg(struct msg_queue_t* queue, struct msg_t msg) {
    bool posted;

    semaphore_wait(&s_client_sem);
    posted = msg_queue_add(queue, msg, FALSE);
    semaphore_signal(&s_client_sem);

    return posted;
}


//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

void
draw_pixel(const struct window_t* window, int x, int y, int colour) {
    volatile u16* display;

    if ((x < 0) || (y < 0) || (x > window->width) || (y > window->height))
        return;

    display = window->video_buffer + ((window->origin_y + y) * window->buffer_width) + window->origin_x + x;

    *display = colour;
}

void
fill_rect(const struct window_t* window,
          int x, int y, int width, int height, int colour) {
    volatile u16* display;

    int i,j;

    if (x < 0) {
        x = 0;
        width += x;
    }

    if (y < 0) {
        y = 0;
        height += y;
    }

    display = window->video_buffer + ((window->origin_y + y) * window->buffer_width) + window->origin_x + x;

    if (width + x > window->width)
        width = window->width - x;


    if (height + y > window->height)
        height = window->height - y;


    for (i=0; i < height; i++) {
        for (j=0; j < width; j++) {
            display[j] = colour;

        }
        display += window->buffer_width;
    }
}

void
draw_rect(const struct window_t* window,
          int x, int y, int width, int height, int thickness, int colour) {
    fill_rect(window, x, y, width, thickness, colour);
    fill_rect(window, x, y+height, width, thickness, colour);
    fill_rect(window, x, y, thickness, height, colour);
    fill_rect(window, x+width, y, thickness, height+thickness, colour);

}



/* based on Bresenham implementation from http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html */
static void
_draw_line(const struct window_t* window, int x0, int y0, int x1, int y1, int colour) {
    int dy;
    int dx;
    int stepx, stepy;

    dy = y1 - y0;
    dx = x1 - x0;

    if (dy < 0) { dy = -dy;  stepy = -window->buffer_width; } else { stepy = window->buffer_width; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;
    dx <<= 1;

    x0 += window->origin_x;
    x1 += window->origin_x;

    y0 += window->origin_y;
    y1 += window->origin_y;

    y0 *= window->buffer_width;
    y1 *= window->buffer_width;

    window->video_buffer[x0+y0] = colour;


    if (dx > dy) {
        int fraction = dy - (dx >> 1);
        while (x0 != x1) {
            if (fraction >= 0) {
                y0 += stepy;
                fraction -= dx;
            }
            x0 += stepx;
            fraction += dy;
            window->video_buffer[x0+y0] = colour;

        }
    } else {
        int fraction = dx - (dy >> 1);
        while (y0 != y1) {
            if (fraction >= 0) {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;
            fraction += dx;
            window->video_buffer[x0+y0] = colour;
        }
    }
}




static int
clip_code(const struct window_t* window, int x, int y) {

    return  (x < 0 ? 8 : 0) | (x > window->width ? 4 : 0) | (y < 0 ? 2 : 0) | (y > window->height ? 1 : 0);
}


/* clips the line to inside the window, and calls _draw_line to do the actual
 * drawing.
 * based on Sutherland-Cohen clipping implementation from
 * Ammeraal, L. (1998) Computer Graphics for Java Programmers
 * http://mercury.tvu.ac.uk/gpa/ammeraal/ClipLine.java
 */
void
draw_line(const struct window_t* window, int x0, int y0, int x1, int y1, int colour)  {
    int c0 = clip_code(window, x0, y0);
    int c1 = clip_code(window, x1, y1);

    //printf("before clipping: x0=%d y0=%d x1=%d y1=%d\r\n", x0, y0, x1, y1);
    int dx, dy;
    while ((c0 | c1) != 0) {

        if ((c0 & c1) != 0) return;

        dx = x1 - x0;
        dy = y1 - y0;

        if (c0 != 0) {
            if ((c0 & 8) == 8) {

                y0 += -x0 * dy / dx;
                x0 = 0;
            }
            else if ((c0 & 4) == 4) {
                y0 += (window->width-x0) * dy / dx;
                x0 = window->width;
            }
            else if ((c0 & 2) == 2) {
                x0 += -y0 * dx / dy;
                y0 = 0;
            }
            else if ((c0 & 1) == 1) {
                x0 += (window->height-y0) * dx / dy;
                y0 = window->height;
            }

            c0 = clip_code(window, x0, y0);
        }
        else if (c1 != 0) {

            if ((c1 & 8) == 8) {
                y1 += (-x1) * dy / dx;
                x1 = 0;
            }
            else if ((c1 & 4) == 4) {
                y1 += (window->width-x1) * dy / dx;
                x1 = window->width;
            }
            else if ((c1 & 2) == 2) {
                x1 += (-y1) * dx / dy;
                y1 = 0;
            }
            else if ((c1 & 1) == 1) {
                x1 += (window->height-y1) * dx / dy;
                y1 = window->height;
            }

            c1 = clip_code(window, x1, y1);
        }
    }

    //printf("after clipping: x0=%d y0=%d x1=%d y1=%d\r\n\r\n", x0, y0, x1, y1);
    _draw_line(window, x0, y0, x1, y1, colour);

}



/*
 * copy the entire src window to dest, placing it at x, y
 */
void
copy_window_unscaled(const struct window_t* dest_win, int x, int y, const struct window_t* src_win) {

    volatile u16* dest_vid;
    volatile u16* src_vid;

    u16 dest_y, dest_x, src_x;
    u16 dest_y_edge, dest_x_edge;

    int src_origin_x, src_origin_y, src_height, src_width;
/*
    printf("\r\ndest_win->height=%d, dest_win->origin_y=%d dest_win->width=%d dest_win->origin_x=%d\r\n",
        dest_win->height, dest_win->origin_y, dest_win->width, dest_win->origin_x);
*/
    if (( x > dest_win->width) ||
        (y > dest_win->height))
        return;
/*
    printf("src_win->height=%d, src_win->origin_y=%d src_win->width=%d src_win->origin_x=%d\r\n",
        src_win->height, src_win->origin_y, src_win->width, src_win->origin_x);

*/

    if (x < 0) {
        //make the src window smaller

        src_origin_x = src_win->origin_x - x;  //remember x is minus
        src_width =  src_win->width + x;
        x = 0;
    }
    else
    {
        src_origin_x  = src_win->origin_x;
        src_width = src_win->width;
    }

    if (y < 0) {
        src_origin_y = src_win->origin_y - y;
        src_height = src_win->height + y;
        y = 0;
    }
    else
    {
        src_origin_y  = src_win->origin_y;
        src_height = src_win->height;
    }

    //printf("src_height=%d, src_width=%d\r\n", src_height, src_width);

    if ((src_height < 0) || (src_width < 0)) return;


    dest_vid = dest_win->video_buffer +
               ((dest_win->origin_y + y) * dest_win->buffer_width) +
               dest_win->origin_x + x;

    src_vid = src_win->video_buffer +
              (src_origin_y * src_win->buffer_width) +
              src_origin_x;

    dest_y_edge =  dest_win->height - y;
    dest_x_edge =  dest_win->width - x;

    for (dest_y=0; (dest_y < src_height) && (dest_y < dest_y_edge); dest_y++) {
        src_x = 0;
        for (dest_x=0; (dest_x < src_width) && (dest_x < dest_x_edge); dest_x++) {
            dest_vid[dest_x] = src_vid[src_x++];
        }
        dest_vid += dest_win->buffer_width;
        src_vid += src_win->buffer_width;
    }
}



void
draw_string(const struct window_t* window, unsigned const char* c, int x, int y, int colour) {
    volatile u16* display = window->video_buffer + ((window->origin_y + y) * window->buffer_width) + window->origin_x + x;
    int max_chars = (window->width-x)/6;  //any further and we go off the screen
    if (!c) return;

    //TODO - improve me!
    if (( x > window->width) ||
        (y > window->height) ||
        (x < 0) || (y < 0))
        return;

    //printf("x=%d y=%d window->origin_x=%d, window->width=%d\r\n", x, y, window->origin_x, window->width);
    while (*c && (max_chars > 0)) {
        u8 bitbuffer;
        int x1, y1;
        int i = (*c) * 8;

        for (y1=0; y1 < 8; y1++) {
            bitbuffer = gbfont[i + y1];

            for (x1=0; x1 < 8; x1++) {
                if (bitbuffer & 0x80) {
                    display[x1] = colour;
                }
                /*
                else {
                    display[x1] = 0x0;
                }
                */
                bitbuffer <<= 1;
            }
            display += window->buffer_width;
        }
        c++;
        max_chars--;
        display += 6-(window->buffer_width*8);
    }
}


void
panic(char* str) {
    draw_string(&s_root_window, str, 10, 140, 0xFF);
}
