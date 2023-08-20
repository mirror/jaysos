
#ifndef __NMJAYSOS_A_H__
#define __NMJAYSOS_A_H__

/* $Id: nmport_a.h,v 1.2 2001/08/18 20:26:21 bornet Exp $

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


//jaysos port

#include <kernel.h>
#include <util.h>
#include <ui_mgr.h>
#include <gbfs.h>

// WHEN PORTING: define FREE_ON_EXIT if you want all object destructors
// to be called when the program exits. Under OS's that release system
// resources automatically, you don't need to do this. On OS's that don't
// automatically release system resources (images, memory, etc.) that
// native functions may allocate, you should set this to 1 so when
// the waba program exits, all native method object destructors get called
// to clean things up
#define FREE_ON_EXIT 1

#undef SECURE_CLASS_HEAP
#define LOCK_CLASS_HEAP
#define UNLOCK_CLASS_HEAP

#define uchar unsigned char
#define int32 long
#define uint32 unsigned long
#define float32 float
#define int16 short
#define uint16 unsigned short

#ifdef WITH_64BITS

// - int64 is a signed 64 bit value
#define int64 long long

// - uint64 is an unsigned 64 bit value
#define uint64 unsigned long long

// - float64 is a signed 64 bit floating point value
#define float64 double

// --- define a struct for double and long support in two Var ...

// ... the hi/lo decomposition
typedef struct {
  uint32 lo;
  uint32 hi;
} HiLoDoubleOrLong;

//... for the double
typedef union {
  float64 value;
  HiLoDoubleOrLong ul;
} float64HiLo;

//... for the long
typedef union {
  int64 value;
  HiLoDoubleOrLong ul;
} int64HiLo;

#endif    /* WITH_64BITS */

// WHEN PORTING: You need to define functions that convert a string
// of 2 or 4 bytes in network byte order to a 16 or 32 bit value as follows:
//
// getUInt32() - converts 4 bytes to a unsigned 32 bit integer
// getUInt16() - converts 2 bytes to a unsigned 16 bit integer
// getInt32() - converts 4 bytes to a signed 32 bit integer
// getInt16() - converts 2 bytes to a signed 16 bit integer
//
// Here is an example from PalmOS, you can make these functions instead of
// a macro if you want the executable to be smaller

#define getUInt32(b) (uint32)( (uint32)((b)[0])<<24 | (uint32)((b)[1])<<16 | (uint32)((b)[2])<<8 | (uint32)((b)[3]) )
#define getUInt16(b) (uint16)(((b)[0]<<8)|(b)[1])
#define getInt32(b) (int32)( (uint32)((b)[0])<<24 | (uint32)((b)[1])<<16 | (uint32)((b)[2])<<8 | (uint32)((b)[3]) )
#define getInt16(b) (int16)(((b)[0]<<8)|(b)[1])

float32 getFloat32(uchar *buf);

#ifdef WITH_64BITS
// WHEN PORTING: You need to define functions that convert a string
// of 8 bytes in network byte order to a 64 bit value as follows:

// get one 8byte float value from buf and return it
float64 getFloat64bits(uchar *buf);

// get one 8byte int value from buf and return it
int64 getInt64bits(uchar *buf);

#endif    /* WITH_64BITS */


// WHEN PORTING: You need to define the following functions:
//
// xstrncmp(s1, s2, n) - compares 2 strings n bytes up to a 0 terminator 
// xstrncpy(s1, s2, n) - copies 2 strings n bytes up to a 0 terminator 
// xstrlen(s) - returns the length of a string
// xstrcat(dst, src) - concats one string to another
// xmemmove(dst, src, size) - copies size bytes from src to dst
// xmemzero(mem, len) - zeros out len bytes at location mem
//
// You could find the native functions that map to the above functions
// or (easier) just use the ones below. In any case, its probably best
// to use the ones below first and then you switch out after things
// are running in case the machines functions don't do exactly what
// is expected.

#define xstrncmp(s1, cs2, n) strncmp(s1, cs2, n)

#define xstrlen(s) strlen(s)

#define xstrncpy(dst, src, n) strncpy(dst, src, n)

//TODO - memmove is not the same as memcpy!!!
#define xmemmove(dst, src, n) memcpy(dst, src, n)

void xstrcat(char *dst, char *src);



#define xmemzero(mem, len) memset(mem, (char)0, len)


// WHEN PORTING: You need to define a function that allocates a block
// of memory and a function that frees a block of memory. The WabaVM
// allocates 4 blocks of memory when it starts up and these methods
// are only used to allocate and free those 4 blocks of memory. When
// the VM is running, it does not call these functions. So, if you have
// a processor without any heap management, it should be rather easy
// to get the VM running. Just allocate 4 blocks of memory for it and
// return pointers to them below. You don't need a heap based malloc()
// implementation or anything to run with the WabaVM. The VM does its
// own memory management within the 4 blocks.
//
// The two functions you need to implement are:
//
// xmalloc(size) - allocates size bytes of memory and returns a pointer
// xfree(ptr) - frees the memory allocated previously
//
// On platforms that have malloc() and free, we can simply use them as
// follows:

#define xmalloc(size) malloc(size);

#define xfree(ptr) free(ptr)

// WHEN PORTING: That's it for the types and memory allocation, the next
// step is to implement the class loader and native functions in the
// nm<platform>_b.c and nm<platform>_c.c files


#endif
