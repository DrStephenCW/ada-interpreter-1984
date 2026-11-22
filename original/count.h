/* $Header:   ../RCS/count.h.v  Revision 1.2  83/01/12  09:03:41  ada  Exp$ */

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


/* $Log:	../RCS/count.h.v $
Revision 1.2  83/01/12  09:03:41  ada
Added copyright notice.

Revision 1.1  82/12/12  21:32:10  tadman
Initial revision (Arcturus release 1.3E)
 */

int     *countingp;
#define counting (*countingp)

int max_node_counts;
