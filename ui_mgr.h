#ifndef UI_MGR_H
#define UI_MGR_H

#include "kernel.h"
#include "msg_queue.h"
#include "interrupts.h"

#define BUTTON_A 1
#define BUTTON_B 2
#define BUTTON_SELECT 4
#define BUTTON_START 8
#define BUTTON_RIGHT 16
#define BUTTON_LEFT 32
#define BUTTON_UP 64
#define BUTTON_DOWN 128
#define BUTTON_SHOULDER_RIGHT 256
#define BUTTON_SHOULDER_LEFT 512


#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define MAX_WINDOWS 4



/*
 * a window_t is a rectangular area of a a video buffer
 * (either the hardware display or an offscreen buffer)
 * buffer_width and buffer_height describe the size of this
 * buffer.
 *
 * origin_x and origin_y specify the offset from the edge 
 * of the buffer..
 * width and height are relative to origin_x and origin_y
 * The origin_x, origin_Y, width and height values
 * determine the area that drawing operations are constrained
 * to stay within.
 * 
 * width + origin_x must be <= buffer_width and
 * height + origin_y must be <= buffer_height
 *
 *
 */


struct window_t {
    u16 origin_x;
    u16 origin_y;
    u16 width;
    u16 height;

    volatile u16* video_buffer;
    u16 buffer_width;
    u16 buffer_height;

};

typedef void (*draw_callback_t) (struct window_t,  bool, void*); 


void
init_uimgr();

void
button_press_isr();

struct msg_queue_t*
get_ui_queue_by_name(const char* name);

void
print_ui_queue_names();

bool
post_ui_msg(struct msg_queue_t* queue, struct msg_t msg);

void
draw_pixel(const struct window_t* window, int x, int y, int colour);

void
fill_rect(const struct window_t* window, 
          int x, int y, int width, int height, int colour);

void
draw_rect(const struct window_t* window,
          int x, int y, int width, int height, int thickness, int colour);

void 
draw_string(const struct window_t* window, unsigned const char* c, int x, int y, int colour);

void 
draw_line(const struct window_t* window, int x0, int y0, int x1, int y1, int colour);


void
copy_window_unscaled(const struct window_t* dest_win, int x, int y, const struct window_t* src_win);

void
copy_window(const struct window_t* dest_win, const struct window_t* src_win);

const char* 
itoa(int i, int minlength);

void
panic(char* str);

const struct window_t*
register_with_uimgr(struct msg_queue_t* queue, draw_callback_t draw_cb, void* cb_data);

void
unregister_with_uimgr(struct msg_queue_t* queue);

bool
ui_alarm_add(struct msg_queue_t* queue, u32 centisecs_from_now, bool reocurring);

#define PUTPIXEL(x, y, c) s_video[(y * 240) + x] = c;
#endif 
