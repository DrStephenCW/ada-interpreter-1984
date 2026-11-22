/* $Header:   ../RCS/castor.h.v  Revision 1.2  83/01/12  09:03:32  ada  Exp$ */

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


/* $Log:	../RCS/castor.h.v $
Revision 1.2  83/01/12  09:03:32  ada
Added copyright notice.

Revision 1.1  82/12/12  21:31:03  tadman
Initial revision (Arcturus release 1.3E)
 */

  /* castor_class is an enumeration of the syntatic classes which can
     be used for macro substitution in Castor.  After the parser produces
     a tree, it must be classified into one of these types so that it
     can be checked against the type of the corresponding substitution
     point when it is referenced */

typedef enum  {
	cas_unkn,		/* unknown (illegal for macro substitution) */
	cas_stmt,		/* statement */
	cas_decl,		/* declaration */
	cas_name,		/* name */
	cas_type,		/* type */
} castor_class;

