/* $Header:   /usr/ics/darpa/ada/RCS/macros.h.v  Revision 1.4  83/03/10  15:51:46  ada  Exp$ */

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


/* $Log:	/usr/ics/darpa/ada/RCS/macros.h.v $
Revision 1.4  83/03/10  15:51:46  ada
same as 1.3.2.1.

Revision 1.3.2.1  83/03/10  14:52:49  willson
boolean now maps to "int" instead of "char for
compatibility with Arcturus boolean representation

Revision 1.2  83/01/12  09:04:10  ada
Added copyright notice.

Revision 1.1  82/12/12  21:35:27  tadman
Initial revision (Arcturus release 1.3E)
 */

/* Willson macro file */
#define true 1L
#define false 0L
#define null 0L
#define boolean long
#define and &&
#define or ||
#define not !
#define eq ==
#define neq !=
#define _ =
