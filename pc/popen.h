/* popen.h
 *
 * Author:  Kai Uwe Rommel <rommel@ars.de>
 * Created: Wed Aug 23 1995
 */
 
/* $Id$ */

/*
 * $Log$ 
 */

#ifndef _POPEN_H
#define _POPEN_H

#define MAXPIPES 256

FILE *popen(const char *cmd, const char *mode);
int pclose(FILE *pipe);
int pipe(int *handles);

FILE *fake_popen(const char *cmd, const char *mode);
int fake_pclose(FILE *pipe);

#endif /* _POPEN_H */

/* end of popen.h */
