/* $Header:   /usr/ics/darpa/ada/RCS/macros.h.v  Revision 1.4  83/03/10  15:51:46  ada  Exp$ */

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
#define true 1
#define false 0
#define null 0
#define boolean int
#define and &&
#define or ||
#define not !
#define eq ==
#define neq !=
#define _ =
