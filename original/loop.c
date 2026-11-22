static char rcsid[] =
"$Header:   /usr/ics/darpa/ada/RCS/loop.v  Revision 1.4  83/05/06  14:35:21  ada  Exp$";

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


/* $Log:	/usr/ics/darpa/ada/RCS/loop.v $
 * Revision 1.4  83/05/06  14:35:21  ada
 * same as 1.2.40.1
 * 
 * Revision 1.2.40.1  83/04/12  12:12:12  tadman
 * Fixed a problem with symbol tables and loops without scopes (plain loop
 * and while).  A layer of symbolforms was being popped off the current
 * scope when such loops were exited.
 * 
 * Revision 1.2  83/01/12  09:17:17  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:32:10  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"

extern  typep tyeval ();

extern nodep intstk[], *intsp, *intstk_end;
extern int stack_level;

   /* runloop, runwhile, and runfor execute the three types of loops.
      doloop, dowhile, and dofor push an entry on the interpreter return
      stack and then call the corresponding runxxx function protected
      by a catch.  This stack entry and catch are used by the exit
      statement to terminate the loop.  For a detailed description of the
      return stack, see docall. 

      The poptabs call clears any symbol tables from the stack from
      higher levels.  It is needed in the case that exit is used to
      exit from one or more for loops or blocks */ 

static runloop(p)
    nodep p;
{
    while (true)
	eval (p -> nd.whilex.bodyx, eval_mode, &dummy, null);
}

doloop(p)
    nodep p;
{
    struct retstkform   rets;

    do_comp_statement(&rets, p, runloop);
}

        /* for loops have their own scope; we create a symbol table for
           the loop control variable.	*/

static runwhile(p)
    nodep p;
{
    typedval r;

    while (true) {
	eval (p -> nd.whilex.expr, eval_mode, &r, null);
	typecheck (r.typ, booltype);
	if (not r.val.integer)
	    break;
	eval (p -> nd.whilex.bodyx, eval_mode, &dummy, null);
    }
}

dowhile(p)
    nodep p;
{
    struct retstkform   rets;

    do_comp_statement(&rets, p, runwhile);
}


static int for_limit, *for_loopvarp;

static runfor(p)
    nodep p;
{
    register int limit = for_limit;
    register int *loopvarp = for_loopvarp;

    if (*loopvarp <= limit)
	do
	    eval (p -> nd.forx.bodyx,
		    eval_mode,
		    &dummy,
		    null);
	while (++(*loopvarp) <= limit);
}

static runforrev(p)
    nodep p;
{
    register int    limit = for_limit;
    register int   *loopvarp = for_loopvarp;

    if (*loopvarp >= limit)
	do
	    eval (p -> nd.forx.bodyx,
		    eval_mode,
		    &dummy,
		    null);
	while (--(*loopvarp) >= limit);
}

dofor(p)
    nodep p;
{
    struct retstkform rets;
    typep rngtyp;
    fixp f;
    symp s;
    symtabp tab;
    symtabp save_currtab = currtab;

    if (p -> nd.forx.table eq null) {
	tab = newtab (4, null);	/* no owner */
    /* make table small enough not to  waste space, but large enough  that
       lookups of variables other than the control var fail sooner */

	tab->symtab_static_link = currtab;
	p -> nd.forx.table _ tab;
    }

    currtab = p->nd.forx.table;

    f = newfix(varfixform, obj_id);
    s = newsym (varsymbolform, obj_id);
    f -> fxf.name = p -> nd.forx.cntrl_var;
    enter(currtab, f, s);

    rngtyp = tyeval (p -> nd.forx.lims, null);
    s -> sy.var.type = rngtyp;
    for_loopvarp = s -> sy.var.value;
    if (p -> nd.forx.reversex) {/* may count backwards */
	*for_loopvarp = rngtyp -> ty.srange.ubound;
	for_limit = rngtyp -> ty.srange.lbound;
    	do_comp_statement(&rets, p, runforrev);
    }
    else {
	*for_loopvarp = rngtyp -> ty.srange.lbound;
	for_limit = rngtyp -> ty.srange.ubound;
    	do_comp_statement(&rets, p, runfor);
    }
    poplevel(currtab, stack_level);
    currtab = save_currtab;
}

    /* doexit executes the exit statement.  It first checks the condition,
       if any and just quits if it is false.  Otherwise, it searches the
       return stack for a loop, or in the case of an exit with a loop_name,
       a loop with the requested label.  If it finds an entry for a
       subprogram or macro before finding the loop, it complains because
       an exit cannot exit from these constructs.  However, it is possible
       to exit from a block.  */

doexit(p)
    nodep p;
{
    typedval r;
    register retsp ret = retstk;
    char *exitwhat;

    if (p->nd.exitx.cond) {
	eval (p -> nd.exitx.cond, eval_mode, &r, null);
	typecheck (r.typ, booltype);
	if (!r.val.integer) return;
    }

    if (p->nd.exitx.loop_name == null) exitwhat = null;
    else if (p->nd.exitx.loop_name->ndf.op == nop_ident)
	exitwhat = p->nd.exitx.loop_name->nd.ident.id;
    else error("Ask Steve Willson to implement dot notation for labels");

    while (ret) {
	enum tokentypes op = ret -> p -> ndf.op;

	if (op == nop_function || op == nop_procedure || op == nop_macro) 
	    break;
	if (op == nop_loop || op == nop_while || op == nop_for) {
	    char *loopname =
		(op == nop_for ? ret->p->nd.forx.label :
				 ret->p->nd.whilex.label);

	    if (exitwhat == null || exitwhat == loopname)
		unwind_retstk(ret,1);
	}
        
	ret = ret->link;
    }
    if (exitwhat)
	error("Exit statement cannot find label %s", exitwhat);
    else
	error("Exit statement not in a loop");
}


runif(p)
    nodep p;
{
    typedval r;

    eval (p -> nd.elsifx.expr, eval_mode, &r, null);
    typecheck (r.typ, booltype);
    if (r.val.integer)
	eval (p -> nd.elsifx.then_part, eval_mode, &dummy, null);
    else
	eval (p -> nd.ifx.elsif_list, &dummy, null);
}

doif(p)
    nodep p;
{
    struct retstkform   rets;

    do_comp_statement(&rets, p, runif);
}
