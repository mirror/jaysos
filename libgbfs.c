/* Modifed by Justin Armstrong Aug 2002 <ja@badpint.org>
 *
 * changes from Damian Yerrick's original 20020404 release:
 * - max pathname is now 128 chars
 * - renamed GBFS_FILE to GBFS_ARCHIVE
 * see readme for more details
 */



/* libgbfs.c
   access object in a GBFS file

Copyright 2002 Damian Yerrick

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

*/


/* This code assumes a LITTLE ENDIAN target.  It'll need a boatload
   of itohs and itohl calls if converted to run on Sega Genesis.  It
   also assumes that the target uses 16-bit short and 32-bit longs.
*/

#include "util.h"
#include "kernel.h"
#include "semaphore.h"
#include "gbfs.h"

static bool s_inited = FALSE;
static struct semaphore_t s_fs_sem;

/* change this to the end of your ROM, or to 0x02040000 for multiboot */
#define GBFS_SEARCH_LIMIT ((const u32 *)0x08800000)

static void
_init_gbfs() {
    s_inited = TRUE;
    semaphore_create(&s_fs_sem, 1);
}

const GBFS_ARCHIVE *find_first_gbfs_archive(const void *start)
{
  /* align the pointer */
  const u32 *here = (const u32 *)
                      ((unsigned long)start & 0xfffffffc);
  const char rest_of_magic[] = "ightGBFS\r\n\032\n";

  if (!s_inited)
    _init_gbfs();

  semaphore_wait(&s_fs_sem);

  /* while we haven't yet reached the end of the ROM space */
  while(here < GBFS_SEARCH_LIMIT)
  {
    /* We have to keep the magic code in two pieces; otherwise,
       this function will find itself and think it's a GBFS file.
       This obviously won't work if your compiler stores this
       numeric literal just before the literal string, but Devkit
       Advance seems to keep numeric constant pools separate enough
       from string pools for this to work.
    */
    if(*here == 0x456e6950)  /* ASCII code for "PinE" */
    {
      /* we're already after here;
         if the rest of the magic matches, then we're through */
      if(!memcmp(here + 1, rest_of_magic, 12)) {
        semaphore_signal(&s_fs_sem);
        return (const GBFS_ARCHIVE *)here;
      }
    }
    ++here;
  }
  
  semaphore_signal(&s_fs_sem);
  
  return 0;
}


const void *skip_gbfs_archive(const GBFS_ARCHIVE *file)
{
  return ((char *)file + file->total_len);
}


static int namecmp(const void *a, const void *b)
{
  return strncmp(a, b, MAX_GBFS_PATH);
}


const void *gbfs_get_obj(const GBFS_ARCHIVE *file,
                         const char *name,
                         u32 *len)
{
  char key[MAX_GBFS_PATH] = {0};
  void* data;

  semaphore_wait(&s_fs_sem);

  GBFS_ENTRY *dirbase = (GBFS_ENTRY *)((char *)file + file->dir_off);
  size_t n_entries = file->dir_nmemb;
  GBFS_ENTRY *here;

  
  //printf("gbfs_get_obj: %s\r\n",name);
  strncpy(key, name, MAX_GBFS_PATH);

  here = bsearch(key, dirbase,
                 n_entries, sizeof(GBFS_ENTRY),
                 namecmp);
  if(!here) {
    semaphore_signal(&s_fs_sem);
    return NULL;
  }

  if(len)
    *len = here->len;
    
  data = (char *)file + sizeof(GBFS_ARCHIVE) + (sizeof(GBFS_ENTRY)*n_entries) + here->data_offset;
  
  semaphore_signal(&s_fs_sem);
  
  return data;
  
}


void *gbfs_copy_obj(void *dst,
                    const GBFS_ARCHIVE *file,
                    const char *name)
{
  u32 len;
  const void *src;
  
  semaphore_wait(&s_fs_sem);
  
  src = gbfs_get_obj(file, name, &len);

  if(!src)
    return NULL;

  memcpy(dst, src, len);
  
  semaphore_signal(&s_fs_sem);
  
  return dst;
}
