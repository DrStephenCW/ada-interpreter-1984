static char rcsid[] =
"$Header:   ../RCS/typ.v  Revision 1.2  83/01/12  09:10:51  ada  Exp$";

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


/* $Log:	../RCS/typ.v $
 * Revision 1.2  83/01/12  09:10:51  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:48:06  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"
#include "color.h"
#include "oblist.h"

/*	Routines that deal with typep's and types.  Used to be in
	sym.c
*/

long blocktype(t)
typep t;
{
	switch(t->tyf.class) {
	    case recordform:
	    case con_arrayform:
	    case un_arrayform:
		return true;
	}
	return false;
}

long rangelength(t)  
typep t;
{
        if(t->tyf.class == srangeform || 
           t->tyf.class == enumform)
                return(t->ty.srange.ubound - t->ty.srange.lbound + 1);
        else error("bad type in rangelength");
}

typep base(t)
    typep t;
{
    while (true)
	if (t->tyf.class == srangeform)
	    t = t->ty.srange.basetype;
	else if (t->tyf.class == subtypeform)
	    t = t->ty.subtype.basetype;
	else
	    break;
    return t;
}

long compatible(a,b) 
typep a,b;
{
        if (a == b)  
                return(true);
        if ((a eq undeftype) || (b eq undeftype))  
                return(true);
        if (a->tyf.class == accessform && b->tyf.class == accessform) {
                return(compatible(a->ty.access.targettype, b->ty.access.targettype));
        }
        if (b->tyf.class == con_arrayform) {
                if(a->tyf.class == con_arrayform)
                        return(a->ty.con_array.base_type == b->ty.con_array.base_type
                          && a->tyf.size == b->tyf.size);
                else if(a->tyf.class == un_arrayform)  /* array param */
                        return(a == b->ty.con_array.base_type);
                else return(false);
        }
        return(base(a) == base(b));
}

long typecheck(a,b) 
typep a,b;
{
        if (a eq b or compatible(a,b)) return; 
	putf("Error: Type clash between ");

	if (not a) system_error("Null argument (a) to typecheck");
	else if (a->tyf.int_form) 
		dopp(a->tyf.int_form,current_output(), false, false);
	else putf("<unknown type>");

	putf(" and ");

	if (not b) system_error("Null argument (b) to typecheck");
	else if (b->tyf.int_form)
		dopp(b->tyf.int_form,current_output(), false, false);
	else putf("<unknown type>");
	putf("\n");
	error(null);
}

long	typecount;

typep newtype(form)
enum typeforms form;
{       typep p;
	typecount += maxtypesize;
        p = (typep)zalloc(maxtypesize, true);
        p->tyf.class = form;
        return(p);
}
