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


#define CELL_COLS 35
#define CELL_ROWS 35

#define DEAD_NOW 0
#define ALIVE_NOW 1
#define ALIVE_NEXT_GEN 2

struct life_locals_t {
    int cells[CELL_COLS][CELL_ROWS];

    struct cond_lock_t screen_displayed_cond;

    volatile bool ready_for_display;
    bool paused;
    int cell_width;
    int cell_height;
    int start_x;
    int start_y;

    char ui_queue_name[32];

};

//------------------------------------------------------------

static void
life_draw_cb(struct window_t window, bool redraw_all, void* cb_data) {

    int row, col, x, y;
    struct life_locals_t* locals = (struct life_locals_t*)cb_data;

    if (locals->paused)
        draw_string(&window, "paused", 0, 0, 0xFFFF);

    if (redraw_all) {
        locals->cell_width = window.width/CELL_COLS;
        locals->cell_height = window.height/CELL_ROWS;
        locals->start_x = (window.width-(locals->cell_width*CELL_COLS))/2;
        locals->start_y = (window.height-(locals->cell_height*CELL_ROWS))/2;
        fill_rect(&window, 0, 0, window.width, window.height, 0);
    }

    if (!locals->ready_for_display)
        return;

    y = locals->start_y;
    for(col = 1; col < CELL_COLS-1; col++) {
        x = locals->start_x;
        for (row = 1; row < CELL_ROWS-1; row++) {

            if (locals->cells[row][col] & ALIVE_NOW)
                fill_rect(&window, x, y, locals->cell_width, locals->cell_height, 0xFFFF);
            else fill_rect(&window, x, y, locals->cell_width, locals->cell_height, 0xEECC);
            x += locals->cell_width;

        }
        y += locals->cell_height;
    }

    locals->ready_for_display = FALSE;
    cond_lock_signal(&locals->screen_displayed_cond);
}
//-----------------------

#define LOCALS     ((struct life_locals_t*)(kern_threads[thread_current()].thread_local_data))


static int
count_neighbours(int x, int y) {
    int n = 0;

    if (LOCALS->cells[x][y-1] & ALIVE_NOW) n++;
    if (LOCALS->cells[x][y+1] & ALIVE_NOW) n++;

    if (LOCALS->cells[x-1][y] & ALIVE_NOW) n++;
    if (LOCALS->cells[x+1][y] & ALIVE_NOW) n++;

    if (LOCALS->cells[x-1][y-1] & ALIVE_NOW) n++;
    if (LOCALS->cells[x+1][y+1] & ALIVE_NOW) n++;

    if (LOCALS->cells[x-1][y+1] & ALIVE_NOW) n++;
    if (LOCALS->cells[x+1][y-1] & ALIVE_NOW) n++;

    return n;
}


static void
run_generation() {
    int x,y,n;
    for(x = 1; x < CELL_COLS-1; x++) {
        for (y = 1; y < CELL_ROWS-1; y++) {
            n = count_neighbours(x, y);
            if (LOCALS->cells[x][y] & ALIVE_NOW) {
                if ((n > 1) && (n < 4))
                    LOCALS->cells[x][y] |= ALIVE_NEXT_GEN;   //still alive...
            }
            else {
                if (n == 3)
                    LOCALS->cells[x][y] |= ALIVE_NEXT_GEN;  //a cell is born
            }
        }
    }
}

static void
clear_cells() {
    int x,y;
    for(x = 0; x < CELL_COLS; x++)
        for (y = 0; y < CELL_ROWS; y++)
           LOCALS->cells[x][y] = DEAD_NOW;
}

static void
init_cells() {
    int i;
    for (i=0; i<400; i++) {
        LOCALS->cells[rand() % (CELL_COLS-1)][(rand() % (CELL_ROWS-1))] = ALIVE_NOW;
    }
}

static void
reap() {
    int x,y;
    for(x = 0; x < CELL_COLS; x++) {
        for (y = 0; y < CELL_ROWS; y++) {
            if (LOCALS->cells[x][y] & ALIVE_NEXT_GEN)
                LOCALS->cells[x][y] = ALIVE_NOW;
            else LOCALS->cells[x][y] = DEAD_NOW;
        }
    }
}

//-----------------------

static void
life_thread_run() {
    struct msg_t msg;
    struct msg_t ui_queue_array[10];
    struct msg_queue_t ui_queue;
    const struct window_t* window;

    LOCALS = malloc(sizeof(struct life_locals_t));
    snprintf(LOCALS->ui_queue_name, 32, "life%d", thread_current());
    msg_queue_create(&ui_queue, LOCALS->ui_queue_name, ui_queue_array, 10);
    cond_lock_create(&LOCALS->screen_displayed_cond);
    LOCALS->ready_for_display = TRUE;

    window = register_with_uimgr(&ui_queue, life_draw_cb, LOCALS);
    if (!window) {
        //aggh!
        return;
    }

    clear_cells();
    init_cells();

    LOCALS->ready_for_display = FALSE;
    LOCALS->paused = FALSE;

    while(1) {
        msg = msg_queue_remove(&ui_queue, LOCALS->paused);
        if (msg.type == MSG_BUTTON_PRESS) {
            if (~msg.m.button & BUTTON_A) {
                clear_cells();
                init_cells();
            }
        }
        else if (msg.type == MSG_PAUSE) {
            LOCALS->paused = !LOCALS->paused;
        }
        else if (msg.type == MSG_QUIT) {
            unregister_with_uimgr(&ui_queue);
            return; //thread dies
        }

        if (!LOCALS->paused) {
            run_generation();
            reap();
            LOCALS->ready_for_display = TRUE;
            cond_lock_wait(&LOCALS->screen_displayed_cond);
        }
    }
}
//-----------------------------------------------------------------------------

static void
life_exited(void* cb_data) {
    struct thread_t* dead_thread = (struct thread_t*)cb_data;
    free(dead_thread->stack_end);
    free(dead_thread->thread_local_data);
}


void
start_life() {
    void* life_thread_stack = malloc(DEFAULT_STACK_SIZE);
    thread_create("life", life_thread_run, life_thread_stack, DEFAULT_STACK_SIZE, life_exited);
}
