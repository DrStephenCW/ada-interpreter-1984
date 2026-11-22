static char rcsid[] =
"$Header:   /usr/ics/darpa/ada/RCS/stack.v  Revision 1.4  83/03/10  16:09:10  ada  Exp$";

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
        int (*func)();
        elem *link;
};

elem *stack;		/* list of exception handlers to invoke on */
			/* control-c interrupt */
pushfunc(f)
int (*f)();
{
        register elem *p;
        p = (elem *)zalloc(sizeof(elem), false);
        p->func = f;
        p->link = stack;
        stack = p;
}

popfunc() {
        register elem *p;
        p = stack;
        stack = stack->link;
        free(p);
}

clean_up_using_control_c_stack() {
	control_c_was_typed _ false;
        while(stack) {
                (*stack->func)();
		popfunc();
        }
}
