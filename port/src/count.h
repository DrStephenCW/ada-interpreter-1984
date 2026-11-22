/* $Header:   ../RCS/count.h.v  Revision 1.2  83/01/12  09:03:41  ada  Exp$ */

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


/* $Log:	../RCS/count.h.v $
Revision 1.2  83/01/12  09:03:41  ada
Added copyright notice.

Revision 1.1  82/12/12  21:32:10  tadman
Initial revision (Arcturus release 1.3E)
 */

int     *countingp;
#define counting (*countingp)

int max_node_counts;
