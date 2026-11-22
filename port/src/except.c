static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/except.v  Revision 1.3  83/09/08  15:40:35  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/except.v $
 * Revision 1.3  83/09/08  15:40:35  ada
 * same as 1.2.9.1
 * 
 * Revision 1.2.9.1  83/08/25  14:25:17  ritter
 * fixed handling of exceptions raised while already inside of a handler
 * 
 * Revision 1.2  83/01/12  09:14:25  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:24:04  tadman
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

nodep handler;		/* global for enterargs, etc. */

static retsp ret;

  /* find_handler runs down the interpreter return stack looking for a
     handler for the given exception.  The argument is a symp which should
     pointer to an entry for an exception handler.  A pointer to the
     first matching handler is return, or null if no handler matches.

     A "matching hander" is either one with a name which evaluates to
     the same symbol table entry or an OTHERS hander.  However, it is
     considered bad Ada manners to invoke an exception handler in a
     scope which is elaborating its declarations, or which is already
     executing an exception handler, so such things are scrupulously
     avoided.  */

       nodep find_handler(exception)
    fixp exception;
{
    register nodep h, l,n ;
    typedval result;
    symtabp save_currtab;
    
    ret = retstk;
    while (ret) {
	enum tokentypes op = ret -> p -> ndf.op;

	/* The following test says: 1)  if this exception happened
	   during the elaboration of declarations, don't look for a handler
	   on this level, but propigate the exception further.  2) if
	   this exception happened during the execution of an exception
	   handler, also propigate further.  3) if this entry if for
	   a subprogram, block or macro (in other words, not a loop) then
	   look here for a handler */

	/* if ( ret->exception == null && */
	if ( !(ret->must_prop_exception) &&         /* PAR 22-Aug-83 */

	    (op == nop_function || op == nop_procedure || 
		op == nop_declare || op == nop_body ||
		op == nop_macro)) {
	    switch (op) {
		case nop_macro:
		    h = ret->p->nd.macrox.except;	/* Castor */
		case nop_body:
		    h = ret->p->nd.packagex.except;
		case nop_function:
		case nop_procedure:
		case nop_declare:
		    h = ret->p->nd.subp.except;	/* point to handler list */
	    }

	    while (h) {
		l = h->nd.whenx.choice_list;	/* point to exception names */
		while (l) {
		    n = l->nd.choice.choice;
		    if (n == null) 
		        return h->nd.whenx.stmt;	/* OTHERS handler */

		    save_currtab = currtab;	/* but what about ^C? */
		    currtab = ret->currtab;
		    eval(n, decl_mode, &result, null);
		    currtab = save_currtab;
		    if (result.typ != excepttype)
			error("Handler name is not an exception");
		    if (result.val.fptr == exception)
		        return h->nd.whenx.stmt;
	    	    l = l->ndf.next;
		}
		h = h->ndf.next;
	    }
	}
	ret = ret -> link;
    }
    return null;
}


 /* This is the code for the raise statement.  It attempts to find a
    handler for the raised exception.  If it can't then it generates an
    error at the point where the exception was raised. */

raise(exception_name)
    nodep exception_name;
{
    typedval result;

    if (exception_name == null) {
	if (retstk && retstk->exception != null) {
	    handler = find_handler(retstk->exception);
            if (handler == null)
	        error ("No handler for exception");
	    ret->exception = retstk->exception;
	    unwind_retstk(ret,2);
	}
        else error("Raise without exception name not in a handler");
    }
    eval(exception_name, eval_mode, &result, null);
    if (result.typ != excepttype)
	error("Raise does not name an exception");
    handler = find_handler(result.val.fptr);
    if (handler == null)
	error("No handler for exception");
    ret->exception = result.val.fptr;
    unwind_retstk(ret,2);	/* 2 means exception */
}

  /* This function is used to raise a predefined exception.  It takes the
     same arguments as error() except for a new first argument which is
     the exception to be raised.  It either silently raises the requested
     exception if there is a handler for the exception, or it calls error
     with the remaining arguments if there is none.  This causes a break
     a the point where the exception was raised. */

/*VARARGS2*/		/* for lint */
sysraise(exception, error_msg, arg1, arg2, arg3, arg4, arg5)
    fixp exception;
    char *error_msg;
{
    handler = find_handler(exception);
    if (handler == null)
	error(error_msg, arg1, arg2, arg3, arg4, arg5);
    ret->exception = exception;
    unwind_retstk(ret,2);
}



