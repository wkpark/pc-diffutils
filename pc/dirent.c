/* @(#)dirent.c
 *
 * public domain implementation of POSIX directory routines 
 * for DOS, OS/2 and Win32
 *
 * Written by Michael Rendell ({uunet,utai}michael@garfield), August 1897
 *
 * Ported to OS/2 by Kai Uwe Rommel, December 1989, February 1990
 * Change for HPFS support, October 1990
 * Updated for better error checking and real rewinddir, February 1992
 * made POSIX conforming, February 1992
 * added Win32 support, December 1997
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <dirent.h>


#define MAGIC    0x4711

#if !defined(TRUE) && !defined(FALSE)
#define TRUE    1
#define FALSE   0
#endif

#if defined(__OS2__) && !defined(OS2)
#define OS2
#endif

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32)
#define WIN32
#endif

#if defined(__MINGW32__) && !defined(__32BIT__)
#define __32BIT__
#endif


#if defined(OS2) || defined(WIN32)

#ifndef __32BIT__
#define far     _far
#define near    _near
#define pascal  _pascal
#define cdecl   _cdecl
#endif

#ifdef OS2
#define INCL_NOPM
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#else /* !OS2 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* OS2 */

#ifdef __32BIT__

#define ALLFILES "*"

#ifdef __EMX__
#include <emx/syscalls.h>
static struct _find find;
#define achName name
#define _FindFirst(p, pd, a, pf, sf, pc)   __findfirst(p, a, pf)
#define _FindNext(d, pf, sf, pc)           __findnext(pf)
#define _FindClose(d)
#else /* !__EMX__ */

#ifdef OS2
static HDIR hdir;
static ULONG count;
static FILEFINDBUF3 find;
#define _FindFirst(p1, p2, p3, p4, p5, p6) DosFindFirst(p1, p2, p3, p4, p5, p6, 1)
#define _FindNext(d, pf, sf, pc)           DosFindNext(d, pf, sf, pc)
#define _FindClose(d)                      DosFindClose(d)
#else /* !OS2 */
static HANDLE hdir;
static WIN32_FIND_DATA find;
#define achName cFileName
#define _FindFirst(p1, p2, p3, p4, p5, p6) (*p2 = FindFirstFile(p1, p4), 0)
#define _FindNext(d, pf, sf, pc)           !FindNextFile(d, pf)
#define _FindClose(d)                      FindClose(d)
#endif /* OS2 */

#endif /* __EMX__ */

#ifndef ENOTDIR
#define ENOTDIR EINVAL
#endif

#ifdef __IBMC__
#define S_IFMT 0xF000
#endif

#else /* !__32BIT__ */

#define ALLFILES (_osmode == DOS_MODE ? "*.*" : "*")

static HDIR hdir;
static USHORT count;
static FILEFINDBUF find;
#define _FindFirst(p1, p2, p3, p4, p5, p6) DosFindFirst(p1, p2, p3, p4, p5, p6, 0)
#define _FindNext(d, pf, sf, pc)           DosFindNext(d, pf, sf, pc)
#define _FindClose(d)                      DosFindClose(d)

#endif /* __32BIT__ */

#else /* !OS2 && !WIN32 */

#define ALLFILES "*.*"

#include <dos.h>

#ifdef __TURBOC__
#include <dir.h>
static struct ffblk find;
#define achName ff_name
#define _FindFirst(p, pd, a, pf, sf, pc)   findfirst(p, pf, a)
#define _FindNext(d, pf, sf, pc)           findnext(pf)
#define _FindClose(d)
#else /* !__TURBOC__ */
static struct find_t find;
#define achName name
#define _FindFirst(p, pd, a, pf, sf, pc)   _dos_findfirst(p, a, pf)
#define _FindNext(d, pf, sf, pc)           _dos_findnext(pf)
#define _FindClose(d)
#endif /* !__TURBOC__ */

#endif /* OS2 || WIN32 */


#ifndef _A_DIR
#define _A_HIDDEN   0x02
#define _A_DIR      0x10
#endif

int attributes = _A_DIR | _A_HIDDEN;

static int getdirent(char *);
static int savedirent(DIR *);
static void free_dircontents(struct d_dircontents *);


DIR *opendir(const char *name)
{
  struct stat statb;
  DIR *dirp;
  char nbuf[_POSIX_PATH_MAX + 1];
  char c;
  int len;

  if ( name == NULL || *name == '\0' )
  {
    errno = ENOENT;
    return NULL;
  }

  if ( (dirp = malloc(sizeof(DIR))) == NULL )
  {
    errno = ENOMEM;
    return NULL;
  }

  strcpy(nbuf, name);
  len = strlen(nbuf);

  if ( ((c = nbuf[len - 1]) == '\\' || c == '/') && (len > 1) )
  {
    nbuf[len - 1] = '\0';
    len--;

    if ( nbuf[len - 1] == ':' )
    {
      strcpy(nbuf + len, "\\.");
      len += 2;
    }
  }
  else
    if ( nbuf[len - 1] == ':' )
    {
      strcpy(nbuf + len, ".");
      len++;
    }

  if ( stat(nbuf, &statb) == 0 )
  {
    if ( (statb.st_mode & S_IFMT) != S_IFDIR )
    {
      free(dirp);
      errno = ENOTDIR;
      return NULL;
    }

    if ( nbuf[len - 1] == '.' && (len == 1 || nbuf[len - 2] != '.') )
      strcpy(nbuf + len - 1, ALLFILES);
    else
      if ( ((c = nbuf[len - 1]) == '\\' || c == '/') && (len == 1) )
	strcpy(nbuf + len, ALLFILES);
      else
	strcat(strcpy(nbuf + len, "\\"), ALLFILES);
    
    _fullpath(dirp -> d_dd_name, nbuf, _POSIX_PATH_MAX);
  }
  else /* pattern, can't stat it, we hope it works ... */
    strcpy(dirp -> d_dd_name, nbuf);

  dirp -> d_dd_id  = MAGIC;
  dirp -> d_dd_loc = 0;
  dirp -> d_dd_contents = NULL;

  if ( getdirent(dirp -> d_dd_name) )
    do
    {
      if ( !savedirent(dirp) )
      {
	free_dircontents(dirp -> d_dd_contents);
	free(dirp);
	errno = ENOMEM;
	return NULL;
      }
    }
    while ( getdirent(NULL) );

  dirp -> d_dd_cp = dirp -> d_dd_contents;

  return dirp;
}


int closedir(DIR *dirp)
{
  if ( dirp == NULL || dirp -> d_dd_id != MAGIC )
  {
    errno = EBADF;
    return -1;
  }

  free_dircontents(dirp -> d_dd_contents);
  free(dirp);

  return 0;
}


struct dirent *readdir(DIR *dirp)
{
  static struct dirent dp;

  if ( dirp == NULL || dirp -> d_dd_id != MAGIC )
  {
    errno = EBADF;
    return NULL;
  }

  if ( dirp -> d_dd_cp == NULL )
    return NULL;

  dp.d_namlen = dp.d_reclen =
    strlen(strcpy(dp.d_name, dirp -> d_dd_cp -> d_dc_entry));

  dp.d_ino = 0;

  dp.d_size = dirp -> d_dd_cp -> d_dc_size;
  dp.d_mode = dirp -> d_dd_cp -> d_dc_mode;
  dp.d_time = dirp -> d_dd_cp -> d_dc_time;
  dp.d_date = dirp -> d_dd_cp -> d_dc_date;

  dirp -> d_dd_cp = dirp -> d_dd_cp -> d_dc_next;
  dirp -> d_dd_loc++;

  return &dp;
}


void seekdir(DIR *dirp, long off)
{
  long i = off;
  struct d_dircontents *dp;

  if ( dirp == NULL || dirp -> d_dd_id != MAGIC )
  {
    errno = EBADF;
    return;
  }

  if ( off >= 0 )
  {
    for ( dp = dirp -> d_dd_contents; (--i >= 0) && dp; dp = dp -> d_dc_next );

    dirp -> d_dd_loc = off - (i + 1);
    dirp -> d_dd_cp = dp;
  }
}


long telldir(DIR *dirp)
{
  if ( dirp == NULL || dirp -> d_dd_id != MAGIC )
  {
    errno = EBADF;
    return -1;
  }

  return dirp -> d_dd_loc;
}


void rewinddir(DIR *dirp)
{
  if ( dirp == NULL || dirp -> d_dd_id != MAGIC )
  {
    errno = EBADF;
    return;
  }

  free_dircontents(dirp -> d_dd_contents);
  dirp -> d_dd_loc = 0;
  dirp -> d_dd_contents = NULL;

  if ( getdirent(dirp -> d_dd_name) )
    do
    {
      if ( !savedirent(dirp) )
      {
	free_dircontents(dirp -> d_dd_contents);
	dirp -> d_dd_contents = NULL;
	errno = ENOMEM;
	break;
      }
    }
    while ( getdirent(NULL) );

  dirp -> d_dd_cp = dirp -> d_dd_contents;
}


static void free_dircontents(struct d_dircontents *dp)
{
  struct d_dircontents *odp;

  while (dp)
  {
    if (dp -> d_dc_entry)
      free(dp -> d_dc_entry);

    odp = dp;
    dp = odp -> d_dc_next;
    free(odp);
  }
}


int savedirent(DIR *dirp)
{
  struct d_dircontents *dp;

  if ( (dp = malloc(sizeof(struct d_dircontents))) == NULL ||
       (dp -> d_dc_entry = malloc(strlen(find.achName) + 1)) == NULL )
  {
    if ( dp )
      free(dp);
    return FALSE;
  }

  if ( dirp -> d_dd_contents )
  {
    dirp -> d_dd_cp -> d_dc_next = dp;
    dirp -> d_dd_cp = dirp -> d_dd_cp -> d_dc_next;
  }
  else
    dirp -> d_dd_contents = dirp -> d_dd_cp = dp;

  dp -> d_dc_next = NULL;
  strcpy(dp -> d_dc_entry, find.achName);

#ifdef __EMX__
  dp -> d_dc_size = ((unsigned long) find.size_hi << 16) + find.size_lo;
  dp -> d_dc_mode = find.attr;
  dp -> d_dc_time = *(unsigned *) &(find.time);
  dp -> d_dc_date = *(unsigned *) &(find.date);
#else /* !__EMX__ */
#ifdef OS2
  dp -> d_dc_size = find.cbFile;
  dp -> d_dc_mode = find.attrFile;
  dp -> d_dc_time = *(unsigned *) &(find.ftimeLastWrite);
  dp -> d_dc_date = *(unsigned *) &(find.fdateLastWrite);
#else /* !OS2 */
#ifdef WIN32
  dp -> d_dc_size = find.nFileSizeLow; /* only up to 4GB */
  dp -> d_dc_mode = find.dwFileAttributes;
  FileTimeToDosDateTime(&(find.ftLastWriteTime), 
			(LPWORD) &(dp -> d_dc_time), 
			(LPWORD) &(dp -> d_dc_date));
#else /* !WIN32 */
  dp -> d_dc_size = find.size;
  dp -> d_dc_mode = find.attrib;
  dp -> d_dc_time = find.wr_time;
  dp -> d_dc_date = find.wr_date;
#endif /* WIN32 */
#endif /* OS2 */
#endif /* __EMX__ */

  return TRUE;
}


int getdirent(char *dir)
{
  int done;
  static int lower = TRUE;

  if (dir != NULL)
  {				       /* get first entry */
    lower = IsFileSystemDumb(dir);

#if !defined(__EMX__) && defined(OS2)
    hdir = HDIR_CREATE;
    count = 1;
#endif

    done = _FindFirst(dir, &hdir, attributes, &find, sizeof(find), &count);
  }
  else				       /* get next entry */
    done = _FindNext(hdir, &find, sizeof(find), &count);

  if (done == 0)
  {
    if ( lower )
      strlwr(find.achName);
    return TRUE;
  }
  else
  {
    _FindClose(hdir);
    return FALSE;
  }
}


int IsFileNameValid(char *name)
{
#ifdef OS2
  HFILE hf;
#ifdef __32BIT__
  ULONG uAction;
#else
  USHORT uAction;
#endif

  switch( DosOpen(name, &hf, &uAction, 0, 0, FILE_OPEN,
                  OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, 0) )
  {
  case ERROR_INVALID_NAME:
  case ERROR_FILENAME_EXCED_RANGE:
  /* case ERROR_PATH_NOT_FOUND: */
    return FALSE;
  case NO_ERROR:
    DosClose(hf);
  default:
    return TRUE;
  }
#else
  return TRUE;
#endif
}


int IsFileSystemDumb(char *dir)
{                         
#ifdef OS2
  char drive[5], path[256], name[256], ext[8];
  
  _splitpath(dir, drive, path, name, ext);
  strcpy(name, drive);
  strcat(name, path);
  strcat(name, "DUMB.TEST.NAME");
  
  return !IsFileNameValid(name);
#else
  return FALSE;
#endif
}


#ifdef TEST
int main(void)
{
  DIR *dirp = opendir(".");
  struct dirent *dp;

  while ( (dp = readdir(dirp)) != NULL )
    printf(">%s<\n", dp -> d_name);

  return 0;
}
#endif
