/* $Header:   ../RCS/parser.h.v  Revision 1.2  83/01/12  09:04:24  ada  Exp$ */

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


/* $Log:	../RCS/parser.h.v $
Revision 1.2  83/01/12  09:04:24  ada
Added copyright notice.

Revision 1.1  82/12/12  21:37:09  tadman
Initial revision (Arcturus release 1.3E)
 */

    /* 	This enumeration type defines the various types of constructs which
	can be accepted by the parser. */

enum parser_construct {
	interactive_input,	/* input valid at the top level of Ada */
	expression,
	name,
	declarative_part,
	sequence_of_statements,
	type_definition,
	subtype_indication,
	compilation
};
