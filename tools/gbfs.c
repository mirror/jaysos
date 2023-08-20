/* Modifed by Justin Armstrong Aug 2002 <ja@badpint.org>
 *
 * changes from Damian Yerrick's original 20020404 release:
 * - can now create archives from files in directories
 * - max pathname is now 128 chars
 * - renamed GBFS_FILE to GBFS_ARCHIVE
 * - generally changed the way in which the archive file is created
 * see readme for more details
 */


/* gbfs.c
   create a GBFS file

Copyright (C) 2002  Damian Yerrick

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to 
  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA  02111-1307, USA.
GNU licenses can be viewed online at http://www.gnu.org/copyleft/

Visit http://www.pineight.com/ for more information.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

//char *basename (const char *fname);

void add_directory(char* path);
void add_file(char* path);

typedef unsigned short u16;
typedef unsigned long u32;  /* this needs to be changed on 64-bit systems */

#include "../gbfs.h"

static const char GBFS_magic[] = "PinEightGBFS\r\n\032\n";

static const char help_text[] =
"Creates a GBFS archive.\n"
"usage: gbfs ARCHIVE [FILE...]\n";

GBFS_ARCHIVE header;

#define MAX_ENTRIES 500
GBFS_ENTRY entries[MAX_ENTRIES];

unsigned int n_entries = 0;

FILE *outfile;


unsigned long fcopy(FILE *dst, FILE *src)
{
  char buf[16384];
  unsigned long sz = 0, this_sz;

  do {
    this_sz = fread(buf, 1, sizeof(buf), src);
    if(this_sz)
    {
      fwrite(buf, 1, this_sz, dst);  /* FIXME: need error checking */
      sz += this_sz;
    }
  } while(this_sz != 0);

  return sz;
}





/* fputi16() ***************************
   write a 16-bit integer in intel format to a file
*/
void fputi16(unsigned int in, FILE *f)
{
  fputc(in, f);
  fputc(in >> 8, f);
}


/* fputi32() ***************************
   write a 32-bit integer in intel format to a file
*/
void fputi32(unsigned long in, FILE *f)
{
  fputc(in, f);
  fputc(in >> 8, f);
  fputc(in >> 16, f);
  fputc(in >> 24, f);
}


/* namecmp() ***************************
   compares the first MAX_GBFS_PATH bytes of a pair of strings.
   useful for sorting names.
*/

int namecmp(const void *a, const void *b)
{
  return memcmp(a, b, MAX_GBFS_PATH);
}


void add_directory(char* path)
{
  DIR *dir;
  struct dirent *ent;
  char fullpath[MAX_GBFS_PATH];

  dir = opendir(path);
  if (dir == NULL)
  {
    printf("could not open directory %s\n", path);
    exit(-1);
  }
  
  
  while ((ent = readdir(dir)))
  {
    if (ent->d_name[0] == '.')
	    continue;
            
    snprintf(fullpath, MAX_GBFS_PATH, "%s/%s", path, ent->d_name);
    add_file(fullpath);
  }

  closedir(dir);

}

void add_file(char* path)
{
  struct stat file_stat;
  FILE *infile;
  unsigned long flen;


  if (stat(path, &file_stat) != 0)
  {
    printf("can't stat file %s\n", path);
    exit(1);
  }
  
  if (S_ISDIR(file_stat.st_mode))
  {
    add_directory(path);
    return;
  }


  infile = fopen(path, "rb");
  if(!infile)
  {
    printf("could not open %s: %s", path, strerror(errno));
    fclose(infile);
    fclose(outfile);
    remove("gbfs.$$$");
    exit(1);
  }

   
  /* get current file offset */
  entries[n_entries].data_offset = ftell(outfile);

  /* copy the file into the data area */
  flen = fcopy(outfile, infile);
  entries[n_entries].len = flen;

  /* copy name */
  strncpy(entries[n_entries].name,
          path,
          MAX_GBFS_PATH);

  /* diagnostic */
  {
    char nameout[MAX_GBFS_PATH] = {0};

    strncpy(nameout,
            entries[n_entries].name,
            MAX_GBFS_PATH);
    printf("%10lu %s\n", flen, nameout);
  }

  /* pad file with 0's to para boundary */
  while(ftell(outfile) & 0x000f)
  {
    fputc(0, outfile);
  }

  /* next file please */
  n_entries++;

  if (n_entries >= MAX_ENTRIES) 
  {
    printf("more than %d files\n", MAX_ENTRIES);
    exit(1);
  }

}


int main(int argc, char **argv)
{

  unsigned int arg = 2;

  if(argc < 3)
  {
    fputs(help_text, stderr);
    return 1;
  }

  n_entries = 0;


  outfile = fopen("gbfs.$$$", "wb+");
  if(!outfile)
  {
    perror("could not open temporary file gbfs.$$$ for writing");
    exit(1);
  }

  //read all input files, write them into temp file, 
  //store GBFS_ENTRY details for later
  while(arg < argc)
  {
    add_file(argv[arg]);
    arg++;
  }



  memcpy(header.magic, GBFS_magic, sizeof(header.magic));
  header.dir_off = 32;
  header.dir_nmemb = n_entries;
  header.total_len = header.dir_off + n_entries * sizeof(GBFS_ENTRY) + ftell(outfile);

  fclose(outfile);  //we have finished writing the data to the temp file
  
  
  remove(argv[1]);  //get rid of any previous archive files
  
  //create the archive file to which we are going to write the header, followed by
  //the data from the temp file
  outfile = fopen(argv[1], "wb+");   
  if(!outfile)
  {
    fprintf(stderr,
            "could not create output file %s: %s", argv[1], strerror(errno));

    exit(1);
  }


  /* sort directory by name */
  qsort(entries, n_entries, sizeof(entries[0]), namecmp);

  /* write header */

  fwrite(GBFS_magic, 16, 1, outfile);
  fputi32(header.total_len, outfile);
  fputi16(header.dir_off, outfile);
  fputi16(header.dir_nmemb, outfile);

  
  /* write directory */
  fseek(outfile, header.dir_off, SEEK_SET);
  {
    unsigned int i;

    for(i = 0; i < n_entries; i++)
    {
      fwrite(entries[i].name, sizeof(entries[i].name), 1, outfile);
      fputi32(entries[i].len, outfile);
      fputi32(entries[i].data_offset, outfile);
    }
  }

  /* copy data from temp file into archive file */
  {
    FILE *infile;

    infile = fopen("gbfs.$$$", "rb");
    if(!infile)
    {
      fprintf(stderr,
            "could not open tempgbfs.$$$: %s", strerror(errno));
      fclose(infile);
      fclose(outfile);
      remove("gbfs.$$$");
      exit(1);
    }
    
    fcopy(outfile, infile);
    fclose(infile);
    remove("gbfs.$$$");
  }


  fclose(outfile);


  return 0;
}

/* The behavior of this program with respect to ordering of files

This information is subject to change.

Currently, the app writes the objects' data in order of appearance
on the command line, but it writes the directory in ABC order as
required by the format spec.

*/
