/* popen.c
 *
 * Author:  Kai Uwe Rommel <rommel@ars.de>
 * Created: Wed Aug 23 1995
 */
 
static char *rcsid =
"$Id$";
static char *rcsrev = "$Revision$";

/*
 * $Log$ 
 */

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>

#ifdef __EMX__
#include <alloca.h>
#endif
#ifdef __IBMC__
#define alloca _alloca
#endif
#ifdef __WATCOMC__
#include <malloc.h>
#define tempnam _tempnam
#endif

#include "popen.h"

#if defined(__OS2__) && !defined(OS2)
#define OS2
#endif

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32)
#define WIN32
#endif

#ifdef M_I86

FILE *popen(const char *cmd, const char *mode)
{
  return (_osmode == DOS_MODE) ? fake_popen(cmd, mode) : _popen(cmd, mode);
}

int pclose(FILE *ptr)
{
  return (_osmode == DOS_MODE) ? fake_pclose(ptr) : _pclose(ptr);
}

int pipe(int *handles)
{
  return _pipe(handles, 4096, O_BINARY);
}

#else

#ifdef OS2

#define INCL_DOS
#define INCL_NOPM
#include <os2.h>

static int pids[MAXPIPES];

FILE *popen(const char *cmd, const char *mode) 
{
  HFILE end1, end2, std, old1, old2, temp;
  FILE *file;
  RESULTCODES res;
  char fail[256], cmdline[256], *args, *shell;
  const char *ptr;
  int fmode, rc;

  for (ptr = mode; *ptr; ptr++)
    switch (*ptr)
    {
    case 'r':
      std = 1;
      break;
    case 'w':
      std = 0;
      break;
    case 't':
      fmode = O_TEXT;
      break;
    case 'b':
      fmode = O_BINARY;
      break;
    }
  
  if (DosCreatePipe(&end1, &end2, 4096))
    return NULL;

  if (std == 0) 
  {
    temp = end1; end1 = end2; end2 = temp;
  }

  old1 = -1; /* save stdin or stdout */
  DosDupHandle(std, &old1);
  DosSetFHState(old1, OPEN_FLAGS_NOINHERIT);
  temp = std; /* redirect stdin or stdout */
  DosDupHandle(end2, &temp);

  if (std == 1) 
  {
    old2 = -1; /* save stderr */
    DosDupHandle(2, &old2);
    DosSetFHState(old2, OPEN_FLAGS_NOINHERIT);
    temp = 2;   /* redirect stderr */
    DosDupHandle(end2, &temp);
  }

  DosClose(end2);
  DosSetFHState(end1, OPEN_FLAGS_NOINHERIT);

  if ((shell = getenv("COMSPEC")) == NULL)
    shell = "cmd.exe";

  strcpy(cmdline, shell);
  args = cmdline + strlen(cmdline) + 1; /* skip zero */
  strcpy(args, "/c ");
  strcat(args, cmd);
  args[strlen(args) + 1] = '\0'; /* two zeroes */

  rc = DosExecPgm(fail, sizeof(fail), EXEC_ASYNCRESULT, cmdline, 0, &res, shell);

  temp = std; /* restore stdin or stdout */
  DosDupHandle(old1, &temp);
  DosClose(old1);

  if (std == 1) 
  {
    temp = 2;   /* restore stderr */
    DosDupHandle(old2, &temp);
    DosClose(old2);
  }

  if (rc) 
  {
    DosClose(end1);
    return NULL;
  }
  
#ifdef __IBMC__
  _setmode(end1, fmode);
#endif

  file = fdopen(end1, mode);
  pids[end1] = res.codeTerminate;

  return file;
}

int pclose(FILE *pipe) 
{
  RESULTCODES rc;
  PID pid;
  int handle = fileno(pipe);

  fclose(pipe);

  if (pids[handle])
    DosWaitChild(DCWA_PROCESSTREE, DCWW_WAIT, &rc, &pid, pids[handle]);

  pids[handle] = 0;

#ifdef __EMX__
  return rc.codeTerminate == 0 ? (rc.codeResult << 8) : -1;
#else
  return rc.codeTerminate == 0 ? rc.codeResult : -1;
#endif
}

int pipe(int *handles)
{
  HFILE end1, end2;

  if (DosCreatePipe(&end1, &end2, 4096))
    return -1;

#ifdef __IBMC__
  _setmode(end1, O_TEXT);
  _setmode(end2, O_TEXT);
#endif

  handles[0] = (int) end1;
  handles[1] = (int) end2;

  return 0;
}

#else

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef __EMX__
/* #define _open_osfhandle(h, m) _imphandle(h) 
   nolonger needed with RSXNT 1.42 */
#include <mscompat.h>
#endif

PROCESS_INFORMATION pid[MAXPIPES];

FILE *popen(const char *cmd, const char *mode) 
{
  HANDLE rpipe, wpipe, handle, other;
#ifdef VERSION2
  HANDLE old0, old1, old2;
#endif
  SECURITY_ATTRIBUTES security;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  char buffer[8192];
  const char *ptr;
  int fhandle, fmode, rc;
  OSVERSIONINFO osvi;
  static int WindowsNT = -1;

  if (WindowsNT == -1)
  {
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    WindowsNT = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);
  }

  for (fmode = 0, ptr = mode; *ptr; ptr++)
    switch (*ptr)
    {
    case 'r':
      fmode |= O_RDONLY;
      break;
    case 'w':
      fmode |= O_WRONLY;
      break;
    case 't':
      fmode |= O_TEXT;
      break;
    case 'b':
      fmode |= O_BINARY;
      break;
    }
  
  security.nLength = sizeof(security);
  security.bInheritHandle = TRUE;
  security.lpSecurityDescriptor = NULL;

  if (!CreatePipe(&rpipe, &wpipe, &security, 0))
    return NULL;

  handle = (fmode & O_WRONLY) ? wpipe : rpipe;
  other  = (fmode & O_WRONLY) ? rpipe : wpipe;

  SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0);
  fhandle = _open_osfhandle((long) handle, fmode);

  if (fhandle < 0 || MAXPIPES <= fhandle)
  {
    CloseHandle(rpipe);
    CloseHandle(wpipe);
    return NULL;
  }
  
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);

  /* Both of the two following versions of redirection work fine. We keep
   * them both here just for reference. The one #ifdef'ed VERSION2 is the
   * more common one but the other one is (fail-) safer.
   */

#ifdef VERSION2
  if (fmode & O_WRONLY) 
  {
    old0 = GetStdHandle(STD_INPUT_HANDLE);
    SetStdHandle(STD_INPUT_HANDLE, rpipe);
  }
  else
  {
    old1 = GetStdHandle(STD_OUTPUT_HANDLE);
    old2 = GetStdHandle(STD_ERROR_HANDLE);
    SetStdHandle(STD_OUTPUT_HANDLE, wpipe);
    SetStdHandle(STD_ERROR_HANDLE, wpipe);
  }
#else
  si.dwFlags = STARTF_USESTDHANDLES;

  if (fmode & O_WRONLY) 
  {
    si.hStdInput  = rpipe;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
  }
  else
  {
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = wpipe;
    si.hStdError  = wpipe;
  }
#endif

  /* Using anonymous pipes and CreateProcess to redirect stdin or stdout does
   * only work without any restrictions on Windows NT. On Windows 95, it
   * does only work if the child process is also a Win32 application and not
   * an old 16-bit executable. The problem is, that the Windows 95 command
   * line interpreter or shell (command.com) is also an old 16-bit program.
   * The semantics of popen() normally require execution through a shell
   * but that can never work on Windows 95 because of this restriction.
   * Therefore, if we do not run on Windows NT, we must try to run the command
   * without a shell to at least cover _some_ cases. Otherwise we just fail.
   */

  if (!WindowsNT)
    rc = CreateProcess(NULL, (char *) cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
  else
  {
    if ((ptr = getenv("COMSPEC")) == NULL)
      ptr = "cmd.exe";

    strcpy(buffer, ptr);
    strcat(buffer, " /c ");
    strcat(buffer, cmd);

    rc = CreateProcess(ptr, buffer, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
  }

#ifdef VERSION2
  if (fmode & O_WRONLY)
    SetStdHandle(STD_INPUT_HANDLE, old0);
  else
  {
    SetStdHandle(STD_OUTPUT_HANDLE, old1);
    SetStdHandle(STD_ERROR_HANDLE, old2);
  }
#endif

  if (rc == 0)
  {
    CloseHandle(rpipe);
    CloseHandle(wpipe);
    return NULL;
  }
  
  CloseHandle(other);

  pid[fhandle] = pi;

  return fdopen(fhandle, (char *) mode);
}

int pclose(FILE *pipe) 
{
  PROCESS_INFORMATION pi;
  DWORD rc;
  int fhandle = fileno(pipe);

  if (fhandle < 0 || MAXPIPES <= fhandle)
    return -1;

  fclose(pipe);

  pi = pid[fhandle];

  WaitForSingleObject(pi.hProcess, INFINITE);
  GetExitCodeProcess(pi.hProcess, &rc);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

#ifdef __EMX__
  return rc << 8;
#else
  return rc;
#endif
}

int pipe(int *handles)
{
  HANDLE rpipe, wpipe;
  SECURITY_ATTRIBUTES security;

  security.nLength = sizeof(security);
  security.bInheritHandle = TRUE;
  security.lpSecurityDescriptor = NULL;

  if (!CreatePipe(&rpipe, &wpipe, &security, 0))
    return -1;

  handles[0] = _open_osfhandle((long) rpipe, O_RDONLY);
  handles[1] = _open_osfhandle((long) wpipe, O_WRONLY);

  return 0;
}

#else

FILE *popen(const char *cmd, const char *mode)
{
  return fake_popen(cmd, mode);
}

int pclose(FILE *pipe)
{
  return fake_pclose(pipe);
}

int pipe(int *handles)
{
  return -1;
}

#endif
#endif
#endif

typedef enum { unopened = 0, reading, writing } pipemode;

static struct
{
  char *name;
  char *cmd;
  pipemode pmode;
}
pipes[MAXPIPES];

FILE *fake_popen(const char *cmd, const char *mode)
{
  FILE *current;
  char *name;
  int cur;
  pipemode curmode;

  if(strchr(mode, 'r') != NULL)
    curmode = reading;
  else if(strchr(mode, 'w') != NULL)
    curmode = writing;
  else
    return NULL;

  if ((name = tempnam(NULL, "pi")) == NULL)
    return NULL;

  if(curmode == reading)
  {
    char *line = alloca(strlen(cmd) + strlen(name) + 4);
    sprintf(line, "%s > %s", cmd, name);
    system(line);

    if((current = fopen(name, mode)) == NULL)
      return NULL;
  }
  else
  {
    if((current = fopen(name, mode)) == NULL)
      return NULL;
  }

  cur = fileno(current);
  pipes[cur].name = name;
  pipes[cur].cmd = strdup(cmd);
  pipes[cur].pmode = curmode;

  return current;
}

int fake_pclose(FILE *pipe)
{
  int cur = fileno(pipe), rval;

  if(pipes[cur].pmode == unopened)
    return -1;

  if(pipes[cur].pmode == reading)
  {
    rval = fclose(pipe);
    unlink(pipes[cur].name);
  }
  else
  {
    char *line = alloca(strlen(pipes[cur].cmd) + strlen(pipes[cur].name) + 4);
    fclose(pipe);
    sprintf(line, "%s < %s", pipes[cur].cmd, pipes[cur].name);
    rval = system(line);
    unlink(pipes[cur].name);
  }

  free(pipes[cur].name);
  free(pipes[cur].cmd);
  pipes[cur].pmode = unopened;

  return rval;
}

#ifdef TEST

#include <conio.h>

int main(void)
{
  FILE *pipe;
  int x = 0;
  char line[256];

  pipe = popen("diff.exe", "r");

  while (fgets(line, sizeof(line), pipe))
  {
    fputs(line, stdout);
    x++;
  }

  printf("%d lines, rc = %d\n", x, pclose(pipe));
  fflush(stdout);
  getch();

  pipe = popen("more.com", "w");

  for (x = 0; x < 32; x++)
    fprintf(pipe, "line %d\n", x);

  printf("rc = %d\n", pclose(pipe));

  return 0;
}

#endif

/* end of popen.c */
