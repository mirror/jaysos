/* jaysos/GBA port by Justin Armstrong <ja@badpint.org> */


/* $Id: nmport_c.c,v 1.8 2002/01/24 17:02:14 cgrigis Exp $

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
/* This file is modified by Monaka (monamour@monaka.org) */

#ifdef COMMENT
/*

If you're looking here, you've probably already looked at (or worked on) the
nm<platform>_b.c file. If you've gotten the _a and _b files working, this is
the easy part. This file contains all the native functions that are part of
the waba core framework. When you've added all the native functions then you're
done porting the VM.

The native functions that need to be ported are all those listed in the waba.c
native function table. There are stubs included in this file to make things
a bit easier than starting from scratch.

Some rules:

As a rule for native functions, don't hold a pointer across any call
that could garbage collect. For example, this is bad:

ptr = WOBJ_arrayStart(array)
...
string = createString(..)
ptr[0]

since the createString() could GC, the ptr inside of array could be invalid
after the call since a GC would move memory around. Instead, use:

ptr = WOBJ_arrayStart(array)
...
string = createString(..)
...
ptr = WOBJ_arrayStart(array)
ptr[0]

to recompute the pointer after the possible GC. The main thing to know there is
that when you call VM functions, it might garbage collect, so you want to 
recompute any pointers again after that function returns since the memory
locations might have changed. When you don't call VM functions that garbage
collect, there isn't a problem, because the VM is all single threaded. Things
don't move around on you unless you call a function that can garbage collect.

Another thing to note is that if you subclass a class with an object
destroy function, you must explicity call your superclasses objects
destroy functions. This isn't done automatically. See one of the objects that does
that in nmwin32_c.c for reference.

Before jumping into writing native functions, we need to look at 'classHooks'.
A classHooks array contains 'class hooks' like this:

ClassHook classHooks[] =
	{
	{ "waba/fx/Graphics", NULL, 11},
	{ "waba/fx/Image", ImageDestroy, 1 },
#if WITH_CATALOG_CLASS
	{ "waba/io/Catalog", CatalogDestroy, 7 },
#endif
	{ "waba/io/Socket", SocketDestroy, 2 }, 
	{ "waba/io/SerialPort", SerialPortDestroy, 2 }, 
	{ NULL, NULL }
	};

You will need to define a classHooks array. Its an array of triples:

- a class name
- a object destroy function
- a number of native variables

You need a class hook when an object needs to allocate some native system
resource which it needs to use later on and/or needs to be freed.

For example, a Socket object will probably need to keep around a reference
to a socket descriptor around. And when a socket object is garbage
collected, it should close the socket.

So, it needs a classHook. In the above, the destroy method for a socket
is SocketDestroy and it will get called when the object is garbage
collected. The classHook for the Socket class above has 2 native
variables associated with it. Each of these native variables
are 32 bit values. One could hold a socket descriptor and the
other could hold something else. You can see nmpalm_c.c or nmwin32_c.c
for more on how that works for sockets.

So, how do you access a class hook variable or a variable that is in
a waba object? You can access a waba object's variables directly like this:

#define WOBJ_RectX(o) (objectPtr(o))[1].intValue

The objectPtr(o) gets a pointer to the data in an object and the [1]
references the second value in the object. The first value is a pointer
to the class of the object so the WOBJ_RectX(o) above is a macro that
pulls out the first actual variable in the object which is its X value.

If you look in the Rect class, you'll see that it looks like:

class Rect
{
int x;
int y;
..

So, the first variable - (objectPtr(o))[1].intValue is the x value.

The hook variables get stuck in after all the normal variables. If you look in the
nmwin32_c.c or nmpalm_c.c code, you'll see things like this:

//
// Graphics
//
// var[0] = Class
// var[1] = Surface
// var[2] = hook var - 1 for window surface and 2 for image
// var[3] = hook var - rgb
// var[4] = hook var - has clip
// var[5] = hook var - clipX
// ...

and then:

#define WOBJ_GraphicsSurface(o) (objectPtr(o))[1].obj
#define WOBJ_GraphicsSurfType(o) (objectPtr(o))[2].intValue
#define WOBJ_GraphicsRGB(o) (objectPtr(o))[3].intValue
#define WOBJ_GraphicsHasClip(o) (objectPtr(o))[4].intValue
#define WOBJ_GraphicsClipX(o) (objectPtr(o))[5].intValue
#define WOBJ_GraphicsClipY(o) (objectPtr(o))[6].intValue
#define WOBJ_GraphicsClipWidth(o) (objectPtr(o))[7].intValue

See how the hook variables start right after the last variable in the
class. The above are macros that access the value of a waba object in C code.
Making macros like the above makes the code easier to read.

With the macros, we can inspect a graphic's object clipX with:

 x = WObjectGraphicClipX(object);

Of course, if you change the Graphics object and add a new variable before the
surface variable or if you add a new variable in a base class, you need to
recompute those mappings or everything will get messed up.

The best way to port the native functions is to start out by making something
that doesn't work but does compile and then take code from the other native
VM's implementations and hack it up one class at a time. It's probably best
to start with the window and drawing classes so you can see something when
you start out.

*/
#endif

#include "../waba.h"


extern struct msg_queue_t waba_ui_msg_queue;

WClass *mainWinClass = 0;
WClass *imageClass = 0;





// WHEN PORTING: You'll probably need classHooks for these things when you get
// things going. Here we've allocate no hook variables for each object but have
// assigned some object destructor functions so when you actually hook some
// data in there, you'll need to set the 0 values to something else


void ImageDestroy(WObject obj);

ClassHook classHooks[] =
{
    { "waba/fx/Graphics", NULL, 11 },
    { "waba/fx/Image", ImageDestroy, 1 }, 
    { NULL, NULL }
};


//
// Rect
//
// var[0] = Class
// var[1] = int x
// var[2] = int y
// var[3] = int width
// var[4] = int height

#define WOBJ_RectX(o) (objectPtr(o))[1].intValue
#define WOBJ_RectY(o) (objectPtr(o))[2].intValue
#define WOBJ_RectWidth(o) (objectPtr(o))[3].intValue
#define WOBJ_RectHeight(o) (objectPtr(o))[4].intValue

//
// Control
//
// var[0] = Class
// var[1] = int x
// var[2] = int y
// var[3] = int width
// var[4] = int height

#define WOBJ_ControlX(o) (objectPtr(o))[1].intValue
#define WOBJ_ControlY(o) (objectPtr(o))[2].intValue
#define WOBJ_ControlWidth(o) (objectPtr(o))[3].intValue
#define WOBJ_ControlHeight(o) (objectPtr(o))[4].intValue

//
// Window
//

Var WindowCreate(Var stack[]) {
    Var v;
    //printf("WindowCreate\r\n");
    v.obj = 0;
    return v;
}

//
// MainWindow
//


extern WObject globalMainWin;

const struct window_t* s_main_win_native;
const struct window_t* s_offscreen_win_native;


static const struct window_t*
_make_offscreen() {

#ifdef JAYSOS_DOUBLE_BUFFER

   
    void* offscreen_buf;  
    struct window_t* window;
    
    offscreen_buf = xmalloc((s_main_win_native->width+1) * (s_main_win_native->height+1) * 2);
    if (!offscreen_buf) {
        //no mem for offscreen buffer? - write direct to the screen!
        //printf("!offscreen_buf\r\n");
        return s_main_win_native;
    }
    else {
        window = xmalloc(sizeof(struct window_t));
        if (!window) {
            xfree(offscreen_buf);
            //printf("!window\r\n");
            return s_main_win_native;
        }
        else {
            window->video_buffer = offscreen_buf;
            window->origin_x = 0;
            window->origin_y = 0;
            window->width = s_main_win_native->width;
            window->height = s_main_win_native->height;
            window->buffer_width = window->width;
            window->buffer_height = window->height;
        
        }
    }
    
    //printf("allocated window=%p s_main_win_native=%p\r\n", window, s_main_win_native);
    //printf("x=%d y=%d width=%d height=%d\r\n", window->origin_x, window->origin_y, window->width, window->height);
    //printf("buffer_width=%d buffer_height=%d\r\n", window->buffer_width, window->buffer_height);

    
    return window;
#else
    
  return s_main_win_native; 

#endif

}


void
free_offscreen() {

    if (s_offscreen_win_native != s_main_win_native) {
        //we are using an offscreen buffer
        //printf("freeing %p\r\n", s_offscreen_win_native);
        xfree((void*)s_offscreen_win_native->video_buffer);
        xfree((void*)s_offscreen_win_native);        
        
    }



}

void
main_win_has_resized() {

    WClassMethod* method;
    Var params[5];

    free_offscreen();
    
    //printf("main_win_has_resized \r\n");   
    
    s_offscreen_win_native = _make_offscreen();
    
    WOBJ_ControlX(globalMainWin)      = s_offscreen_win_native->origin_x;
    WOBJ_ControlY(globalMainWin)      = s_offscreen_win_native->origin_y;
    WOBJ_ControlWidth(globalMainWin)  = s_offscreen_win_native->width;
    WOBJ_ControlHeight(globalMainWin) = s_offscreen_win_native->height;
    

    method = getMethod( mainWinClass,
          createUtfString("_doPaint"),
          createUtfString("(IIII)V"),
          &mainWinClass);

    if(method != NULL)
    {
      params[0].obj       = globalMainWin;
      params[1].intValue  = s_offscreen_win_native->origin_x;
      params[2].intValue  = s_offscreen_win_native->origin_y;
      params[3].intValue  = s_offscreen_win_native->width;
      params[4].intValue  = s_offscreen_win_native->height;

      executeMethod(mainWinClass, method, params, 5);
    }


}

void
waba_draw_cb(struct window_t window, bool redraw_all, void* cb_data);





Var MainWinCreate(Var stack[]) {

    Var v;
    WObject wabaWin;
    
#ifdef JAYSOS_DOUBLE_BUFFER        
    s_main_win_native = register_with_uimgr(&waba_ui_msg_queue, waba_draw_cb, NULL);
#else
    s_main_win_native = register_with_uimgr(&waba_ui_msg_queue, NULL, NULL);
#endif
    
    s_offscreen_win_native = _make_offscreen();
    
    //printf("MainWinCreate s_main_win_native=%p\r\n", s_offscreen_win_native);

    wabaWin = stack[0].obj;

    WOBJ_ControlX(wabaWin)      = s_offscreen_win_native->origin_x;
    WOBJ_ControlY(wabaWin)      = s_offscreen_win_native->origin_y;
    WOBJ_ControlWidth(wabaWin)  = s_offscreen_win_native->width;
    WOBJ_ControlHeight(wabaWin) = s_offscreen_win_native->height;

    globalMainWin = wabaWin;
        
    v.obj = 0;
    return v;
}


extern bool waba_die;

Var MainWinExit(Var stack[]) {
    Var v;
    
    waba_die = TRUE;
        
    v.obj = 0;
    return v;
}


extern int waba_timer_interval;


Var MainWinSetTimerInterval(Var stack[]) {

    Var v;
    waba_timer_interval = stack[1].intValue/10;
    ui_alarm_add(&waba_ui_msg_queue, waba_timer_interval, FALSE);
    //printf("MainWinSetTimerInterval waba_timer_interval=%d\r\n", waba_timer_interval);    

    v.obj = 0;
    return v;
}

//
// Surface
//

#define SURF_MAINWIN 1
#define SURF_IMAGE 2



int SurfaceGetType(WObject surface) {
    WClass *wclass;

    if (surface == 0)
        return 0;

    // cache class pointers for performance
    if (!mainWinClass)
        mainWinClass = getClass(createUtfString("waba/ui/MainWindow"));
    if (!imageClass)
        imageClass = getClass(createUtfString("waba/fx/Image"));

    wclass = WOBJ_class(surface);
    if (compatible(wclass, mainWinClass))
        return SURF_MAINWIN;
    if (compatible(wclass, imageClass))
        return SURF_IMAGE;
    return 0;
}

//
// Font
//
// var[0] = Class
// var[1] = String name
// var[2] = int size
// var[3] = int style
//

#define WOBJ_FontName(o) (objectPtr(o))[1].obj
#define WOBJ_FontStyle(o) (objectPtr(o))[2].intValue
#define WOBJ_FontSize(o) (objectPtr(o))[3].intValue 
#define Font_PLAIN 0
#define Font_BOLD 1

//
// FontMetrics
//

#define WOBJ_FontMetricsFont(o) (objectPtr(o))[1].obj
#define WOBJ_FontMetricsSurface(o) (objectPtr(o))[2].obj
#define WOBJ_FontMetricsAscent(o) (objectPtr(o))[3].intValue
#define WOBJ_FontMetricsDescent(o) (objectPtr(o))[4].intValue
#define WOBJ_FontMetricsLeading(o) (objectPtr(o))[5].intValue


Var FontMetricsCreate(Var stack[]) {
    Var v;
        
    WObject fontMetrics = stack[0].obj;
    WOBJ_FontMetricsAscent(fontMetrics) = 8;
    WOBJ_FontMetricsDescent(fontMetrics) = 0;
    WOBJ_FontMetricsLeading(fontMetrics) = 0;

    v.obj = 0;
    return v;
}

#define FM_STRINGWIDTH 1
#define FM_CHARARRAYWIDTH 2
#define FM_CHARWIDTH 3

Var FontMetricsGetWidth(int type, Var stack[]) {

    WObject string, charArray;
    int width, count;
    Var v;

    width = 0;

    switch( type ) {

        case FM_STRINGWIDTH:

            /* we want the width of the string */
        
            /* get the string from the stack */
            string = stack[1].obj;
            if (string == 0)
                /* no string */
                break;

            /* convert the string to an "array of char" */
            charArray = WOBJ_StringCharArrayObj(string);
            if (charArray == 0)
                /* no string */
                break;

            /* get the len of the string */
            count = WOBJ_arrayLen(charArray);

            width = count*6;

            break;

        case FM_CHARWIDTH:
        {

            /* we want the width of a single char */

            width = 6;

        }
        break;

        default:
            width = 0;

    }


    /* return the wanted value */
    v.intValue = width;
    return v;
}

Var FontMetricsGetStringWidth(Var stack[]) {
    return FontMetricsGetWidth(FM_STRINGWIDTH, stack);
}

Var FontMetricsGetCharArrayWidth(Var stack[]) {
    return FontMetricsGetWidth(FM_CHARARRAYWIDTH, stack);
}

Var FontMetricsGetCharWidth(Var stack[]) {
    return FontMetricsGetWidth(FM_CHARWIDTH, stack);
}

//
// Image
//

#define WOBJ_ImageWidth(o) (objectPtr(o))[1].intValue
#define WOBJ_ImageHeight(o) (objectPtr(o))[2].intValue
#define WOBJ_ImageBuffer(o) (objectPtr(o))[3].refValue



/* Intel-architecture getUInt32 and getUInt16 */
/* This have to do with BIG ENDIAN/LO ENDIAN  */
#define inGetUInt32(b) (u32)( (u32)((b)[3])<<24 | \
                       (u32)((b)[2])<<16 | \
                       (u32)((b)[1])<<8  | \
                       (u32)((b)[0]) )
                       
#define inGetUInt16(b) (u16)( (u16)((b)[1])<<8  | \
                       (u16)((b)[0]) )

#define RGB24TO16(R,G,B) ( ((R/8)<<10) | ((G/8)<<5) | (B/8) )

/* taken from the linux port of waba */

/* implementation of function for loading a "windows" bitmap file         */
/* WARNING ! a "windows" bitmap file is not the same as a X11 bitmap file */
void ImageLoadBMP( WObject image, const u8 *p, u32 sizeInMemory ) {

    u32 bitmapOffset, infoSize, width, height, bpp;
    u32 i, x, y, compression, numColors, scanlen;
    u16 *rgbbuf;
    u16 *pos;
    u8 *b;
    u8 *bsave;
    s32 bit;
    u8 startMask;
    u8 currentMask;
    s32 currentMaskShift;
    u32 nbUsedColors;
    u16 *theUsedColors;
    u16 *c;

    //printf("ImageLoadBMP p=%p  size=%d\r\n", p, sizeInMemory);

    // content of a "windows" bitmap file :
    // header (54 bytes)
    // 0-1   magic chars 'BM'
    // 2-5   u32 filesize (not reliable)
    // 6-7   u16 0
    // 8-9   u16 0
    // 10-13 u32 bitmapOffset
    // 14-17 u32 info size
    // 18-21 s32  width
    // 22-25 s32  height
    // 26-27 u16 nplanes
    // 28-29 u16 bits per pixel
    // 30-33 u32 compression flag
    // 34-37 u32 image size in bytes
    // 38-41 s32  biXPelsPerMeter
    // 32-45 s32  biYPelsPerMeter
    // 46-49 u32 colors used
    // 50-53 u32 important color count

    if (p[0] != 'B' || p[1] != 'M')
        return; // not a BMP file -> we don't load it

    
    bitmapOffset = inGetUInt32(&p[10]);
    infoSize = inGetUInt32(&p[14]);
    if (infoSize != 40)
        return; // old-style BMP -> we don't load it

    width = inGetUInt32(&p[18]);
    height = inGetUInt32(&p[22]);
    //printf("image width=%d height=%d\r\n", width, height);
    if (width > 65535 || height > 65535)
        return; // bad width/height -> we don't load it

    

    bpp = inGetUInt16(&p[28]);
    //printf("image depth=%d\r\n", bpp);
    if (bpp != 1 && bpp != 4 && bpp != 8)
        return; // not a 2, 16 or 256 color image -> we don't load it

    

    compression = inGetUInt32(&p[30]);
    if (compression != 0)
        return; // compressed image -> we don't load it

    //printf("not compressed\r\n");

    numColors = 1 << bpp;
    scanlen = ((width * bpp) + 7) / 8; // # bytes
    scanlen = ((scanlen + 3) / 4) * 4; // end on 32 bit boundary

    // colormap
    //
    // 0-3 u32 col[0]
    // 4-7 u32 col[1]
    // ...
    nbUsedColors = inGetUInt32(&p[46]) > 0 ? inGetUInt32(&p[46]) : numColors;

    theUsedColors = xmalloc( 2 * nbUsedColors );
    if (!theUsedColors) {
        printf("couldn't allocate palette\r\n");
        return;
    }
    
    c = theUsedColors;
    b = (u8 *)&p[54];
    
    
    for( i = 0; i < nbUsedColors; i++ ) {
        u8 red = *b++;
        u8 green = *b++;
        u8 blue = *b++;
        *c++ =   RGB24TO16(red, green, blue);
	b++;   /* skip the non-used parameter */
    }

    /* allocate the buffer to put the image */
    rgbbuf = xmalloc( sizeof( u16 ) * width * height );

    if (!rgbbuf) {
        printf("couldn't allocate buffer\r\n");
        xfree(theUsedColors);
        return;
    
    }
    //printf("buffer=%p\r\n", rgbbuf);

    /* Set up the RGB buffer. */
    b = (u8 *)&p[bitmapOffset];
    bsave = b;


    bit = 0;

    startMask = 0;
    for( i = 0; i < bpp; i++ ) {
        startMask = startMask >> 1;
        startMask |= 0x80;
    }
    currentMask = startMask;
    currentMaskShift = 0;
    
    pos = rgbbuf + width * ( height - 1 );  /* start at last line, because the image is inversed */

    for( y = 0; y < height; y++ ) {
        for( x = 0; x < width; x++ ) {


            u8 current;
            int tmpIndex;

            current = *b & currentMask;
            currentMask = currentMask >> bpp;
            currentMaskShift += bpp;

            tmpIndex = current >> ( 8 - currentMaskShift );
            
            *pos++ = theUsedColors[ tmpIndex ];
	    
            if( currentMask == 0 ) {

                currentMask = startMask;
                currentMaskShift = 0;
                b++;

                if( b > p + sizeInMemory ) {
                    /* problem : we need more data, but the buffer in memory don't have more */
                    /*          the original file is maybe corrupted => stop the load       */
                    return;
                }
            }
        }

        pos -= 2 * width;
        currentMask = startMask;
        currentMaskShift = 0;

        // go to the next line
        bsave += scanlen;
        b = bsave;

    }

    WOBJ_ImageBuffer( image ) = rgbbuf;

    /* release memory */
    xfree( theUsedColors );

    /* update the variable of the Image class */
    WOBJ_ImageWidth(image) = width;
    WOBJ_ImageHeight(image) = height;

}

extern GBFS_ARCHIVE* gbfs_archive;

Var ImageLoad(Var stack[]) {

    WObject image = stack[0].obj;
    WObject pathObj = stack[1].obj;
    UtfString pathStr = stringToUtf(pathObj, STU_USE_STATIC | STU_NULL_TERMINATE);
    const void* data;
    Var v;
    u32 len;
    
    v.obj = 0;
    //printf("pathStr.len=%d, pathStr.str=<%s>\r\n", pathStr.len, pathStr.str);
    if(pathStr.len == 0)
        return v;

    gc();   //this will allow us to free up any stale images


    data = gbfs_get_obj(gbfs_archive, pathStr.str, &len);

    if (!data) {
        printf("file not found: <%s>\r\n", pathStr.str);
        return v;
    }

    /* init the variable of the class */
    WOBJ_ImageBuffer(image) = NULL;
    WOBJ_ImageWidth(image) = 0;
    WOBJ_ImageHeight(image) = 0;    
    
    ImageLoadBMP(image, (const u8*)data, len);
    
    return v;   
    
}

Var ImageCreate(Var stack[]) {
    WObject image;
    Var v;
    int width, height;
	
    gc();
    
    image   = stack[0].obj;
    //printf("ImageCreate before\r\n");
    width   = WOBJ_ImageWidth(image);
    height  = WOBJ_ImageHeight(image);        
    WOBJ_ImageBuffer(image) = malloc(width*height*2);
    //printf("ImageCreate after\r\n");
    v.obj = 0;
    return v;
}

void ImageDestroy(WObject image) {

    //printf("ImageDestroy\r\n");
    free(WOBJ_ImageBuffer(image));
    WOBJ_ImageBuffer(image) = NULL;
}

//what is this used for???
Var ImageFree(Var stack[]) {
    Var v;
    //printf("ImageFree\r\n");
    v.obj = 0;
    return v;
}



Var ImageSetPixels(Var stack[]) {
    Var v;
    //printf("ImageSetPixels\r\n");
    v.obj = 0;
    return v;
}



/* implementation of native waba/fx/Image.useImagePalette() */
Var ImageUseImagePalette( Var stack[] ) {
    Var v;

    /* return nothing */
    v.obj = 0;
    return v;

}


//
// Graphics
//

#define WOBJ_GraphicsSurface(o)     (objectPtr(o))[1].obj
#define WOBJ_GraphicsHasClip(o)     (objectPtr(o))[2].intValue
#define WOBJ_GraphicsClipX(o)       (objectPtr(o))[3].intValue
#define WOBJ_GraphicsClipY(o)       (objectPtr(o))[4].intValue
#define WOBJ_GraphicsClipWidth(o)   (objectPtr(o))[5].intValue
#define WOBJ_GraphicsClipHeight(o)  (objectPtr(o))[6].intValue
#define WOBJ_GraphicsDrawOp(o)      (objectPtr(o))[7].intValue
#define WOBJ_GraphicsTransX(o)      (objectPtr(o))[8].intValue
#define WOBJ_GraphicsTransY(o)      (objectPtr(o))[9].intValue
#define WOBJ_GraphicsColor(o)       (objectPtr(o))[10].intValue


#define GR_FILLRECT   0
#define GR_DRAWLINE   1
#define GR_FILLPOLY   2
#define GR_DRAWCHARS  3
#define GR_DRAWSTRING 4
#define GR_DOTS       5
#define GR_COPYRECT   6
#define GR_DRAWCURSOR 7

#define DRAW_OVER   1
#define DRAW_AND    2
#define DRAW_OR     3
#define DRAW_XOR    4


Var GraphicsCreate(Var stack[]) {
    Var     v;
    WObject gr, surface;

    gr = stack[0].obj;

    surface = WOBJ_GraphicsSurface(gr);

    WOBJ_GraphicsHasClip(gr)    = 0;
    WOBJ_GraphicsDrawOp(gr)     = DRAW_OVER;
    WOBJ_GraphicsTransX(gr)     = 0;
    WOBJ_GraphicsTransY(gr)     = 0;

    v.obj = 0;

    return v;
}


Var GraphicsSetFont(Var stack[]) {
    Var v;

    v.obj = 0;
    return v;
}

Var GraphicsSetColor(Var stack[]) {
    WObject gr;
    Var v;

    gr = stack[0].obj;
        
    WOBJ_GraphicsColor(gr) = RGB24TO16(stack[1].intValue, stack[2].intValue, stack[3].intValue);
    v.obj = 0;
    return v;
}

/* implementation of native waba/fx/Graphics.setTextColor() */
Var GraphicsSetTextColor( Var stack[] ) {
    Var v;
    
    v.obj = 0;
    return v;

}


Var GraphicsSetDrawOp(Var stack[]) {
    WObject gr;
    Var v;

    gr = stack[0].obj;
    WOBJ_GraphicsDrawOp( gr ) = stack[1].intValue;
        
    v.obj = 0;
    return v;
}

Var GraphicsSetClip(Var stack[]) {

    WObject gr;
    int   transX, transY;
    Var     v;

    gr = stack[0].obj;

    transX = WOBJ_GraphicsTransX(gr);
    transY = WOBJ_GraphicsTransY(gr);

    WOBJ_GraphicsHasClip(gr)    = 1;

    // clip X and Y are stored in absolute coordinates
    WOBJ_GraphicsClipX(gr)      = stack[1].intValue + transX;
    WOBJ_GraphicsClipY(gr)      = stack[2].intValue + transY;
    WOBJ_GraphicsClipWidth(gr)  = stack[3].intValue;
    WOBJ_GraphicsClipHeight(gr) = stack[4].intValue;

    v.obj = 0;

    return v;
}

Var GraphicsGetClip(Var stack[]) {
    WObject gr, rect;
    Var     v;

    v.obj = 0;

    gr      = stack[0].obj;
    rect    = stack[1].obj;

    if(rect == 0 || WOBJ_GraphicsHasClip(gr) != 1)
        return v;

    WOBJ_RectX(rect)        = WOBJ_GraphicsClipX(gr) - WOBJ_GraphicsTransX(gr);
    WOBJ_RectY(rect)        = WOBJ_GraphicsClipY(gr) - WOBJ_GraphicsTransY(gr);
    WOBJ_RectWidth(rect)    = WOBJ_GraphicsClipWidth(gr);
    WOBJ_RectHeight(rect)   = WOBJ_GraphicsClipHeight(gr);

    v.obj = rect;

    return v;
}

Var GraphicsClearClip(Var stack[]) {
    WObject gr;
    Var     v;

    gr = stack[0].obj;
    WOBJ_GraphicsHasClip(gr) = 0;

    v.obj = 0;

    return v;
}

Var GraphicsTranslate(Var stack[]) {
    WObject gr;
    Var     v;

    gr = stack[0].obj;

    WOBJ_GraphicsTransX(gr) += stack[1].intValue;
    WOBJ_GraphicsTransY(gr) += stack[2].intValue;

    v.obj = 0;

    return v;
}

Var GraphicsDraw(int type, Var stack[]) {

    WObject gr;
    WObject string, charArray;
    Var v;    
    int surfaceType;
    int16* chars;
    struct window_t dest_win;
    int x, y, start, count;
    v.obj = 0;


    //printf("GraphicsDraw\r\n");
    gr = stack[0].obj;
    surfaceType = SurfaceGetType( WOBJ_GraphicsSurface( gr ) );

    if ( surfaceType == SURF_MAINWIN ) {

        dest_win = *s_offscreen_win_native;
        
        dest_win.origin_x += WOBJ_GraphicsTransX(gr);
        dest_win.origin_y += WOBJ_GraphicsTransY(gr);
        if (WOBJ_GraphicsHasClip(gr)) { 
            dest_win.width = WOBJ_GraphicsClipWidth(gr);
            dest_win.height = WOBJ_GraphicsClipHeight(gr);
        }

    } 
    else if ( surfaceType == SURF_IMAGE ) {

        dest_win.video_buffer = WOBJ_ImageBuffer( WOBJ_GraphicsSurface( gr ) );
        dest_win.buffer_width = WOBJ_ImageWidth( WOBJ_GraphicsSurface( gr ) );
        dest_win.buffer_height = WOBJ_ImageHeight( WOBJ_GraphicsSurface( gr ) );
        dest_win.origin_x = 0;
        dest_win.origin_y = 0; 
        if (WOBJ_GraphicsHasClip(gr)) { 
            dest_win.width = WOBJ_GraphicsClipWidth(gr);
            dest_win.height = WOBJ_GraphicsClipHeight(gr);
        }
        else {
            dest_win.width = dest_win.buffer_width;
            dest_win.height = dest_win.buffer_height;
        }
    }
    else {
        //printf("unknown surface!");
        return v;
    }


/*
    printf("dest_win.buffer_width =%d dest_win.buffer_height=%d\r\n", 
            dest_win.buffer_width, dest_win.buffer_height);
    printf("dest_win.origin_x=%d dest_win.origin_y=%d ,dest_win.width=%d dest_win.height=%d\r\n", 
            dest_win.origin_x, dest_win.origin_y, dest_win.width, dest_win.height);

*/


    /* draw depending of the type of drawing */
    switch( type ) {

        case GR_FILLRECT:   /* fill the given rectangle */
        {

            int x, y, w, h;

            /* get the position/size of the rectangle from the stack */
            x = stack[1].intValue;
            y = stack[2].intValue;
            w = stack[3].intValue;
            h = stack[4].intValue;

            //printf("fill_rect x=%d y=%d w=%d h=%d\r\n", x, y, w, h);
            fill_rect(&dest_win, x, y, w, h, WOBJ_GraphicsColor(gr));

        }
                    break;

        case GR_DRAWLINE:   /* draw a line */
        {

            int x1, y1, x2, y2;

            /* get the 2 points of the line from the stack */
            x1 = stack[1].intValue;
            y1 = stack[2].intValue;
            x2 = stack[3].intValue;
            y2 = stack[4].intValue;
            
            //printf("draw_line x1=%d y1=%d x2=%d y2=%d\r\n", x1, y1, x2, y2);           
            draw_line(&dest_win, x1, y1, x2, y2, WOBJ_GraphicsColor(gr));

        }
        break;

        case GR_DOTS:    /* draw a horitontal or vertical dotted line  */
        {

            int x1, y1, x2, y2, stepx, stepy, tempSwap;
            int x, y;

            /* get the 2 points of the line from the stack */
            x1 = stack[1].intValue;
            y1 = stack[2].intValue;
            x2 = stack[3].intValue;
            y2 = stack[4].intValue;

            if( x1 > x2 ) {

                /* reverse to have allways x1 lower than x2 */
                tempSwap = x1;
                x1 = x2;
                x2 = tempSwap;

            }

            if( y1 > y2 ) {

                /* reverse to have allways y1 lower than y2 */
                tempSwap = y1;
                y1 = y2;
                y2 = tempSwap;

            }

            if( x1 == x2 ) {

                /* vertical line */

                stepx = 0;
                stepy = 2;

             } else if( y1 == y2 ) {

                /* horizontal line */

                stepx = 2;
                stepy = 0;

            } else {

                /* neither horizontal, nor vertical => not correct */

                break;

            }

            /* now, we can loop to make the draw */
            for( x = x1, y = y1; ( x <= x2 ) && ( y <= y2 ); x += stepx, y += stepy ) {

                /* draw the current point */
                draw_pixel(&dest_win, x, y, WOBJ_GraphicsColor(gr));

            }
        }
        break;

        case GR_DRAWSTRING:    /* draw a string */
        case GR_DRAWCHARS:     /* draw chars */

            if( type == GR_DRAWSTRING ) {

                /* get the string to draw from the stack */
                string = stack[1].obj;
                charArray = WOBJ_StringCharArrayObj(string);

                /* compute the x coordinate */
                x = stack[2].intValue;

                /* compute the y coordinate */
                y = stack[3].intValue;

                /* start at 0, and compute the size of the string from the charArray */
                start = 0;
                count = WOBJ_arrayLen(charArray);

            } else { 
            
                /* get the chars to draw from the stack */
                charArray = stack[1].obj;

                /* get the starting position from the stack */
                start = stack[2].intValue;

                /* get the number of chars from the stack */
                count = stack[3].intValue;

                /* compute the x coordinate */
                x = stack[4].intValue;

                /* compute the y coordinate */
                y = stack[5].intValue;

                /* check the validity of the chars array */
                if( arrayRangeCheck( charArray, start, count ) == 0 ) {

                    break;

                }
            }

            /* convert the string to an "array of char" */
            chars = (u16 *)WOBJ_arrayStart(charArray);
            chars = &chars[start];

            /* unicode => chars */
            while( count > 0 ) {

                uchar buf[40];
                int i, n;

                /* limit the copy to the size of p */
                n = sizeof( buf ) - 1;
                if( n > count ) {
                    n = count;
                }

                /* copy each char from the unicode chars variable to the "normal" p variable */
                for( i = 0; i < n; i++ ) {
                    buf[i] = (char)chars[i];
                }

                /* the terminator */
                buf[i] = 0;

                /* draw the string */
                draw_string(&dest_win, buf, x, y, WOBJ_GraphicsColor(gr));

                /* update the counters */
                count -= n;
                chars += n;

                /* update the x position to draw */
                x += n*6;

            }
            break;
            
            case GR_COPYRECT:    /* copy of a rectangle from a Surface to another Surface */
            {

                WObject src_surf;
                int src_surf_type;
                struct window_t src_win;
                int x, y;

                /* get the original surface */
                src_surf = stack[1].obj;

                src_surf_type = SurfaceGetType( src_surf );

                if ( src_surf_type == SURF_MAINWIN ) {
                    //printf("mainwindow source\r\n");
                    src_win.video_buffer = s_offscreen_win_native->video_buffer;
                    src_win.buffer_width = s_offscreen_win_native->buffer_width;
                    src_win.buffer_height = s_offscreen_win_native->buffer_height;
                } 
                else if ( src_surf_type == SURF_IMAGE ) {
                    //printf("image source\r\n");
                    src_win.video_buffer = WOBJ_ImageBuffer( src_surf );
                    src_win.buffer_width = WOBJ_ImageWidth( src_surf );
                    src_win.buffer_height = WOBJ_ImageHeight( src_surf  );

                }
                else {
                    //printf("unknown surface type\r\n");
                    return v;
                }

                /* get the coordinate/size of the source rectangle to copy */
                src_win.origin_x = stack[2].intValue;
                src_win.origin_y = stack[3].intValue;
                src_win.width = stack[4].intValue;
                src_win.height = stack[5].intValue;
        
/*                printf("src_win.video_buffer=%p src_win.buffer_width =%d src_win.buffer_height=%d src_win.origin_x=%d src_win.origin_y=%d ,src_win.width %d\r\n", 
                        src_win.video_buffer, src_win.buffer_width, src_win.buffer_height,
                        src_win.origin_x, src_win.origin_y, src_win.width );

*/        
                /* get the position where to put the copy in the destination */

                x = stack[6].intValue;
                y = stack[7].intValue;
                //printf("x=%d y=%d\r\n", x, y);
                copy_window_unscaled(&dest_win, x, y, &src_win);

        }
        break;

        case GR_FILLPOLY:      /* draw a filled polygon */
        {
            WObject xArray, yArray;
            int   i, count;
            int*  x;
            int*  y;

            //couldn't be bothered with a real implementation of fillpoly
            //so this just draws the outlines


            xArray = stack[1].obj;
            yArray = stack[2].obj;

            if(xArray == 0 || yArray == 0)
            break;

            x = (int*)WOBJ_arrayStart(xArray);
            y = (int*)WOBJ_arrayStart(yArray);

            count = stack[3].intValue;

            if( count < 3 ||
                count > WOBJ_arrayLen(xArray) ||
                count > WOBJ_arrayLen(yArray))
                break;

            for(i=0; i < count - 1; i++)
                draw_line(&dest_win, x[i], y[i],  x[i + 1], y[i + 1], WOBJ_GraphicsColor(gr));

            draw_line(&dest_win, x[0], y[0],  x[i], y[i], WOBJ_GraphicsColor(gr));    

        }
        break;

        case GR_DRAWCURSOR:
        {

            //TODO
        }
        break;

        default:
            // *** ERROR *** WRONG DRAWING TYPE *** 
           break;

    }




    return v;
}

Var GraphicsFillRect(Var stack[]) {
    return GraphicsDraw(GR_FILLRECT, stack);
}

Var GraphicsDrawLine(Var stack[]) {
    return GraphicsDraw(GR_DRAWLINE, stack);
}

Var GraphicsFillPolygon(Var stack[]) {
    return GraphicsDraw(GR_FILLPOLY, stack);
}

Var GraphicsDrawChars(Var stack[]) {
    return GraphicsDraw(GR_DRAWCHARS, stack);
}

Var GraphicsDrawString(Var stack[]) {
    return GraphicsDraw(GR_DRAWSTRING, stack);
}

Var GraphicsDrawDots(Var stack[]) {
    return GraphicsDraw(GR_DOTS, stack);
}

Var GraphicsCopyRect(Var stack[]) {
    return GraphicsDraw(GR_COPYRECT, stack);
}

Var GraphicsDrawCursor(Var stack[]) {
    return GraphicsDraw(GR_DRAWCURSOR, stack);
}


/* implementation of native waba/fx/Graphics.setForeColor() */
Var GraphicsSetForeColor( Var stack[] ) {
    Var v;
    v.obj = 0;
    return v;

}

/* implementation of native waba/fx/Graphics.setBackColor() */
Var GraphicsSetBackColor( Var stack[] ) {
    Var v;

    v.obj = 0;
    return v;

}

/* implementation of native waba/fx/Graphics.free() */
Var GraphicsFree( Var stack[] ) {
    Var v; 
    
    v.obj = 0;
    return v;

}


//
// File
//

// WHEN PORTING: Note, this was just another way of stubbing these functions out

#define FileGetLength Return0Func
#define FileCreateDir Return0Func
#define FileRead ReturnNeg1Func
#define FileCreate Return0Func
#define FileWrite ReturnNeg1Func
#define FileListDir Return0Func
#define FileIsDir Return0Func
#define FileClose Return0Func
#define FileDelete Return0Func
#define FileExists Return0Func
#define FileIsOpen Return0Func
#define FileSeek Return0Func
#define FileRename Return0Func

Var Return0Func(Var stack[]) {
    Var v;

    v.obj = 0;
    return v;
}

Var ReturnNeg1Func(Var stack[]) {
    Var v;

    v.intValue = -1;
    return v;
}

//
// Socket
//


#define SocketCreate Return0Func
#define SocketDestroy Return0Func
#define SocketClose Return0Func
#define SocketIsOpen Return0Func
#define SocketSetReadTimeout Return0Func
#define SocketReadWriteBytes Return0Func
#define SocketRead Return0Func
#define SocketWrite Return0Func

//
// Sound
//


Var SoundTone(Var stack[]) {
    Var v;

    v.obj = 0;
    return v;
}

Var SoundBeep(Var stack[]) {
    Var v;

    v.obj = 0;
    return v;
}


//
// SoundClip
//

#define SoundClipPlay Return0Func

//
// Convert
//

Var ConvertFloatToIntBitwise(Var stack[]) {
        
    return stack[0];
	
}

Var ConvertIntToFloatBitwise(Var stack[]) {

    return stack[0];

}

Var ConvertStringToInt(Var stack[])
{
    WObject string, charArray;
    int32 i, isNeg, len, value;
    uint16 *chars;
    Var v;

    v.intValue = 0;
    string = stack[0].obj;
    if (string == 0)
        return v;
    charArray = WOBJ_StringCharArrayObj(string);
    if (charArray == 0)
        return v;
    chars = (uint16 *)WOBJ_arrayStart(charArray);
    len = WOBJ_arrayLen(charArray);
    isNeg = 0;
    if (len > 0 && chars[0] == '-')
        isNeg = 1;
    value = 0;
    for (i = isNeg; i < len; i++)
    {
        if (chars[i] < (uint16)'0' || chars[i] > (uint16)'9')
            return v;
        value = (value * 10) + ((int32)chars[i] - (int32)'0');
    }
    if (isNeg)
        value = -(value);
    v.intValue = value;
    return v;
}

Var ConvertIntToString(Var stack[])
{
    Var v;
    char buf[20];

    snprintf(buf, 20, "%d", stack[0].intValue);

    v.obj = createString(buf);
    return v;
}

Var ConvertFloatToString(Var stack[])
{
    Var v;
    char buf[20];
        
    //TODO - the printf in jaysos does not support %f!!!
    int intVal = (int)stack[0].floatValue;
    snprintf(buf, 20, "%d", intVal);

    v.obj = createString(buf);
    return v;
}



/* implementation of native waba/sys/Convert.toString(long l) : not supported */
Var ConvertLongToString( Var stack[] ) {
    Var v;

    v.obj = 0;
    return v;
}


/* implementation of native waba/sys/Convert.toString(double d) : not supported */
Var ConvertDoubleToString( Var stack[] ) {
    Var v;

    v.obj = 0;
    return v;
}




Var ConvertCharToString(Var stack[]) {
    Var v;
    char buf[2];

    buf[0] = (char)stack[0].intValue;
    buf[1] = 0;
    v.obj = createString(buf);
    return v;
}

Var ConvertBooleanToString(Var stack[]) {
    Var v;
    char *s;

    if (stack[0].intValue == 0)
        s = "false";
    else
        s = "true";
    v.obj = createString(s);
    return v;
}



//
// Time
//

#define WOBJ_TimeYear(o) (objectPtr(o))[1].intValue
#define WOBJ_TimeMonth(o) (objectPtr(o))[2].intValue
#define WOBJ_TimeDay(o) (objectPtr(o))[3].intValue
#define WOBJ_TimeHour(o) (objectPtr(o))[4].intValue
#define WOBJ_TimeMinute(o) (objectPtr(o))[5].intValue
#define WOBJ_TimeSecond(o) (objectPtr(o))[6].intValue
#define WOBJ_TimeMillis(o) (objectPtr(o))[7].intValue

Var TimeCreate(Var stack[]) {
    Var v;
    WObject otime;
    int millisecs = kern_uptime_centisecs*10;
    int secs = millisecs/1000;
    int minutes = secs/60;
    int hours = minutes/60;
    int days = hours/60;
    
    secs = secs - minutes*60;
    minutes = minutes - hours*60;
    hours = hours - days*24;

    /* get the Time object from the stack */
    otime = stack[0].obj;

    //ok, we don't have on onboard clock on the gba, so
    //we just throw some random time out here
  
    /* set the variable of the Time object */
    WOBJ_TimeYear(otime) = 2019;
    WOBJ_TimeMonth(otime) = 9;
    WOBJ_TimeDay(otime) = 1 + days; //your batteries will run out before you get to sep31 ;)
    WOBJ_TimeHour(otime) = 2 + hours;
    WOBJ_TimeMinute(otime) = 45 + minutes;
    WOBJ_TimeSecond(otime) = secs;
    WOBJ_TimeMillis(otime) = millisecs;

    /* return nothing */
    v.obj = 0;
    
    return v;
}
//
// SerialPort
//

#define SerialPortCreate Return0Func
#define SerialPortIsOpen Return0Func
#define SerialPortSetReadTimeout Return0Func
#define SerialPortReadCheck Return0Func
#define SerialPortSetFlowControl Return0Func
#define SerialPortClose Return0Func

void SerialPortDestroy(WObject port) {

}


Var SerialPortReadWriteBytes(Var stack[], int isRead) {
    Var v;

    v.obj = 0;
    return v;
}

Var SerialPortRead(Var stack[]) {
    return SerialPortReadWriteBytes(stack, 1);
}

Var SerialPortWrite(Var stack[]) {
    return SerialPortReadWriteBytes(stack, 0);
}


//
// Vm
//

Var VmIsColor(Var stack[]) {
    Var v;

    v.intValue  = 1;
    return v;
}

Var VmGetTimeStamp(Var stack[]) {
    Var v;

    v.intValue = kern_uptime_centisecs*10; //in milliseconds
    return v;
}

Var VmExec(Var stack[]) {
    Var v;

    v.obj = 0;
    return v;
}

Var VmExit(Var stack[]) {
    Var v;
        
    waba_die = TRUE;
    
    //printf("VmExit\r\n");
    v.obj = 0;
    return v;
}

Var VmSleep(Var stack[]) {
    Var v;
        
    thread_sleep_self(stack[0].intValue/10);
	
    v.obj = 0;
    return v;

}

Var VmGetPlatform(Var stack[]) {
    Var v;

    v.obj = createString("JaysOS GBA");
    return v;
}

Var VmSetDeviceAutoOff(Var stack[]) {
    Var v;

    v.obj = 0;
    return v;
}

Var VmGetUserName(Var stack[]) {
    Var v;

    v.obj = createString("<unknown>");
    return v;
}


//
// VmShell
//

Var VmPrint(Var stack[]) {
    WObject strString;
    UtfString str;
    Var v;

    v.intValue = 0;
    strString = stack[0].obj;
    str = stringToUtf(strString, STU_NULL_TERMINATE);

    printf("%s", str.str);
    return v;

}

Var VmPrintLn( Var stack[] ) {
    WObject strString;
    UtfString str;
    Var v;

    v.intValue = 0;
    strString = stack[0].obj;
    str = stringToUtf(strString, STU_NULL_TERMINATE);


    printf("%s\r\n", str.str);
    return v;

}

