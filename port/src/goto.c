static char rcsid[] =
"$Header:   ../RCS/goto.v  Revision 1.2  83/01/12  09:15:14  ada  Exp$";

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


/* $Log:	../RCS/goto.v $
 * Revision 1.2  83/01/12  09:15:14  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:26:35  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"
/* port: block-scope externs, repeated at file scope */
extern nill();

     /*	This file implements the goto statement.  There are two cases
	that need to be handled.  This first is when the goto statement
	and its target label are in the same statement_list.  This is
	easy.  The interpreter pointer (p) in eval() is simply changed
	to point to the label.  (Note: a label in the internal form
	is just a node in the list of statements for the statement_list.
	A label simply "executes" as no_op.)  

	The second case is when	the label is in a statement_list which 
	also contains a	compound statement (loop, block, case, or if) 
	which contains	(at some level of nesting) the goto.  In this 
	case, the goto has the effect of doing an exit to the compound
	statement, followed by changing the interpreter pointer.  The
	exit statement causes the interpreter stack, any symbol
	tables for blocks and for loops, and the recursive calls to
	eval() to be popped.  (Note: yes, I know you can't exit a
	block, case statement, or if statement with an exit statement,
	but you get the idea.)

	In ada, it is not allowed to jump into any compound statement
	from outside, so there is no third case.  This is very fortunate
	because if you could, you wouldn't be able to have goto
	statements for a long time.	*/

extern  typep tyeval ();

extern nodep intstk[], *intsp, *intstk_end;

extern int curr_generation;

nodep goto_p;			/* used by the interpreter to do the goto */

static nodep label_p;
static char *label;

    /*	find_subp() finds the top entry for a subprogram or macro on the
	return stack and returns a pointer to it.  This bounds the search
	for the target label, since you can't jump out of these
	constructs.  */

retsp find_subp()
{
    retsp ret = retstk;

    while (ret != null) {
	enum tokentypes op = ret -> p -> ndf.op;

	if (op == nop_function || op == nop_procedure || op == nop_macro)
	    break;
	ret = ret -> link;
    }
    if (ret == null)
	error ("Goto not allowed at the top level");
    return ret;
}

    /*  maptree applies this function to every node in the current
	subprogram.  It checks to see if it is the target label and
	if it is, it saves a pointer to it.  If it finds more than one
	it complains. */

static fix_pre(n)
    nodep n;
{
    if (n->ndf.op == nop_lb && n->nd.ident.id == label) {
	if (label_p)  error("Doubly defined label");
	label_p = n;
    }
}


     /*	find_connection finds the node in the same statement_list as
	the label which is either the goto being executed or the
	compound statement containing the goto. (see the comment at
	the top of this file.)  It does this by searching the return
	stack from the top down to the level of the enclosing subprogram
	or macro.  */

       nodep find_connection(subp_ret, goto_node)
    retsp subp_ret;
    nodep goto_node;
{
    register    nodep p;
    register    retsp ret = retstk;

    if (label_p == null)
	error ("Undefined label in GOTO");

	/* see if the label is after the goto in the same statement_list */

    p = goto_node;
    while (p && p != label_p)
	p = p -> ndf.next;
    if (p)
	return null;

	/* see if the label is before the goto in the same statement_list */
    p = label_p;
    while (p && p != goto_node)
	p = p -> ndf.next;
    if (p)
	return null;

	/* check the stack of currently active compound statements */

    while (ret && ret != subp_ret) {

		/* see if the compound_statement on the same level
		   as the label is before the label. */
	p = ret -> p;
	while (p && p != label_p)
	    p = p -> ndf.next;
	if (p)
	    return ret -> p;

		/* see if the compound_statement on the same level
		   as the label is after the label. */

	p = label_p;
	while (p && p != ret -> p)
	    p = p -> ndf.next;
	if (p)
	    return ret -> p;

	ret = ret->link;
    }
		/* if none of the above, then the label must be
		   nested in something which is not active. */
    error ("Goto into a block, a loop, an if statement, or a case statement");
}

     /* fixgoto sets two things in the goto node for the currently
	executing goto.  nd.gotox.dest is a pointer to the label which
	is the target of the goto.  nd.gotox.throw_dest is the
	address of the compound statement containing the goto in the
	same list as the label, or null if the label and goto are in
	the same list. */

       fixgoto(goto_node)
    nodep goto_node;
{
    nodep p;
    retsp ret;
    extern nill();

    label_p = null;

    if (goto_node->nd.gotox.label->ndf.op != nop_ident)
	error("Goto statement must have an identifier label");
    label = goto_node->nd.gotox.label->nd.ident.id;
    ret = find_subp();
    p = ret->p;
    maptree(p, fix_pre, nill, nill, nill);
    goto_node->nd.gotox.dest = label_p;
    goto_node->nd.gotox.throw_dest = find_connection(ret, goto_node);
}

     /* dogoto executes the actual goto.  We use the same generation
	numbering scheme as we do for lookups of symbols.  The information
	for the goto is calculated once and stored in the goto_node.
	From then on, the goto need not recalculate the information
	until curr_generation changes.  curr_generation changes whenever
	anything happens which could change the destination of the
	goto (such as editing the procedure which contains it). */

nodep dogoto(goto_node)
    register nodep goto_node;
{
    register retsp ret;

    if (goto_node->nd.gotox.gen_number != curr_generation) {
	goto_node->nd.gotox.gen_number = curr_generation;
	fixgoto(goto_node);
    }
    if (goto_node->nd.gotox.throw_dest == null)
		/* case 1:  return pointer to label */
	return goto_node->nd.gotox.dest;
    else {
		/* case 2: put pointer to label in goto_p and then
		   do an "exit".  eval will set its pointer to goto_p
		   after the exit is caught. */
 	goto_p = goto_node->nd.gotox.dest;
        ret = retstk;
        while (ret && ret->p != goto_node->nd.gotox.throw_dest)
	    ret = ret->link;
        if (ret == null)  error("Internal error in GOTO");
        unwind_retstk(ret,3);
    }
}

    
