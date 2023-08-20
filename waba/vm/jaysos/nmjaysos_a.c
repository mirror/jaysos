/* jaysos/GBA port by Justin Armstrong <ja@badpint.org> */

/* $Id: nmport_a.c,v 1.3 2001/08/18 20:26:21 bornet Exp $

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
/* This file is modified by Monaka (monaka@monamour.org) */

/*

This file is the starting point when porting the WabaVM to a new platform.

To port the WabaVM, you need to fill in platform dependent code in 3 files:

- nm<platform>_a.c
- nm<platform>_b.c
- nm<platform>_c.c

The nm stands for 'native methods'. The 3 files contain:

- code to deal with platform-specific type sizes and byte order
- code to allocate a few blocks of memory
- code to load a class from memory or disk
- the main application loop
- native functions

This file contains the code to deal with platform-specific types and
allocation of blocks of memory. To port the VM, read though the comments
and look for the 'WHEN PORTING' comments and fill in what you need to.

You'll probably want to take a look at waba.c as you go along to see
where the code is used. A WabaVM is comprised of the 3 native
function files and the VM code itself contained in waba.c

*/



// WHEN PORTING: You need to define the basic types of uchar, int32, etc.
// for the platform being ported to as follows:
//
// - uchar is an 8 bit unsigned value
// - int32 is a signed 32 bit value
// - uint32 is an unsigned 32 bit value
// - float32 is a signed 32 bit floating point value
// - int16 is a signed 16 bit value
// - uint16 is an unsigned 16 bit value
//
// below is an example from PalmOS

/* all the needed include */
#include "../waba.h"

//
// type converters
//


float32 getFloat32(uchar *buf) {
    uint32 i;
    float32 f;

    // we need to make sure we're aligned before casting
    i = ((uint32)buf[0] << 24) | ((uint32)buf[1] << 16) | ((uint32)buf[2] << 8) | (uint32)buf[3];
    f = *((float32 *)&i);
    return f;
}


void xstrcat(char *dst, char *src) {
    while (*dst != 0)
    dst++;
    while (*src != 0)
    *dst++ = *src++;
    *dst = 0;
}








