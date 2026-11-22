static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/cassym.v  Revision 1.3  84/01/12  16:45:59  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/cassym.v $
 * Revision 1.3  84/01/12  16:45:59  ada
 * same as 1.2.4.1
 * 
 * Revision 1.2.4.1  84/01/06  16:13:53  tadman
 * Changes for static symbol tables.  In particular, get the parent
 * internal form from the return stack rather than the dynamic links.
 * 
 * Revision 1.2  83/01/12  09:12:04  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:18:17  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"
#include "oblist.h"

char *macro_hash_name(p)
    nodep p;	/* macro designator or calling form */
{
    char name[1000];

    name[0] = '@';
    name[1] = '\0';
    
    p = p->nd.call_form.list;
    while (p) {
	if (p->ndf.op == nop_callf_ident)
	    strcat(name, p->nd.id_decl.id);	/* should use strncat */
	else
	    strcat(name, "()");
	p = p->ndf.next;
	if (p) strcat(name,"%");
    }
    return intern(name);
}

char *macro_pname(p)
    nodep p;	/* macro designator or calling form */
{
    static char pname[1000];		/* Gene likes long calling forms */

    pname[0] = '\0';
    p = p->nd.call_form.list;
    while (p) {
	if (p->ndf.op == nop_callf_ident)
	    strcat(pname, p->nd.id_decl.id);	/* should use strncat */
	else
	    strcat(pname, "()");
	p = p->ndf.next;
	if (p) strcat(pname," ");
    }
    return pname;
}

fixp macro_lookup(id,fn,a)
    char *id;
    boolean (*fn)();
    int a;
{
    register fixp f;
    register usep up;
    retsp r;
    symtabp tab;

    for (r = retstk; r; r = r -> link) {
	tab = r -> currtab;
	if (tab && tab -> symtab_owner &&
	    tab -> symtab_owner -> fxf.class == macro_id) break;
    }

    if (tab == null)
	error ("genxxxx not called in macro");

    if (f = lookuptab(tab, id, fn, a)) return f;
# ifdef SKIP
    up = tab -> use_list;
    while (up != null) {
	if (f = lookuptab(up, id, fn, a))
	    return f->fxf.dyna;
	up = up -> symtab_dyn_link;
    }
# endif SKIP
    return null;
}

    
    
    
