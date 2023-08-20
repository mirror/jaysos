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



#include "kernel.h"
#include "ui_mgr.h"
#include "cond_lock.h"
#include "util.h"


#define ROWS 5
#define COLS 10


#define BLOCK_HEIGHT 4

#define BAT_WIDTH 20
#define BAT_HEIGHT 4

#define BALL_DMTR 5

#define BALL_DELTA_SIZE 2
#define BAT_DELTA_SIZE 2

struct breakout_locals_t {
    bool blocks[ROWS][COLS];

    struct window_t window;
    struct cond_lock_t screen_displayed_cond;

    volatile bool ready_for_display;
    bool paused;

    int old_ball_x;
    int old_ball_y;
    int ball_x;
    int ball_y;
    int ball_dx;
    int ball_dy;

    int old_bat_x;
    int bat_x;
    int bat_y;

    int block_width;
    int block_start_x;

    int lives;

    bool have_focus;

    char ui_queue_name[32];

};



//---------------------------------------
static void
init_game(struct breakout_locals_t* locals);

static void
draw_blocks(struct breakout_locals_t* locals);

static void
draw_ball(struct window_t* window, int x, int y, u16 colour);

static void
move_ball();

static void
destroy_block(int row, int col);

static void
player_die();

static bool
check_for_collision();

//-----------------------------------------------------------------

/*
 * The functions below are called from the redraw thread.
 *
 * Note, because of this they can't access the thread local data thru
 * the LOCALS macro. instead we register a pointer to the locals struct
 * with the ui mgr, and it passes that each time it calls the redraw function.
 * from there we pass it to each of these functions.
 *
 */

static void
init_game(struct breakout_locals_t* locals) {
    int row,col;
    for (col=0; col < COLS; col++) {
        for (row=0; row < ROWS; row++) {
            locals->blocks[row][col] = TRUE;
        }

    }
    locals->old_bat_x = locals->bat_x;
    locals->bat_x = (locals->window.width/2);
    locals->bat_y = (locals->window.height-BAT_HEIGHT-4);
    locals->ball_x = locals->bat_x+(BAT_WIDTH/2);
    locals->ball_y = locals->bat_y-BAT_HEIGHT-BALL_DMTR;
    locals->ball_dx = BALL_DELTA_SIZE;
    locals->ball_dy = -BALL_DELTA_SIZE;

    locals->block_width = (locals->window.width-10-5)/COLS;
    locals->block_start_x = 8;

    locals->lives = 3;

    draw_blocks(locals);

}


static void
draw_blocks(struct breakout_locals_t* locals) {
    int row,col,x,y;
    int colour = 0xFF;

    y = 0;
    for (row=0; row < ROWS; row++) {
        x = locals->block_start_x;
        for (col=0; col < COLS; col++) {
            if (locals->blocks[row][col]) {
                fill_rect(&locals->window, x, y, locals->block_width, BLOCK_HEIGHT, colour);
            }
            else fill_rect(&locals->window, x, y, locals->block_width, BLOCK_HEIGHT, 0);
            x+= locals->block_width+1;
        }
        colour += 256;
        y+= BLOCK_HEIGHT+1;
    }
}

static const int s_ball_img[] = {
     0, 1, 1, 1, 0,
     1, 1, 1, 1, 1,
     1, 1, 1, 1, 1,
     1, 1, 1, 1, 1,
     0, 1, 1, 1, 0

};


static void
draw_ball(struct window_t* window, int x, int y, u16 colour) {
    volatile u16* display = VideoBuffer + ((window->origin_y + y) * SCREEN_WIDTH) + window->origin_x + x;

    const int* ball_img = s_ball_img;

    int i,j;
    for (i=0; i < BALL_DMTR; i++) {
        for (j=0; j < BALL_DMTR; j++) {
            if (ball_img[j] == 1)
                display[j] = colour;
        }
        ball_img += BALL_DMTR;
        display += SCREEN_WIDTH;
    }
}


static void
breakout_draw_cb(struct window_t window, bool redraw_all, void* cb_data) {

    struct breakout_locals_t* locals = (struct breakout_locals_t*)cb_data;

    if (locals->paused)
        draw_string(&window, "paused", 0, 0, 0xFFFF);

    if (redraw_all) {
        locals->window = window;
        fill_rect(&window, 0, 0, window.width, window.height, 0);
        init_game(locals);

    }

    if (!locals->ready_for_display)
        return;

    draw_blocks(locals);
    draw_ball(&window, locals->old_ball_x, locals->old_ball_y, 0);
    draw_ball(&window, locals->ball_x, locals->ball_y, 0xFFFF);

    fill_rect(&window, locals->old_bat_x, locals->bat_y, BAT_WIDTH, BAT_HEIGHT, 0); //clear the old bat image
    fill_rect(&window, locals->bat_x, locals->bat_y, BAT_WIDTH, BAT_HEIGHT, 0xFFFF);
    draw_string(&window, itoa(locals->lives, 1), 0, 0, 0xFFFF);

    locals->ready_for_display = FALSE;
    cond_lock_signal(&locals->screen_displayed_cond);

}

//---------------------------------------------------------------------

/*
 * The functions below are called from the breakout thread.
 */

#define LOCALS     ((struct breakout_locals_t*)(kern_threads[thread_current()].thread_local_data))

static void
player_die() {
    if (--LOCALS->lives < 1)
        init_game(LOCALS);

    LOCALS->ball_x = LOCALS->bat_x+(BAT_WIDTH/2);
    LOCALS->ball_y = LOCALS->bat_y-BAT_HEIGHT-BALL_DMTR;

    LOCALS->ball_dx = BALL_DELTA_SIZE;
    LOCALS->ball_dy = -BALL_DELTA_SIZE;

}


static void
move_ball() {
    LOCALS->old_ball_x = LOCALS->ball_x;
    LOCALS->old_ball_y = LOCALS->ball_y;
    LOCALS->ball_x += LOCALS->ball_dx;
    LOCALS->ball_y += LOCALS->ball_dy;
}


static void
move_bat(int dx) {
    LOCALS->old_bat_x = LOCALS->bat_x;
    LOCALS->bat_x += dx;
}

static bool
rect_intersects(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2) {

    bool x_intersects = FALSE;
    bool y_intersects = FALSE;

    if (x2 >= x1)  {                         /* x1  x2 */
        if(x1+width1 >= x2)  {
            x_intersects = TRUE;
        }
    }
    else  {                                  /* x2 x1 */
        if (x2+width2 >= x1) {
            x_intersects = TRUE;
        }
    }


    if (y2 >= y1) {
        /*  y1
         *  y2
         */
        if (y1+height1 >= y2) {
            y_intersects = TRUE;
        }
    }
    else {
        /*  y2
         *  y1
         */
        if (y2+height2 >= y1) {
            y_intersects = TRUE;
        }
    }

    if (x_intersects && y_intersects)
        return TRUE;
    return FALSE;
}

static void
destroy_block(int row, int col) {
    LOCALS->blocks[row][col] = FALSE;
}


static bool
check_for_collision() {
    int row,col,x,y;

    //collision with wall
    if (LOCALS->ball_x <= BALL_DELTA_SIZE) {
        LOCALS->ball_dx = -LOCALS->ball_dx;
        return TRUE;
    }
    else if (LOCALS->ball_x >= (LOCALS->window.width-(BALL_DMTR + BALL_DELTA_SIZE))) {
        LOCALS->ball_dx = -LOCALS->ball_dx;
        return TRUE;
    }

    if (LOCALS->ball_y <= BALL_DELTA_SIZE) {
        LOCALS->ball_dy = -(LOCALS->ball_dy);
        return TRUE;
    }
    else if (LOCALS->ball_y >= (LOCALS->window.height-(BALL_DMTR + BALL_DELTA_SIZE))) {
        player_die();
        return TRUE;
    }

    //collision with bat
    if (rect_intersects(LOCALS->bat_x, LOCALS->bat_y, BAT_WIDTH, BAT_HEIGHT,
                        LOCALS->ball_x-BALL_DELTA_SIZE, LOCALS->ball_y-BALL_DELTA_SIZE, BALL_DMTR+BALL_DELTA_SIZE, BALL_DMTR+BALL_DELTA_SIZE)) {
        LOCALS->ball_dy = -(LOCALS->ball_dy);
        return TRUE;
    }

    y = 0;
    //collision with block
    for (row=0; row < ROWS; row++) {
        x = LOCALS->block_start_x;
        for (col=0; col < COLS; col++) {
            if (LOCALS->blocks[row][col] == TRUE) {

                if (rect_intersects(x, y, LOCALS->block_width, BLOCK_HEIGHT,
                                     LOCALS->ball_x, LOCALS->ball_y, BALL_DMTR, BALL_DMTR)) {;
                    destroy_block(row, col);
                    LOCALS->ball_dy = -(LOCALS->ball_dy);
                    return TRUE;
                }

            }
            x+= LOCALS->block_width+1;
        }
        y+= BLOCK_HEIGHT+1;

    }

    return FALSE;
}


static void
breakout_thread_run() {

    struct msg_t ui_queue_array[10];
    struct msg_queue_t ui_queue;
    struct msg_t msg;
    const struct window_t* window;

    LOCALS = malloc(sizeof(struct breakout_locals_t));

    LOCALS->window.origin_x = 0;
    LOCALS->window.origin_y = 0;
    LOCALS->window.width = 240;
    LOCALS->window.height = 160;
    LOCALS->block_start_x = 8;

    LOCALS->ball_x = 0;
    LOCALS->ball_y = 0;
    LOCALS->ball_dx = BALL_DELTA_SIZE;
    LOCALS->ball_dy = -BALL_DELTA_SIZE;

    LOCALS->old_bat_x = 60;
    LOCALS->bat_x = 60;
    LOCALS->bat_y = 72;
    LOCALS->have_focus = FALSE;

    LOCALS->ready_for_display = FALSE;
    LOCALS->paused = FALSE;


    cond_lock_create(&LOCALS->screen_displayed_cond);

    snprintf(LOCALS->ui_queue_name, 32, "breakout%d", thread_current());

    msg_queue_create(&ui_queue, LOCALS->ui_queue_name, ui_queue_array, 10);

    window = register_with_uimgr(&ui_queue, breakout_draw_cb, LOCALS);
    if (!window) {
        return;
    }

    while(1) {
        msg = msg_queue_remove(&ui_queue, LOCALS->paused);
        if (msg.type == MSG_LOST_FOCUS) {
            LOCALS->have_focus = FALSE;
        }
        else if (msg.type == MSG_GOT_FOCUS) {
            LOCALS->have_focus = TRUE;
        }
        else if (msg.type == MSG_PAUSE) {
            LOCALS->paused = !LOCALS->paused;
        }
        else if (msg.type == MSG_QUIT) {
            unregister_with_uimgr(&ui_queue);
            return;
        }

        if (!LOCALS->paused) {
            move_ball();

            //we poll the button registers directly here instead of using the msg queues
            //because the button press interrupts don't autorepeat and so we would
            //only get one button press msg

            if (LOCALS->have_focus) {
                if ((~REG_KEY & BUTTON_LEFT) && (LOCALS->bat_x > BAT_DELTA_SIZE))
                    move_bat(-BAT_DELTA_SIZE);
                else if ((~REG_KEY & BUTTON_RIGHT) && (LOCALS->bat_x < (LOCALS->window.width-(BAT_WIDTH + BAT_DELTA_SIZE))))
                    move_bat(BAT_DELTA_SIZE);
            }
            check_for_collision();
            LOCALS->ready_for_display = TRUE;
            cond_lock_wait(&LOCALS->screen_displayed_cond);

        }
    }
}


//-----------------------------------------------------------------------------

static void
breakout_exited(void* cb_data) {
    struct thread_t* dead_thread = (struct thread_t*)cb_data;
    free(dead_thread->stack_end);
    free(dead_thread->thread_local_data);
}


void
start_breakout() {
    void* breakout_thread_stack = malloc(DEFAULT_STACK_SIZE);
    thread_create("breakout", breakout_thread_run, breakout_thread_stack, DEFAULT_STACK_SIZE, breakout_exited);
}
