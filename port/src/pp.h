/* $Header:   ../RCS/pp.h.v  Revision 1.2  83/01/12  09:04:28  ada  Exp$ */

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


/* $Log:	../RCS/pp.h.v $
Revision 1.2  83/01/12  09:04:28  ada
Added copyright notice.

Revision 1.1  82/12/12  21:37:31  tadman
Initial revision (Arcturus release 1.3E)
 */

enum pp_color_use {
	ppc_none,
	ppc_count,
	ppc_random,
 	ppc_circular,
	ppc_depth,
};

enum pp_capitalize {
	ppcap_lower,
	ppcap_upper,
	ppcap_initial,
	ppcap_mixed,
};
