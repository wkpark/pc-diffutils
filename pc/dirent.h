/* @(#) dirent.h
 * public domain implementation of POSIX directory routines 
 * for DOS, OS/2 and Win32
 */

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#ifndef _POSIX_NAME_MAX
#define _POSIX_NAME_MAX  256
#endif
#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX  260
#endif

struct dirent
{
  ino_t    d_ino;                         /* a bit of a farce */
  int      d_reclen;                      /* more farce */
  int      d_namlen;                      /* length of d_name */
  char     d_name[_POSIX_NAME_MAX + 1];   /* null terminated */
  /* nonstandard fields */
  long     d_size;                        /* size in bytes */
  unsigned d_mode;                        /* file attributes */
  unsigned d_time;                        /* file time */
  unsigned d_date;                        /* file date */
};

/* The fields d_size, d_mode, d_time and d_date are extensions.
 * The find_first and find_next calls deliver this data without
 * any extra cost. If this data is needed, these fields save a lot
 * of extra calls to stat() (each stat() again does a find_first!).
 */

struct d_dircontents
{
  char *d_dc_entry;
  long  d_dc_size;
  unsigned d_dc_mode;
  unsigned d_dc_time;
  unsigned d_dc_date;
  struct d_dircontents *d_dc_next;
};

struct d_dirdesc
{
  int   d_dd_id;           /* uniquely identify each open directory */
  long  d_dd_loc;          /* where we are in directory entry is this */
  struct d_dircontents *d_dd_contents; /* pointer to contents of dir */
  struct d_dircontents *d_dd_cp;       /* pointer to current position */
  char d_dd_name[_POSIX_PATH_MAX];     /* remember name for rewinddir */
};

typedef struct d_dirdesc DIR;


extern DIR *opendir(const char *);
extern struct dirent *readdir(DIR *);
extern void rewinddir(DIR *);
extern int closedir(DIR *);

#ifndef _POSIX_SOURCE

extern void seekdir(DIR *, long);
extern long telldir(DIR *);

#ifndef _A_NORMAL
#define _A_NORMAL   0x00
#define _A_RONLY    0x01
#define _A_HIDDEN   0x02
#define _A_SYSTEM   0x04
#define _A_LABEL    0x08
#define _A_DIR      0x10
#define _A_ARCHIVE  0x20
#endif

extern int attributes;

extern int scandir(char *, struct dirent ***,
                   int (*)(struct dirent *),
                   int (*)(struct dirent *, struct dirent *));

extern int getfmode(char *);
extern int setfmode(char *, unsigned);

extern int IsFileNameValid(char *name);
extern int IsFileSystemDumb(char *dir);

#endif
