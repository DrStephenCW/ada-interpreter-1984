/* $Header:   /usr/ics/darpa/ada/RCS/adaio.h.v  Revision 1.4  83/03/10  15:05:29  ada  Exp$ */

/*
 * Arcturus was written by the UC Irvine Arcturus team (Programming
 * Environment Project, 1982-84).  See CREDITS.md for who they were.
 * [2026: the original copyright notice was replaced by this pointer.]
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */


/* $Log:	/usr/ics/darpa/ada/RCS/adaio.h.v $
Revision 1.4  83/03/10  15:05:29  ada
Same as 1.3.2.1.

Revision 1.3.2.1  83/03/10  14:42:09  willson
Ada 82 i/o support

Revision 1.2  83/01/12  09:03:20  ada
Added copyright notice.

Revision 1.1  82/12/12  21:28:16  tadman
Initial revision (Arcturus release 1.3E)
 */

#include "color.h" /* since this file needs color.h always */
#define std_adain  0
#define std_adaout 1
#define std_adamsg 2
#define adaeof 0
#define adaeos 1

typedef	enum {in_file, out_file, inout_file} file_mode;

typedef struct {
	int ibufp;
	int ibuflast;
	char *ibuf;
	int what;  /* like file or window */
	int which; /* A UNIX file or "tty" window */
} infile_des, outfile_des, *FILE_TYPE;

FILE_TYPE adaout;
FILE_TYPE adain;
FILE_TYPE adamsg;

