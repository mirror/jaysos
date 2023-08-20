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
#include "cond_lock.h"
#include "util.h"

static struct cond_lock_t s_screen_displayed_cond;

static volatile bool s_ready_for_display;

static int s_usage_since_boot[6];
static int s_usage_this_sample[6];
static int s_percentage_usage_this_sample[6];
static int s_total_usage_this_sample;

static void
stats_draw_cb(struct window_t window, bool redraw_all, void* cb_data) {

    int i,x,y;

    if (!s_ready_for_display)
        return;

    fill_rect(&window, 0, 0, window.width, window.height, 0);

    x = 5;
    y = 5;

    for (i = 0; i < 6; i++) {
        draw_string(&window, itoa(s_percentage_usage_this_sample[i], 2), x, y, 0xFFFF);
        draw_string(&window, "%", x+16, y, 0xFFFF);
        draw_string(&window, thread_name(i), x+32, y, 0xFFFF);
        y += 8;
    }
    draw_string(&window, "heap:", x, y, 0xFFFF);
    draw_string(&window, itoa(count_bytes_free(), 8), x+34, y, 0xFFFF);
    
    s_ready_for_display = FALSE;
    cond_lock_signal(&s_screen_displayed_cond);

}
//-----------------------

static bool s_stats_running = FALSE;

static void
stats_thread_run() {

    int i;
    
    struct msg_t ui_queue_array[10];
    struct msg_queue_t ui_queue;
    struct msg_t msg;
    
    msg_queue_create(&ui_queue, thread_name(thread_current()), ui_queue_array, 10);
   
    cond_lock_create(&s_screen_displayed_cond); 
    s_ready_for_display = TRUE;

    register_with_uimgr(&ui_queue, stats_draw_cb, NULL);

    for (i = 0; i < 5; i++) {
        s_usage_since_boot[i] = 0;
    }  
        
    while(1) {


        msg = msg_queue_remove(&ui_queue, FALSE);
        
        //the only msg we are interested in is MSG_QUIT
        
        if (msg.type == MSG_QUIT) {
            unregister_with_uimgr(&ui_queue);
            s_stats_running = FALSE;
            return; //thread dies
        
        }        
        
        
        s_total_usage_this_sample = 0;
        for (i = 0; i < 6; i++) {
            s_usage_this_sample[i] = thread_cpu_usage(i) - s_usage_since_boot[i];
            s_usage_since_boot[i] += s_usage_this_sample[i];
            s_total_usage_this_sample += s_usage_this_sample[i];
        }
    
        for (i = 0; i < 6; i++) {
            s_percentage_usage_this_sample[i] = ((float)s_usage_this_sample[i]/(float)s_total_usage_this_sample)*100.0;

        }       


        s_ready_for_display = TRUE;
        cond_lock_wait(&s_screen_displayed_cond); 

        thread_sleep_self(100);
        
    }

}

//-----------------------------------------------------------------------------

static u8 s_stats_thread_stack[DEFAULT_STACK_SIZE];



void
start_stats() {
    if (s_stats_running) return;
    thread_create("stats", stats_thread_run, s_stats_thread_stack, DEFAULT_STACK_SIZE, NULL);  
    s_stats_running = TRUE;

}

