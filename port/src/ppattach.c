static char rcsid[] =
"$Header:   ../RCS/ppattach.v  Revision 1.2  83/01/12  09:08:25  ada  Exp$";

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
    
