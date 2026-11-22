static char rcsid[] =
"$Header:   ../RCS/attach.v  Revision 1.2  83/01/12  09:11:01  ada  Exp$";

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


/* $Log:	../RCS/attach.v $
 * Revision 1.2  83/01/12  09:11:01  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:16:39  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include "macros.h"
# include "nodes.h"
# include "int.h"
# include "sym.h"

/****************************************************************
 This file contains functions to manipulate the list of attachments
attached to each node of the internal form.
****************************************************************/


int apply_attach(p,fn,op)	/* apply the function fn to all attachments
				   of type op attached to the node pointed to
				   by p.  fn is a function of one argument, a
				   nodep. Returns the number of such 
				   attachments. */
    nodep p;
    int (*fn)();
    enum tokentypes op;
{
    int howmany = 0;

    p = p->ndf.ag;
    while (p) {
	if (p->ndf.op == op) {
	    (*fn)(p);
	    howmany++;
	}
	p = p->ndf.next;
    }
    return howmany;
}


apply_all_attach(p,fn)		/* apply fn, a function of one argument, to
				   all attachments attached to the node
				   pointed to by p */
    register nodep p;
    int (*fn)();
{
    register nodep q;

    p = p->ndf.ag;
    while (p) {
	q = p->ndf.next;	/* apply_all_attach is coded so that it
				   can be used to free all of the attachments
				   associated with a node.  Therefore, the
				   ndf.next link is saved before the function
				   is applied */
	(*fn)(p);
	p = q;
    }
}

nodep find_attach(p,op)	        /* find the first attachment
				   of type op attached to the node pointed to
				   by p.  Returns a pointer to the 
				   attachment. */
    nodep p;
    enum tokentypes op;
{
    p = p->ndf.ag;
    while (p) {
	if (p->ndf.op == op) return p;
	p = p->ndf.next;
    }
    return null;
}

nodep extract_attach(p,op)	/* remove the first attachment of type op 
				   from the node pointed to by p and return
				   it */
    nodep p;
    enum tokentypes op;
{
    nodep *q = &(p->ndf.ag);
    
    while (*q) {
	if ((*q)->ndf.op == op) {
	    nodep r = *q;
	    
	    *q=(*q)->ndf.next;
	    r->ndf.next = null;
	    return r;
	}
	q = &((*q)->ndf.next);
    }
    return null;
}

remove_attach(p,op)		/* remove all attachments of type op from the
				   node pointed to by p */
    nodep p;
    enum tokentypes op;
{
    nodep *q = &(p->ndf.ag);
    
    while (*q) {
	if ((*q)->ndf.op == op) {
	    nodep r = *q;
	    
	    *q=(*q)->ndf.next;
	    free(r);
	}
	else q = &((*q)->ndf.next);
    }
}

insert_attach(p,a)		/* attach the attachment pointed to by a to
				   the beginning of the list of attachments
				   attached to the node pointed to by p */
    nodep p,a;
{
    a->ndf.next = p->ndf.ag;
    p->ndf.ag = a;
}

append_attach(p,a)		/* attach the attachment pointed to by a to
				   the end of the list of attachments
				   attached to the node pointed to by p.  If
				   order doesn't matter, use insert_attach
				   instead of this function */
    nodep p,a;
{
    nodep *q = &(p->ndf.ag);
    
    while (*q) q = &((*q)->ndf.next);
    *q = a;
    a->ndf.next = null;
}
