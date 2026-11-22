static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/misc.v  Revision 1.3  83/02/04  14:50:16  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/misc.v $
 * Revision 1.3  83/02/04  14:50:16  ada
 * same as 1.2.4.1
 * 
 * Revision 1.2  83/01/12  09:18:02  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:34:58  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "oblist.h"
#include "eval.h"
#include "case.h"


char *malloc();

static int memcount;
static int memsize;
static int newnode_count;
extern int typecount;
extern int symtabform_count;
extern int hash_table_count;
extern fixp fixpstack;
extern symp sympstack;
extern struct choice_item *case_choice_stack;
extern end();

/*	Print total memory expansion only.  */

quick_memreport() {
	putf("[%dk bytes of memory allocated for this session]\n",
		((sbrk(0) - (int)end) / 1024));
}

/*	Print memory statistics.  Easy way to make yourself be depressed. */

memreport() {
        int count;
        symp sym;
        struct choice_item  *case_choice_ptr;
	fixp fix;

        putf("%d malloc's for %d bytes\n", memcount, memsize);
        putf("Total expansion due to malloc's: %d\n", sbrk(0) - (int)end);

        for (count _ 0, sym _ sympstack;
                sym;
                count++, sym _ sym->syf.link) ;
        putf("%d symbolforms (%d bytes) in sympstack\n", 
                count,
                count * sizeof(struct symbolform));

        for (count _ 0, case_choice_ptr _ case_choice_stack;
                case_choice_ptr;
                count++, case_choice_ptr _ case_choice_ptr->next_choice_item);
        putf("%d case stmt choice items (%d bytes) in case_choice_stack\n",
		count,
                count * sizeof(struct choice_item));

	for (count _ 0, fix _ fixpstack;
		fix;
		count++, fix _ fix->fxf.next) ;
	putf("%d fixforms (%d bytes) in fixpstack; ", 
                count,
		count * sizeof(struct fixform));

        putf("%d bytes for types\n", typecount);

	putf("%d bytes for symtabforms; ", symtabform_count);
        putf("%d bytes for hash tables\n", hash_table_count);

	putf("%d bytes for internal form nodes\n", newnode_count);
}


getval(s)        /* lookup a value in the interpreter */
char *s;                /* environment; return zero if not defined */
{
	fixp fix;

        s _ intern(s);  /* change into useful pointer */
        fix _ lookup(currtab, s, null); /* find in interpreter */
        if (not fix)  return null;       /* not found eq false */
        if (fix->fxf.class neq obj_id)  {
                sysputf("Attempt to getval symbol %s that's not an object",
                        s);
                return null;
        };
        return *fix->fxf.dyna->sy.var.value;
};

/* calls the function f on each fixform in the table tab.  A pointer to
    the fixform is the first argument to f.  The arg1 and arg2 arguments
    to maptab are passed on to f as optional additional arguments. */

maptab(tab, f, arg1, arg2)
    symtabp tab;
    int (*f)();
{
    register fixp p;
    register int i,n;

    n = tab -> symtab_size;
    for (i = 0; i < n; i++)
	for (p = tab -> symtab_tab[i]; p; p = p -> fxf.next)
	    (*f) (p, arg1, arg2);
}
         
char *zalloc(size, zero) {
register char *p,*q;
        memcount++;
        memsize += size;
        p = malloc(size);
        if (p == 0) {
            system_error("Storage allocation error  req %d  alloc %d  tot %d",
                size, memcount, memsize);
        }
        if(zero)
                for(q = p; size--; *q++ = 0) ;
        return(p);
}

nodep newnode1(size, op)
int size;
enum tokentypes op;
{
        register nodep p;
        p = (nodep)zalloc(size, true);
        p->ndf.op = op;
        p->ndf.ag = null;
	newnode_count += size;
        return(p);
}

memq(tok, list)
enum tokentypes tok;
tokenlist list;
{
        enum tokentypes *p;
        for(p = list; *p != nop_err; p++)
                if(*p == tok)
                        return true;
        return false;
}


