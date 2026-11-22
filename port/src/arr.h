/* $Header:   ../RCS/arr.h.v  Revision 1.3  83/01/12  09:03:23  ada  Exp$ */

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


/* $Log:	../RCS/arr.h.v $
Revision 1.3  83/01/12  09:03:23  ada
Added copyright notice.

Revision 1.2  83/01/11  15:38:34  ada
Same as 1.1.2.1.

Revision 1.1.2.1  83/01/11  15:33:07  ada
Removed size field, put in _first and _last instead.

Revision 1.1  82/12/12  21:28:42  tadman
Initial revision (Arcturus release 1.3E)
 */

#define arr_size(arr) ((arr)->arr_last - (arr)->arr_first + 1)

/*	Arrays are always integer arrays, even for characters! */

typedef struct {
	int arr_first;
	int arr_last;
	int *arr_base;
} *arrayp;
