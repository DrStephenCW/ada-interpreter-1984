static char rcsid[] =
"$Header:   ../RCS/ppattach.v  Revision 1.2  83/01/12  09:08:25  ada  Exp$";

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


/* $Log:	../RCS/ppattach.v $
 * Revision 1.2  83/01/12  09:08:25  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:40:58  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include "macros.h"
# include "nodes.h"
# include "int.h"

/****************************************************************
 prattach takes a node which is an attachment and returns a string
which is the printable form of that node.
****************************************************************/

char *prattach(p)
    nodep p;
{
    static char t[1000];
    
    switch (p->ndf.op) {
	case nop_comment:
	    return p->nd.comment.text;
	case nop_count:
	    sprintf(t, "* counter: %d", p->nd.counter.count);
	    return t;
	case nop_decl_callf:
	case nop_name_callf:
	case nop_stmt_callf:
	case nop_type_callf:
	case nop_expr_callf:
	case nop_top_callf:
	    sprintf(t, "!(%d) {%s}", p->nd.call_form.items, macro_pname(p));
	    return t;
	default:
	    sprintf(t, "*** invalid attachment (%s)", nodename(p));
	    return t;
	}
    }
    
