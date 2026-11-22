/* $Header:   ../RCS/handler.h.v  Revision 1.2  83/01/12  09:04:01  ada  Exp$ */

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


/* $Log:	../RCS/handler.h.v $
Revision 1.2  83/01/12  09:04:01  ada
Added copyright notice.

Revision 1.1  82/12/12  21:34:30  tadman
Initial revision (Arcturus release 1.3E)
 */


boolean	control_c_was_typed;
		/* true when we've trapped a control-c and want to */
		/* invoke the stack of exception handlers for control-c */
