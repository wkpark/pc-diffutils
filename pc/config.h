/* emx configuration */

#ifdef __CRTRSXNT__
#include <crtrsxnt.h>
#undef optarg
#undef optind
#undef opterr
#endif

#include <malloc.h>

#ifdef __EMX__
#include <sys/emxload.h>
#ifdef __RSXNT__
#define _emxload_env(x)
#endif
#define arginit(pargc, pargv) \
    _response (pargc, pargv); \
    _wildcard (pargc, pargv); \
    _emxload_env ("RCSLOAD");
#else
#define arginit(pargc, pargv)
#endif

#define initialize_main(pargc, pargv) \
  { \
    arginit(pargc, pargv); \
    setvbuf (stdout, NULL, _IOFBF, BUFSIZ); \
    if (*pargc == 1) { \
      program_name = (*pargv)[0]; \
      printf ("\n%s\n\n", version_string); \
      usage (); exit(0); \
    } \
  }

/* diffutils doesn't need re_comp,
   but the shared library that diffutils builds
   is used by other (old-fashioned) programs that need re_comp.  */
#define _REGEX_RE_COMP

#define STAT_BLOCKSIZE(s) 65536

/* config.h for PC operating systems */

/* Define if using alloca.c.  */
#undef C_ALLOCA

/* Define if the closedir function returns void instead of int.  */
#undef CLOSEDIR_VOID

/* Define to empty if the keyword does not work.  */
#undef const

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
#undef CRAY_STACKSEG_END

/* Define if you have alloca, as a function or macro.  */
#undef HAVE_ALLOCA

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#undef HAVE_ALLOCA_H

/* Define if you don't have vprintf but do have _doprnt.  */
#undef HAVE_DOPRNT

/* Define if your struct stat has st_blksize.  */
#undef HAVE_ST_BLKSIZE

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#undef HAVE_SYS_WAIT_H

/* Define if you have <vfork.h>.  */
#undef HAVE_VFORK_H

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define if on MINIX.  */
#undef _MINIX

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
#undef _POSIX_1_SOURCE

/* Define if you need to in order for stat and other things to work.  */
#undef _POSIX_SOURCE

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown
 */
#undef STACK_DIRECTION

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
#undef STAT_MACROS_BROKEN

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define vfork as fork if vfork does not work.  */
#define vfork fork

/* Define if you have the dup2 function.  */
#define HAVE_DUP2 1

/* Define if you have the memchr function.  */
#define HAVE_MEMCHR 1

/* Define if you have the sigaction function.  */
#undef HAVE_SIGACTION

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the tmpnam function.  */
#define HAVE_TMPNAM 1

/* Define if you have the <dirent.h> header file.  */
#ifndef _MSC_VER
#define HAVE_DIRENT_H 1
#endif

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <libintl.h> header file.  */
#undef HAVE_LIBINTL_H

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
#undef HAVE_LOCALE_H

/* Define if you have the <ndir.h> header file.  */
#ifdef _MSC_VER
#define HAVE_NDIR_H 1
#endif

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/dir.h> header file.  */
#ifndef _MSC_VER
#define HAVE_SYS_DIR_H 1
#endif

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <time.h> header file.  */
#define HAVE_TIME_H 1

/* Define if you have the <unistd.h> header file.  */
#if !defined(_MSC_VER) && !defined(__IBMC__)
#define HAVE_UNISTD_H 1
#endif

/* Define if you have the intl library (-lintl).  */
#undef HAVE_LIBINTL


/* PC specific definitions */

#include <io.h>
#include <process.h>

/* The PC tradition is that file names that differ only in case are the same,
   and using `stricmp' is required for proper behavior with HPFS.
   OS/2 does not strictly forbid case sensitive file systems like NFS,
   but it's too painful to get this right, unfortunately.  */
#define filename_cmp(a, b) stricmp (a, b)

char *filename_lastdirchar (char const *);
#define filename_lastdirchar (filename_lastdirchar)

#define HAVE_FORK 0

#define HAVE_SETMODE 1
#ifndef _MSC_VER
int setmode (int, int);
#endif

#define same_file(s,t) (-1)

#define TMPDIR_ENV "TEMP"

#define PR_PROGRAM "pr"
#define DIFF_PROGRAM "diff.exe"
#define NULL_DEVICE "nul"
#define DEFAULT_EDITOR_PROGRAM "edit"

#ifdef _MSC_VER
#define alloca _alloca
#define getenv getenv
#endif

#ifdef __IBMC__

#define S_IFMT 0xF000
#define EISDIR EINVAL

#define WEXITSTATUS(stat_val) ((stat_val) & 0xFF)
#define WIFEXITED(stat_val) (((stat_val) & 0xFF00) == 0)

#endif

#include <stdio.h>
#include <popen.h>
