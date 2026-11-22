static char rcsid[] =
"$Header:   ../RCS/typeonly.v  Revision 1.2  83/01/12  09:11:10  ada  Exp$";

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


/* $Log:	../RCS/typeonly.v $
 * Revision 1.2  83/01/12  09:11:10  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:49:01  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"

extern nodep intstk[], *intsp, *intstk_end;	/* interpreter stack */

/* this function evaluates an expression in "type only" mode */
/* It attempts to deduce the type of an expression without   */
/* evaluating any expressions which might have side effects  */
/* Its primary purpose is to analyze the arguments to a call */
/* in order to do overloading resolution.  Since it has no   */
/* side effects, it can also be used in "static" tools such  */
/* as code generators and cross referencers.		     */

typep tyeval();

typep typeonly(p)
    nodep p;
{

    typep ret;
		/* stack a pointer to the current node on the interpreter
		   stack.  This is so that if the expression is a macro
		   call, then the macro expansion functions can find the
		   parent of the current node.  It also is used by
		   backtrace to locate the point at which an error 
		   occurred. */
    	if (intsp++ == intstk_end) {/* reserve space on stack */
            putf ("\nInterpreter stack overflow\n");
            intsp = intstk;
    	}
        *intsp = p;                 /* save pointer on stack */

	switch(p -> ndf.op) {
	    case nop_in:
	    case nop_equal:
	    case nop_nequal:
	    case nop_lt:
	    case nop_le:
	    case nop_gt:
	    case nop_ge:
	    case nop_andthen:
	    case nop_orelse:
	    case nop_not:
	        ret = booltype;
		break;
	    case nop_int:
		ret = inttype;
		break;
	    case nop_char:
		ret = chartype;	/* ignore char lits in enums */
		break;
	    case nop_real:
		ret = realtype;
		break;
	    case nop_string:
		ret = strtype;	/* don't compute constraint yet */
		break;
	    case nop_ident: {
		fixp f;
		/* should do static analysis of id's */
		/* ignore overloaded subprograms for now */
		f = lookup(currtab, p -> nd.ident.id, null);
		if(f == null)
		    ret = null;
	        else ret = f->fxf.dyna->sy.var.type;
		break;
	    }
	    case nop_null:
		ret = nulltype;	/* type of null is undeftype */
		break;
	    case nop_new:
	        ret = nulltype;	/* same for allocators */
		break;		/* should this be undeftype? */
	    case nop_lp:
		ret = typeonly(p -> nd.uop.child);
		break;
	    case nop_plus:
	    case nop_minus:
	    case nop_star:
	    case nop_div:
	    case nop_mod:
	    	/* assumes both operands are of same type */
		/* if not, an error is generated later	  */
	        ret = typeonly(p -> nd.binop.left);
		break;
	    case nop_amper:
		/* doesn't work for 'a' & "bcd"	*/
		ret = typeonly(p -> nd.binop.left);
		break;
	    case nop_neg:
	    case nop_pos:
		ret = typeonly(p -> nd.uop.child);
		break;
	    case nop_call: {

		/* same as nop_dot below */

		typedval call;

		docall(p, ovl_mode, &call, null);

		ret _ call.typ;
		break;
		}
	    case nop_dot: {
		/* eval the dotted name in ovl_mode, thus avoiding
		side-effects. */

		typedval dot;

		dodot(p, ovl_mode, &dot);

		ret _ dot.typ;
		break;
	    }
	    case nop_quote: {
		/* same comment as nop_dot		*/

		typedval quote;

		doattr(p, &quote);

		ret _ quote.typ;
		break;
	    }
	    case nop_qual_expr:
		/* should lookup type staticly	*/
		ret = tyeval (p -> nd.aggregate.qualifier, null);
		break;
	    case nop_aggregate: 
		/* same here */
		if(p -> nd.aggregate.qualifier)
		    ret = tyeval (p -> nd.aggregate.qualifier, null);
		else ret = null;
		break;

		/* Castor */

	    case nop_name_callf:
	    case nop_expr_callf: {
	        boolean defined;
		typedval result;

	        defined = macro_call(p,macro_mode,&result);
		if (defined) {
		    dyn_macro_expand(result.val.nptr, p);
		    p = result.val.nptr;
		    ret = typeonly(p);
		}
		else error("macro {%s} is not defined",	macro_pname(p));
	    }
		break;

	    default:
	   	ret = null;
		break;
	}

	intsp--;		/* pop the interpreter stack */
	return ret;
}
