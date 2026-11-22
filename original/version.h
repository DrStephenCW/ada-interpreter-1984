/* $Header:   ../RCS/version.h.v  Revision 1.2  83/01/12  09:04:53  ada  Exp$ */

/*******************************************************************
 *
 * Copyright 1983      Thomas A. Standish
 *		       Programming Environment Project
 *                     Department of Information and Computer Science
 *                     University of California
 *                     Irvine, California  92717
 *
 * All rights reserved. No part of this software may be sold or distributed
 * in any form or by any means without the prior written permission of the 
 * author.
 *
 * Report problems and direct all inquiries to ada%uci@rand-relay (ARPA net).
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
