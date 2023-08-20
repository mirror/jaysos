/* jaysos/GBA port by Justin Armstrong <ja@badpint.org> */


/* $Id: nmport_b.c,v 1.4 2001/09/02 07:24:03 monaka Exp $

Copyright (C) 1998, 1999, 2000 Wabasoft

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.
*/

/*

If you're looking here, you've probably already looked at (or worked on) the
nm<platform>_a.c file.

This file is the second step in porting the VM. In it, you need to place the
main application loop and class loader. If you get through this, you'll have
a basic VM running and from there you can implement the native functions
so you can actually see something on the screen.

The WabaVM, like other applications, needs some type of main application loop
where the OS sends it key clicks, mouse presses, etc. The VM also needs to
load classes from memory or disk or wherever.

Different platforms have different main application loops. For example, under
Win32, a program has a WinMain() function. Under PalmOS, applications have
a PilotMain() function.

You'll want to implement a main application loop for the platform you are
porting to. Then, when things start up (initialization), you need to:

- parse the launch command and then call VmInit()
- call VmStartApp passing the class name of the program's main window

You'll also need to figure out how programs will launch the VM. The VM
is normally passed parameters telling it what class heap, class name, etc.
to use. It's usually another program that does this but you could do it
by building a waba launcher that draws a tray of icons and when you click
one, it runs the WabaVM with various parameters as an example of something
different. Whatever you do for this is platform-specific.

Just before the program exits, you'll should call VmStopApp(mainWinObj);

That's it for initialization and exit, then you need to do some work to
hook up key presses, mouse presses, etc. to the VM. When a key is pressed,
you'll want to invoke the main window's onEvent() method. Here's an
example of how to do that:

	mainWinObj = globalMainWin;
	if (!mainWinObj)
		return;
	vclass = WOBJ_class(mainWinObj); // get runtime class
	method = getMethod(vclass, createUtfString("_postEvent"),
		createUtfString("(IIIIII)V"), &vclass);
	if (method != NULL)
		{
		params[0].obj = mainWinObj;
		params[1].intValue = type; // type
		params[2].intValue = key; // key
		params[3].intValue = 0; // x
		params[4].intValue = 0; // y
		params[5].intValue = 0; // modifiers
		params[6].intValue = 0; // timeStamp
		executeMethod(vclass, method, params, 7);
		}

This gets the runtime class of the main window object, gets a
reference to the _postEvent method, fills in an array of parameters
to pass and then calls executeMethod() to execute the method.

There is also a quickbind version of this which is quicker than the
above. You can look at nmpalm_b.c or nmwin32_b.c to get an idea how
that works (look for the #ifdef QUICKBIND) but you should start
without using the quickbind method.

You'll want to hook up mouse or pen click, key presses, calls from
the OS to repaint the window (which should call the main window's
_doPaint routine).

Overall, you need to make sure the following main window functions
get called:

_onTimerTick()
_postEvent() for key presses and mouse/pen clicks and moves

You'll need to map special keys (escape, etc.) if the device has
them to suitable numbers for waba. To do this, you can see how the
other native implementations do key mapping (a big switch statement).

The _onTimerTick() one can be tricky for devices that don't have
a built in timer (like PalmOS). When using one of those devices,
copy the code from the nmpalm_b.c to deal with timers. Basically,
this consists of getting the current time and looking for an event
up until the time the next timer period expires.

The VM only needs a single timer, the waba core classes have the
code to support multiple timers even though the underlying system
only supports a single one.

The last code you need to implement is:

nativeLoadClass()

This is the function that load a class from memory, disk or whatever.
It looks like this:

static u8 *nativeLoadClass(UtfString className, u32 *size)

and it is passed a class name (not zero terminated, the UtfString
contains the len in:

className.len

and it returns a pointer to the class in memory after it is loaded
and fills in the *size variable with the size of the class in bytes.

The classes loaded are classes that are compiled by a java compiler
or other compiler that can generate code in the subset of java
bytecode that waba supports.

*/

/* all the needed include */
#include "../waba.h"

#ifdef WITH_THREAD

/* PORT: If you wanna use thread, you need to define function follows
 * so that they are required by waba.c
 */

// *** This method is called from "start()" method.
Var ThreadStart(Var stack[])
{
}

// *** This method is called from "stop()" method.
Var ThreadStop(Var stack[])
{
}

// *** This method is called from "sleep()" method.
Var ThreadSleep(Var stack[])
{
}

// *** This method is called from "wait()" method.
Var ThreadWaitForSignal(Var stack[])
{
}

Var ThreadSignalAll(Var stack[])
{
}

// *** This "ThreadCurrentThread()" method is called from "curentThread()" method.
Var ThreadCurrentThread(Var stack[])
{
  Var v;
  v.obj = 0;
  return v;
}

void tickThread(void)
{
}


int registerAsSyncronized(WClassMethod* method)
{
}

void exitFromSyncronized(WClassMethod* method)
{
}


int enterMonitor(WObject obj)
{
}


int exitMonitor(WObject obj)
{
}

#endif

extern const struct window_t* s_main_win_native;
WObject globalMainWin = 0;

extern WClass *mainWinClass;
extern WClass *imageClass;

#ifdef QUICKBIND
static int32 postEventMethodMapNum = -1;
static int32 onTimerTickMethodMapNum = -1;
#endif





/* Calls the _doPaint method of the main window.
 */
/*static*/ void drawMainWin()
{
    WObject mainWinObj;
    WClass *vclass;
    WClassMethod *method;
    Var params[7];

    mainWinObj = globalMainWin;
    if (!mainWinObj)
	return;
    vclass = WOBJ_class(mainWinObj);	// get runtime class
    method = getMethod(vclass, createUtfString("_doPaint"),
		       createUtfString("(IIII)V"), &vclass);
    if (method == NULL)
	return;


    params[0].obj = mainWinObj;
    params[1].intValue = 0;	// x
    params[2].intValue = 0;	// y
    params[3].intValue = s_main_win_native->width  ;	// width
    params[4].intValue = s_main_win_native->height;	// height
    executeMethod(vclass, method, params, 5);

}






//------------------------------------------------------------------

bool waba_die;

const GBFS_ARCHIVE* gbfs_archive = NULL;


u8 *nativeLoadClass(UtfString className, u32 *size) {

    u32 len;
    const void* data;
    char path[MAX_GBFS_PATH];


    if (!gbfs_archive) {
        gbfs_archive = find_first_gbfs_archive((void*)0x8000000);
        if (!gbfs_archive) {
            panic("no gbfs archive found!");
            return NULL;
        }
    }


    // make full path by appending .class
    if (className.len + 7 > MAX_GBFS_PATH)
        return NULL;
    xstrncpy(path, className.str, className.len);
    xstrncpy(&path[className.len], ".class", 7);

    data = gbfs_get_obj(gbfs_archive, path, &len);

    if (!data) {
        printf("file not found: <%s>\r\n", path);
        waba_die = TRUE;
    }

    return (u8*)data;

}

//------------------------------------------------------------------


//------------------------------------------------------------------

extern const struct window_t* s_main_win_native;
extern const struct window_t* s_offscreen_win_native;

static struct cond_lock_t s_screen_displayed_cond;

static volatile bool s_ready_for_display;

void
waba_draw_cb(struct window_t window, bool redraw_all, void* cb_data)  {

    if (!s_ready_for_display) return;

    if (s_offscreen_win_native != s_main_win_native) {
        //we are using an offscreen buffer

        copy_window_unscaled(s_main_win_native, 0, 0, s_offscreen_win_native);
        s_ready_for_display = FALSE;
        cond_lock_signal(&s_screen_displayed_cond);
    }

}


struct msg_queue_t waba_ui_msg_queue;
int waba_timer_interval;


static bool s_waba_running = FALSE;
static char s_classname[256];

void
free_offscreen();
void
main_win_has_resized();




void
waba_init_thread_run() {
    int waba_event = 0;
    int key = 0;
    WClass*  vclass;
    WClassMethod* method;


    struct msg_t msg;
    struct msg_t waba_ui_queue_array[20];

    u32 vmStackSize, nmStackSize, classHeapSize, objectHeapSize;


    // defaults
    vmStackSize     = 1500;
    nmStackSize     = 300;
    classHeapSize   = 14000;
    objectHeapSize  = 8000;


    //we have to reset all globals in case waba has been run before
    globalMainWin = 0;
    s_main_win_native = NULL;
    s_offscreen_win_native = NULL;
    waba_timer_interval = 0;
    mainWinClass = NULL;
    imageClass = NULL;
    waba_die = FALSE;
#ifdef QUICKBIND
    postEventMethodMapNum = -1;
    onTimerTickMethodMapNum = -1;
#endif

    s_ready_for_display = FALSE;
    cond_lock_create(&s_screen_displayed_cond);

    msg_queue_create(&waba_ui_msg_queue, "waba", waba_ui_queue_array, 20);

    VmInit(vmStackSize, nmStackSize, classHeapSize, objectHeapSize);


    VmStartApp(s_classname);

    if (waba_die) {
        VmFree();
        s_waba_running = FALSE;
        return;
    }

    drawMainWin();

#ifdef QUICKBIND
    if (globalMainWin != 0) {
        WClass *vclass;

        // cache method map numbers for commonly called methods
        vclass = WOBJ_class(globalMainWin);

        postEventMethodMapNum =
          getMethodMapNum( vclass, createUtfString("_postEvent"),
                           createUtfString("(IIIIII)V"), SEARCH_ALL);
        onTimerTickMethodMapNum =
            getMethodMapNum( vclass, createUtfString("_onTimerTick"),
                             createUtfString("()V"), SEARCH_ALL);

        if (postEventMethodMapNum == -1 || onTimerTickMethodMapNum == -1)
            globalMainWin = 0;
    }

#endif

    while(!waba_die) {


        msg = msg_queue_remove(&waba_ui_msg_queue, FALSE);

        if (msg.type == MSG_LOST_FOCUS) {

        }
        else if (msg.type == MSG_GOT_FOCUS) {

        }
        else if (msg.type == MSG_PAUSE) {

        }
        else if (msg.type == MSG_RESIZE) {
            main_win_has_resized();
        }
        else if (msg.type == MSG_QUIT) {
            //printf("MSG_QUIT\r\n");
            break;
        }
        else if (msg.type == MSG_BUTTON_PRESS) {
            Var params[7];

            //printf("!");
            if (~msg.m.button & BUTTON_A) {
                waba_event = WABA_EVENT_PEN_DOWN;
            }
            else {
                waba_event = WABA_EVENT_KEY_PRESS;

                if (~msg.m.button & BUTTON_LEFT)
                    key = WABA_KEY_LEFT;
                else if (~msg.m.button & BUTTON_RIGHT)
                    key = WABA_KEY_RIGHT;
                else if (~msg.m.button & BUTTON_UP)
                    key = WABA_KEY_UP;
                else if (~msg.m.button & BUTTON_DOWN)
                    key = WABA_KEY_DOWN;

            }

            vclass = WOBJ_class(globalMainWin); // get runtime class

#ifdef QUICKBIND
            method = getMethodByMapNum( vclass,
                                        &vclass,
                                        postEventMethodMapNum);
#else
            method = getMethod( vclass,
                    createUtfString("_postEvent"),
                    createUtfString("(IIIIII)V"),
                    &vclass);
#endif
            if(method != NULL)
            {
                params[0].obj       = globalMainWin;
                params[1].intValue  = waba_event; // type
                params[2].intValue  = key; // key
                params[3].intValue  = 0; // x
                params[4].intValue  = 0; // y
                params[5].intValue  = 0; // modifiers
                params[6].intValue  = 0; // timeStamp
                executeMethod(vclass, method, params, 7);
            }


        }
        else if (msg.type == MSG_ALARM) {
            Var params[1];
            //printf(".");
            vclass = WOBJ_class(globalMainWin); // get runtime class
#ifdef QUICKBIND
            method = getMethodByMapNum( vclass, &vclass,
                                        (u16)onTimerTickMethodMapNum );
#else
            method = getMethod(vclass, createUtfString("_onTimerTick"),
                                createUtfString("()V"), &vclass);
#endif
            if (method != NULL) {
              params[0].obj = globalMainWin;
              executeMethod(vclass, method, params, 1);
            }

        }

#ifdef JAYSOS_DOUBLE_BUFFER
        s_ready_for_display = TRUE;

        if (s_offscreen_win_native != s_main_win_native)
            cond_lock_wait(&s_screen_displayed_cond);
#endif

    }

    unregister_with_uimgr(&waba_ui_msg_queue);
    free_offscreen();

    VmFree();
    s_waba_running = FALSE;

}




static u8 s_waba_init_thread_stack[DEFAULT_STACK_SIZE*2];


void
waba_exited(void* cb_data) {

}


void
start_waba(char* classname) {
    if (s_waba_running) return;

    if ((classname) && (*classname))
        strncpy(s_classname, classname, 255);
    else strncpy(s_classname, "waba/ui/Welcome", 255);

    thread_create("waba", waba_init_thread_run, s_waba_init_thread_stack, DEFAULT_STACK_SIZE*2, NULL);
    s_waba_running = TRUE;
}
