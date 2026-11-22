static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/symtab.v  Revision 1.8  83/11/30  14:34:43  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/symtab.v $
 * Revision 1.8  83/11/30  14:34:43  ada
 * same as 1.7.5.1
 * 
 * Revision 1.7.5.1  83/11/29  16:16:43  snider
 * Modified some printf statements in printfix for the symbol_table command.
 * 
 * Revision 1.7  83/05/09  14:13:33  ada
 * same as 1.4.40.2
 * 
 * Revision 1.4.40.2  83/04/14  08:30:38  tadman
 * Static_link doesn't print usr and standard, unless its argument is
 * usr'sym.  The symbol_table report no longer segmentation faults if its
 * argument is a package without a body or a specification.
 * Poplevel calls itself recursively for all packages within its argument
 * table.  This eliminates "symbol redefined" errors for local packages
 * the second time the scope containing it is invoked.
 * 
 * Revision 1.4.40.1  83/04/12  12:10:11  tadman
 * Corrected problem with symbol table reports (psyms, static_link,
 * and symbol_table) which sometimes caused a segmentation fault if
 * a static link was null.
 * 
 * Revision 1.4  83/01/12  09:09:32  ada
 * Added copyright notice.
 * 
 * Revision 1.3  83/01/04  14:25:32  willson
 * Fixed a free(fix) => freefixp(fix).
 * 
 * Revision 1.2  82/12/30  14:02:07  ada
 * Same as 1.1.1.1
 * 
 * Revision 1.1.1.1  82/12/20  15:22:31  willson
 * Change use_sym to not report when a package has already been used.  With
 * static tables, this was often erroneous, and anyway, the Ada LRM doesn't
 * say this is an error.  (In fact, it says it is o.k.)
 * 
 * Revision 1.1  82/12/12  21:45:24  tadman
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

/*	This file contains stuff to manipulate symbol tables and symbols.
	The contents of this file used to be in tab.c and a part of sym.c.
*/

extern int stack_level;		/* level of interpreter return stack */

/*	sympstack is a stack of symbolforms.  See below. */

symp	sympstack _ null;
fixp	fixpstack _ null;

/* 	Allocate space for a symbolform.  Malloc's it from the heap.
	(Anything can be made into a verb.)
	Optimization: Tests show that we're spending a ton of time in
	malloc.  So, poplevel() pushes popped symbolforms onto a list.
	We check this list first and return a symbolform from there to
	avoid calling malloc (which likes to search all of memory before
	getting more space from the Kernel).  To make this work, all
	symbolforms are allocated the same size (the maximum) regardless
	of the size requested.
*/

fixp newfix1(size, class)
    register int size;
    enum symbolclasses class;
{
    register fixp fix;
    register char *p;

    size _ sizeof(struct fixform);
    if (fixpstack) {
	fix = fixpstack;
	fixpstack _ fixpstack -> fxf.next;
	for (p _ (char *)fix; size--; *p++ _ 0);
    /* zero out fixform the way zalloc would */
    }
    else
	fix = (fixp)zalloc (sizeof (struct fixform), true);

    fix -> fxf.class = class;
    return fix;
}

symp newsym1(size, class)
    register int size;
    enum symbolclasses class;
{
    register symp sym;

/* Old code:
        sym = (symp)zalloc(size, true);
*/
    if (sympstack) {
	register char  *p;
	register int size _ sizeof (struct symbolform);

	sym _ sympstack;
	sympstack _ sympstack -> syf.link;
	for (p _ (char *) sym; size--; *p++ _ '\0');
    }
    else
	sym = (symp) zalloc (sizeof (struct symbolform), true);
    if (class == obj_id || class == param_id)
	sym -> sy.var.value = &sym -> sy.var.value_space;
 /* pre-initialized space for scalars */
    return (sym);
}


/*   	Print all the symbolforms for a symbol.  This is called by
	printfix after all of the fixed part is printed. */

printsym(s,class)
    symp s;
    enum symbolclasses class;
{
    putf("     level: %4d ", s->syf.level);
    switch (class) {
	case param_id: 
	case obj_id: 
	    putf ("*var.value: %d", *(s -> sy.var.value));
	    putf (" type: ");
	    dopp (s -> sy.var.type -> tyf.int_form,
		    current_output (), false, true);
	    break;
	case type_id:
	case subtype_id:
	    putf (" type: ");
	    dopp (s -> sy.var.type -> tyf.int_form,
		    current_output (), false, true);
	    break;
	case func_id:
	case proc_id:
	case macro_id:
	    putf (" type: ");
	    dopp (s -> sy.var.type -> tyf.int_form,
		    current_output (), false, true);
	    break;
	case pack_id:
	    break;
    }
    putf("\n");
}

/*	Print the contents of a symbol; this is a formatted debugging aid. */

printfix(f)
    register fixp f;
{
    register symp s;

    switch (f -> fxf.class) {
	case undef_id: 
	    putf ("undefid");
	    break;
	case obj_id: 
	    putf ("obj_id");
	    break;
	case except_id: 
	    putf ("except_id");
	    break;
	case enum_id: 
	    putf ("enum_id");
	    break;
	case param_id: 
	    putf ("param_id");
	    break;
	case func_id: 
	    putf ("func_id");
	    break;
	case proc_id: 
	    putf ("proc_id");
	    break;
	case type_id: 
	    putf ("type_id");
	    break;
	case subtype_id: 
	    putf ("subtype_id");
	    break;
	case field_id: 
	    putf ("field_id");
	    break;
	case pack_id: 
	    putf ("pack_id");
	    break;
	case macro_id: 
	    putf ("macro_id");
	    break;
	default: 
	    putf ("unknown %d in syf.class", f -> fxf.class);
	    break;
    }				/* end switch */
    putf (" %s %x  static_link %x ", f -> fxf.name, f, f->fxf.static_link);
    switch (f -> fxf.class) {
	case param_id: 
	case obj_id: 
	    if (f->fx.var.is_const) putf("constant ");
	    if (f->fx.var.is_enum_lit) putf("enum ");
	    if (f->fx.var.param_is_in) putf("in ");
	    if (f->fx.var.param_is_out) putf("out ");
	    if (f->fx.var.param_has_value) putf("has_value");
	    break;
	case func_id:
	case proc_id:
	case macro_id:
	    if (f -> fx.func.int_form != null)
		 putf("int_form: %x ", f -> fx.func.int_form);
	    if (f -> fx.func.code != null)
		 putf("code: %x", f -> fx.func.code);
	    break;
	case pack_id:
	    if (f -> fx.pack.int_form != null)
		 putf("spec: %x", f -> fx.pack.int_form);
	    if (f -> fx.pack.body_int_form != null)
		 putf("body: %x", f->fx.pack.body_int_form);
	    break;
    }
    putf ("\n");

	/* now print the symps */

    for (s = f -> fxf.dyna; s; s = s->syf.link)
	printsym(s, f->fxf.class);
};

/*	print all the symbols in a table (formatted debugging aid) */

printtab(tab)
   symtabp tab;
{
    register fixp p;
    register int  i, n;
    register usep up;

 /* list some of the fixed fields of the table */

   if (tab -> symtab_owner)
	putf ("\n%s: %x  static_link  %s (%x)\n", 
	  	tab -> symtab_owner ?
		    tab -> symtab_owner -> fxf.name :
			"<block or loop>",
		tab,
		tab->symtab_static_link ?
		    tab->symtab_static_link->symtab_owner ?
			tab->symtab_static_link->symtab_owner->fxf.name :
			"<block or loop>" :
		    "<null>",
	        tab->symtab_static_link);

 /* print the contents of this table */

    putf ("<<<\n");
    n = tab -> symtab_size;
    for (i = 0; i < n; i++)
	for (p = tab -> symtab_tab[i]; p; p = p -> fxf.next)
	    if (p) printfix(p);
 /* now print any tables this one has on its uses list */
    up _ tab -> use_list;
    if (up)
	putf ("---> Use list:\n");
    while (up neq null) {
	if (up -> table -> symtab_owner)
	    putf ("-------> uses %s:\n",
		    up -> table -> symtab_owner -> fxf.name);
	up _ up -> link;
    }
    putf (">>>\n");
}

/*	print all symbol tables by tracing the return stack (formatted
	debugging aid).  Also prints what tables are "used" by the
	table being printed.
*/

printall() 
{
    register retsp ret;
    register symtabp tab;

    for (ret = retstk; ret; ret = ret -> link) {
	tab = ret->currtab;
	printtab (tab);
	putf ("\n");
    }
}

 /* pPints all tables on the static link starting with tab.  If *
     the argument table is usr'sym, then the tables for usr and standard
     are printed, otherwise, printing stops with the table before usrtab
     since almost every static link ends in standard.usr, this saves a
     lot of printing.	*/


printstatic(tab)
    symtabp tab;
{
    while (tab != null) {
	printtab(tab);
	tab = tab->symtab_static_link;
	if (tab == usrtab) break;
    }
}

 /* print the symbol table corresponding to the given symbol (fixp) */

symbol_table(fix)
    fixp fix;
{
    switch (fix->fxf.class) {
	case func_id:
	case proc_id:
	case macro_id:
	    printtab(fix->fx.func.int_form->nd.subp.table);    
	    break;
	case pack_id:
	    if (fix->fx.pack.int_form) {
	        putf("Visible:\n");
	    	printtab(fix->fx.pack.int_form->nd.packagex.table);
	    }
	    if (fix -> fx.pack.body_int_form) {
	    	putf("\nBody:\n");
	    	printtab(fix->fx.pack.body_int_form->nd.packagex.table);
	    }
	    break;
	default:
	    putf("That thing doesn't have a scope!\n");
	    break;
    }
}

 /* print the symbol table corresponding to the given symbol and
    the tables of all enclosing scopes by following the static link */

static_link(fix)
    fixp fix;
{
    switch (fix->fxf.class) {
	case func_id:
	case proc_id:
	case macro_id:
	    printstatic(fix->fx.func.int_form->nd.subp.table);    
	    break;
	case pack_id:
	    if (fix->fx.pack.body_int_form &&
		fix->fx.pack.body_int_form->nd.packagex.table)
	    	printstatic(fix->fx.pack.body_int_form->nd.packagex.table);
	    else
	        printstatic(fix->fx.pack.int_form->nd.packagex.table);
	    break;
	default:
	    putf("That thing doesn't have a scope!\n");
	    break;
    }
}

/*	Create a new symbol table out of the void.  */

int	symtabform_count;
int	hash_table_count;

symtabp newtab(n, owner)  
    int n;               /* n should be power of 2     */
    fixp owner;          /* and should not exceed 256  */
{
    register    symtabp p;	/* and should not exceed 256 */

    symtabform_count += sizeof (struct symtabform);
    hash_table_count += n;
    p = (symtabp) zalloc (sizeof (struct symtabform), true);
    p -> symtab_tab = (fixp *) zalloc (n * sizeof (int), true);
    p -> symtab_size = n;
    p -> symtab_owner = owner;
    return (p);
}



/*	Free all the elements in a table; this should only be called by
	the guy who frees internal forms, since symbol tables are now
	effectively a part of the internal form for a subp or package. */

freetab(tab) 
symtabp tab;
{
        int     i,size;

        size = tab->symtab_size;
        for(i = 0; i<size; i++)  
                freelist(tab->symtab_tab[i]);
        free(tab->symtab_tab);
        free(tab);             
}

freefixp(p)
fixp p;
{
    p -> fxf.next _ fixpstack;	/* put fixp into recovery stack */
    fixpstack _ p;
}

freelist(p)		/* free list pointed at by fixp p and then free p */
fixp p;
{	if (p) {
		freelist2(p->fxf.dyna);
		freefixp(p);
	}
}

       freelist2(p)
symp p;
{
        if(p) {
                freelist2(p->syf.link);
                freesym(p);        
        }                       
}

/*	freesym doesn't do a "free"; instead, it pushes the symbol on
	the free list of symbolforms for later reuse.  */

freesym(sym) 
symp sym;
{                  /* later: free space allocated for       */
				/*   array or record objects             */
				/* much later: free type structures, too */
	sym->syf.link _ sympstack;
	sympstack _ sym;
}                               


 /*     Enter a symbol into a symbol table.  If there is no fixp for the
	symbol, use the supplied one, otherwise simply push the symbol
	on the front of the dynamic list of symp hanging off the fixp
	and free the supplied fixp.

 	Note: this is a temporary hack until the static symbol tables are
 	fully imlemented.  Ideally, the fixps would be placed in the symbol
 	table before execution begins.
 */

enter(tab, fix, sym)
    symtabp tab;
    fixp fix;
    symp sym;
{
    register int h;
    register fixp p;

    p _ lookuptab (tab, fix -> fxf.name, null);
    if (p == null) {
	h = hashval (fix -> fxf.name);
	h &= tab -> symtab_size - 1;
	fix -> fxf.next _ tab -> symtab_tab[h]; /* push in bucket */
	tab -> symtab_tab[h] = fix;
	p = fix;
	fix->fxf.dyna = null;
	fix->fxf.static_link = tab;
    }
    else {
	freefixp(fix);
	if (p -> fxf.dyna) {
	    if (p -> fxf.dyna -> syf.level == stack_level)
		error ("Attempt to redefine %s", fix -> fxf.name);
	    if (p -> fxf.dyna -> syf.level > stack_level)
		system_error("Symbolform %s not released.", fix -> fxf.name);
 	}
    }

    sym -> syf.link _ p -> fxf.dyna; /* push symbol onto dynamic stack */
    p -> fxf.dyna _ sym;
    sym -> syf.level = stack_level;

 /* incrementally update completion structures with top level definitions 
 */
    if (tab == usrtab)
	add_sym_for_completion (p);
}
		

/*	lookup the fixed part of a symbol; this must be made more complicated
	later on to do overload resolution on a lookup */

/*VARARGS3*/		/* for lint */
fixp lookuptab(tab, id, fun, a)
    symtabp tab;
    char *id;
    boolean (*fun)();
    int a;
{
    register int h = hashval (id);
    register    fixp f;

    f = tab -> symtab_tab[h & (tab -> symtab_size - 1)];
    while (f) {
	if (id eq f -> fxf.name)
	    if ((not fun) or (*fun) (f, a))
		return f;
	f _ f -> fxf.next;
    }
    return null;
}

/*	Lookup a symbol.  Look first in tab, and if not found there,
	check if it's in a table that is used by tab.  If still
	not found, then repeat by chasing the static link. 
*/


/*VARARGS3*/		/* for lint */
fixp lookup(tab, id, f, a)
    symtabp tab;
    char *id;       /* symbol identifier */
    boolean (*f)();
    int a;
{
    register int h = hashval (id);
    register fixp fix;
    register usep up;

    while (tab != null) {
	if (fix = lookuptab(tab, id, f, a))
	    return fix;
	up = tab -> use_list;
	while (up != null) {
	    if (fix = lookuptab(up->table, id, f, a))
		return fix;
	    up = up->link;
	}
	tab = tab -> symtab_static_link;
    }
    return null;
}

 /* this procedure places the table for the symbol f on the use list
    of table tab  */

use_sym(f, tab)
    fixp f;
    symtabp tab;
{
    usep p, *pt;

    if (f -> fxf.class != pack_id)
	error ("can only 'use' a package");

    pt = &(tab -> use_list);
 /* whew.  not much to be done about this... */
    while (*pt != null) {
	if ((*pt)->table == f->fx.pack.int_form->nd.packagex.table) {
	    /* putf ("package %s already used\n", f -> fxf.name); */
				/* no need to say this (shouldn't, in fact) */
	    return;
	}
	pt = &((*pt) -> link);
    }
    p = (usep) zalloc (sizeof (struct useform) , true);
    p -> table = f -> fx.pack.int_form->nd.packagex.table;
    (*pt) = p;			/* backpatch last pointer in list to point
				   to this new entry */
}

/*	Delete the given symbol from the given table.
	Used only to explicitly delete symbols via del(x'sym); from Ada. 
*/

deltab(fix)
fixp fix;
{
	/* short term hack: set to undef_id */
	fix -> fxf.class = undef_id;
}


/*      Pop back to a  dynamic execution level; this means finding all
	the fixp's
	in the symbol table given and removing all symps with a level
	number greater than or equal to level from each fixp's stack of
	dynamic symp's.  There might not be a symp for each fixp:
	symp's are added as a recursive instances of subprograms are
	invoked.  But a declaration's initialization might invoke
	another subprogram, which might recursively call the first one,
	which would then have the wrong number of symp's on each fixp.
	Another problem is that an exception could be raised in the
	middle of a declaration, which "blows you out of the middle" of
	things.  So, when popping a symp, we check if its back pointer
	to the table it was defined in equals currtab:  if it does,
	then we should remove the symp, otherwise, it should be left as
	it is still active in some other invocation of the subprogram.

	See the comment for newsym1 about the optimization done here in
	cooperation with newsym1.  */

poplevel(tab, level)
    symtabp tab;
    int level;
{
    register int size = tab -> symtab_size;
    register fixp p;
    register int i;

    for (i _ 0; i < size; i++)
	for (p _ tab -> symtab_tab[i]; p; p _ p -> fxf.next) {
		/* If this scope contains a local package, then
		   the symbolforms must be removed from both the
		   package specification and the package body. */

	    if (p -> fxf.class == pack_id) {
		if (p -> fx.pack.int_form && 
		    p -> fx.pack.int_form -> nd.packagex.table)
		    poplevel(p->fx.pack.int_form->nd.packagex.table, level);
		if (p -> fx.pack.body_int_form && 
		    p -> fx.pack.body_int_form -> nd.packagex.table)
		    poplevel(p->fx.pack.body_int_form->nd.packagex.table, 
			     level);
	    }

		/* Now remove the symbolforms from the current symbol,
		   if necessary. */

	    if (p -> fxf.dyna && p -> fxf.dyna -> syf.level >= level) {
		symp next _ p -> fxf.dyna -> syf.link;
		freesym (p -> fxf.dyna);
		p -> fxf.dyna _ next;
	    }
	}
}
