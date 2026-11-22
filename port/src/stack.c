static char rcsid[] =
"$Header:   /usr/ics/darpa/ada/RCS/stack.v  Revision 1.4  83/03/10  16:09:10  ada  Exp$";

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


/* $Log:	/usr/ics/darpa/ada/RCS/stack.v $
 * Revision 1.4  83/03/10  16:09:10  ada
 * same as 1.3.2.1
 * 
 * Revision 1.3.2.1  83/03/10  14:58:48  willson
 * *** empty log message ***
 * 
 * Revision 1.2  83/01/12  09:09:22  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:44:21  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include "macros.h"
#include "handler.h"

/*	Package of routines to make a stack of exception handlers for 
	taking care of control-c interrupts.  For instance, the parser
	might push a routine to close its open files.  Pushfunc and popfunc
	are used to push the exception handler, and popfunc is used to 
	get rid of it when everything is clean.  Note that any exception
	handler you write can assume almost NOTHING about the state of
	execution, since the control-c could happen almost anytime.
*/

typedef struct stack_element elem;

struct stack_element {
        long (*func)();
        elem *link;
};

elem *stack;		/* list of exception handlers to invoke on */
			/* control-c interrupt */
long pushfunc(f)
long (*f)();
{
        register elem *p;
        p = (elem *)zalloc(sizeof(elem), false);
        p->func = f;
        p->link = stack;
        stack = p;
}

long popfunc() {
        register elem *p;
        p = stack;
        stack = stack->link;
        free(p);
}

long clean_up_using_control_c_stack() {
	control_c_was_typed _ false;
        while(stack) {
                (*stack->func)();
		popfunc();
        }
}
