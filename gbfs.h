/* Modifed by Justin Armstrong Aug 2002 <ja@badpint.org>
 *
 * changes from Damian Yerrick's original 20020404 release:
 * - max pathname is now 128 chars
 * - renamed GBFS_FILE to GBFS_ARCHIVE
 * see readme for more details
 */


/* gbfs.h
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


/* Dependency on prior include files

Before you #include "gbfs.h", you should define the following types:
  typedef (unsigned 16-bit integer) u16;
  typedef (unsigned 32-bit integer) u32;
Your gba.h should do this for you.
*/

#ifndef INCLUDE_GBFS_H
#define INCLUDE_GBFS_H
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_GBFS_PATH  128

typedef struct GBFS_ARCHIVE
{
  char magic[16];    /* "PinEightGBFS\r\n\032\n" */
  u32  total_len;    /* total length of archive */
  u16  dir_off;      /* offset in bytes to directory */
  u16  dir_nmemb;    /* number of files */
  char reserved[8];  /* for future use */
} GBFS_ARCHIVE;

typedef struct GBFS_ENTRY
{
  char name[MAX_GBFS_PATH];     /* filename, nul-padded */
  u32  len;          /* length of object in bytes */
  u32  data_offset;  /* in bytes from beginning of file */
} GBFS_ENTRY;


const GBFS_ARCHIVE *find_first_gbfs_archive(const void *start);
const void *skip_gbfs_archive(const GBFS_ARCHIVE *file);
const void *gbfs_get_obj(const GBFS_ARCHIVE *file,
                         const char *name,
                         u32 *len);
void *gbfs_copy_obj(void *dst,
                    const GBFS_ARCHIVE *file,
                    const char *name);


#ifdef __cplusplus
}
#endif
#endif
