/* $Header:   ../RCS/handler.h.v  Revision 1.2  83/01/12  09:04:01  ada  Exp$ */

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


/* $Log:	../RCS/handler.h.v $
Revision 1.2  83/01/12  09:04:01  ada
Added copyright notice.

Revision 1.1  82/12/12  21:34:30  tadman
Initial revision (Arcturus release 1.3E)
 */


boolean	control_c_was_typed;
		/* true when we've trapped a control-c and want to */
		/* invoke the stack of exception handlers for control-c */
