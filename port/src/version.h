/* $Header:   ../RCS/version.h.v  Revision 1.2  83/01/12  09:04:53  ada  Exp$ */

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


/* $Log:	../RCS/version.h.v $
Revision 1.2  83/01/12  09:04:53  ada
Added copyright notice.

Revision 1.1  82/12/12  21:40:53  tadman
Initial revision (Arcturus release 1.3E)
 */

 /* The major version incremented for major system changes.  The minor
    version is incremented for all other *functional* changes.  The
    fixversion is incremented for new versions with bug fixes *only*. */

extern char systemname[], sysdate[], systemtype[];
extern int majorversion, minorversion;
extern char fixversion;
