/* sgtty.h -- just enough of the 4.2BSD terminal interface for error.c.
   The interpreter only uses it to put the terminal back into normal
   (echoing, line-buffered) mode, which is what a modern terminal is
   already in, so gtty and stty do nothing. */
#ifndef _COMPAT_SGTTY_H
#define _COMPAT_SGTTY_H
struct sgttyb { int sg_flags; };
#define ECHO	0x08
#define CBREAK	0x02
#define CRMOD	0x10
#define gtty(fd, p)	0
#define stty(fd, p)	0
#endif
