static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/scope.v  Revision 1.9  83/09/21  10:01:36  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/scope.v $
 * Revision 1.9  83/09/21  10:01:36  ada
 * same as 1.8.9.1
 * 
 * Revision 1.8.9.1  83/09/12  15:28:57  ritter
 * Added code to fix propogation of exceptions raised in handlers.
 * 
 * 
 * 
 * Revision 1.8  83/09/08  15:51:51  ada
 * same as 1.7.9.1
 * 
 * Revision 1.7.9.1  83/08/25  14:32:33  ritter
 * Fixed handling of exceptions that occur while already inside of a handler.
 * 
 * Revision 1.7  83/06/14  21:27:28  tadman
 * Fix typo in call to eval for context specification.
 * 
 * Revision 1.6  83/06/14  16:32:43  ada
 * Same as 1.5.4.1.
 * 
 * Revision 1.5.4.1  83/06/14  14:15:01  tadman
 * Added code to "elaborate" context specifications to doscope.
 * 
 * Revision 1.5  83/05/06  14:44:55  ada
 * same as 1.4.4.1
 * 
 * Revision 1.4.4.1  83/04/12  12:07:25  tadman
 * Fixed a problem with symbol tables and loops without scopes.  Such
 * loops (plain loop and while) would cause a layer of symbolforms
 * to be popped off the current scope.
 * 
 * Revision 1.4  83/03/15  16:15:18  ada
 * same as 1.3.2.1
 * 
 * Revision 1.3.2.1  83/03/15  16:12:23  willson
 * Fixed bug with return types from exceptions.
 * 
 * Revision 1.3  83/02/04  14:50:27  ada
 * same as 1.2.4.1
 * 
 * Revision 1.2.4.1  83/02/04  13:05:11  tadman
 * Removed some obsolete diagnostic code.
 * 
 * Revision 1.2  83/01/12  09:09:08  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:43:26  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"

extern nodep intstk[], *intsp, *intstk_end;

extern nodep handler;		/* exception handler -- set by raise */

long stack_level = 0;

  /* 	doscope does the evaluation of a scope.  It is also used around
	for if, loop, while, and case statements.  Although these are not
	scopes of local declarations, they can be exited via exit and
	goto statements and therefore need the same kind of catch/throw
	protection as scopes do.  */
	
long doscope(rets, int_form, context, decls, body, result)
    retsp rets;
    nodep int_form;		/* pointer to internal form of subprogram,
				   package, loop, etc. */
    nodep context;		/* context specification */
    nodep decls;		/* pointer to internal form for locals */
    nodep body;			/* pointer to internal form for body */
    typedval *result;
{
    long exception;		/* return value from unwind_retstk */

    /* evaluate the context specificaton */

    eval (context, decl_mode, &dummy, null);

   /* evaluate the locals into currtab */

    eval (decls, decl_mode, &dummy, null);

  /* link new entry into stack */

    rets->link = retstk;
    retstk = rets;
    stack_level++;
 
  /* preserve return value of outer subprogram */

    rets->retvalsav = retval;
    retval _ result;

    rets->p = int_form;		/* save pointer to internal form */
    rets->currtab = currtab;	/* remember current table */

    rets->intsp_save = intsp;	/* save interpreter stack position
				   around catch & throw */

/* PAR 22-Aug-83 */
/* old code */
/*  rets->exception = null;	/* not in an exception handler */
/* end of old code */

/* new code */
    /*  This code says 1) if we are entering a procedure or function then
			   a raise; (with no exception name) is no longer
			   legal even if we are still in an exception handler.
		       2) If we are entering a procedure, function or block
			   then we must not automatically propogate new 
			   exceptions that happen inside of a handler
			   since each of these could have their own handlers 
			   and need first crack at handling the problem
    */
    
    if(rets->link == null ||
	int_form->ndf.op == nop_function ||
	int_form->ndf.op == nop_procedure)
	    rets->exception = null;	/* no longer in a handler */
    else
	    rets->exception = rets->link->exception;

    if(rets->link == null ||
	int_form->ndf.op == nop_function ||
	int_form->ndf.op == nop_procedure ||
	int_form->ndf.op == nop_declare)
	    rets->must_prop_exception = null;
    else
	    rets->must_prop_exception = rets->link->must_prop_exception;

/* end of new code */
 
   exception = catch (&rets->retenv,	/* do the body of the scope */
	    eval,
	    body,
	    eval_mode,
	    &dummy,
	    null);

    if (exception > 1) {
	intsp = rets->intsp_save;	/* restore interpreter 
					   stack position */
	/* retval = rets->retvalsav;	/* restore return value */
	/* bug fix: the reason I left it in as a comment is that I
	   don't quite understand this stuff. */
	retstk = rets;			/* restore return stack position */
	/* next line added by PAR 9/6/83 */
	retstk->must_prop_exception = 1; /* cannot handle another one here */

	exception = catch (&rets->retenv,	/* do exception handler */
		eval,
		handler,	/* global set by raise */
		eval_mode,
		&dummy,
		null);
    }

    retval = rets->retvalsav;	/* restore return value */
    retstk = rets->link;	/* pop the return stack */
    intsp = rets->intsp_save;	/* restore the interpreter stack position */

    stack_level--;
}

long do_comp_statement(rets, int_form, fn)
    retsp rets;		/* return stack entry */
    nodep int_form;	/* internal form of compound statement */
    long (*fn)();	/* function to interpret compound statement */
{
  /* link new entry into stack */

    rets->link = retstk;
    retstk = rets;
    stack_level++;

    rets->p = int_form;		/* save pointer to internal form */

    rets->intsp_save = intsp;	/* save interpreter stack position 
	 			   around catch & throw */
    rets->currtab = currtab;	/* remember dynamic stack position */
    catch (&rets->retenv,
	    fn,
	    int_form);

    intsp = rets->intsp_save;
    retstk = rets->link;

	/* get rid of local symbols */

    --stack_level;
}

long popretstk(where)
    retsp where;
{
    register retsp rets;

    for (rets = retstk; rets && rets != where; rets = rets->link)
	poplevel(rets->currtab, --stack_level);
    if (rets != where) system_error("Invalid position for popretstk.");
    retstk = rets;
}


	/* This function unwinds the return stack to a given position
	   by first freeing symps in the unwound scopes and then doing
	   a throw to get the interpreter back to the right place.
	   The type argument is as follows:

		1	return
		2	exception
		3	exit or goto
	*/


long unwind_retstk(where, type)
    retsp where;
    long type;
{
    popretstk(where);
    throw(retstk->retenv, type);	
}
