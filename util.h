#ifndef UTIL_H
#define UTIL_H

#include "kernel.h"

typedef unsigned long size_t;


void *
memcpy(void* dest, const void* src, int n);

void* 
memset(void* m, int c, size_t n);

int
memcmp(const void *s1, const void *s2, size_t n);

char*
strncpy(char *dest, const char *src, size_t n);

size_t
strlen(const char *str);

void *
bsearch(const void *key, const void *base0, size_t nmemb, size_t size, int (*compar)(const void *, const void*));

int
strncmp(const char *s1, const char *s2, size_t n);

int
strcmp(const char* s1, const char* s2);

const char* 
itoa(int i, int minlength);

int 
rand();

int 
abs(int x);

//-----------------------------------

typedef int jmp_buf[11];

//defined in lowlevel.s
void longjmp(jmp_buf env, int val);
int setjmp(jmp_buf env);

//-----------------------------------

#define isupper(c)      ((c) >= 'A' && (c) <= 'Z')
#define tolower(c)      ((c) - 'A' + 'a')
#define isspace(c)      ((c) == ' ' || (c) == '\t')
#define isdigit(c)      ((c) >= '0' && (c) <= '9')

//-----------------------------------

#define     UCHAR_MAX       0xff

//-------------------------------------------------------------------------------------------

typedef char *va_list;

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_start(AP, LASTARG)                                           \
 (AP = ((char *) __builtin_next_arg (LASTARG)))

#define va_arg(AP, TYPE)                                                \
 (AP = ((char *) (AP)) += __va_rounded_size (TYPE),                     \
  *((TYPE *) ((char *) (AP) - __va_rounded_size (TYPE))))

#define va_end(ap)

//-------------------------------------------------------------------------------------------

//defined in printf.c


//hmm...this should really return an int
void
printf(const char *fmt, ...);

int
snprintf(char *buf, size_t size, const char *fmt, ...);

void
vprintf(const char *fmt, va_list ap);

int
vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);


//-------------------------------------------------------------------------------------------
//defined in malloc.h

u32
count_bytes_free();
u32
max_heap_size();

void*
malloc(int nbytes);

void
free(void* blk);


#endif
