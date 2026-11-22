static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/dir.v  Revision 1.6  84/03/16  17:09:43  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/dir.v $
 * Revision 1.6  84/03/16  17:09:43  ada
 * same as 1.5.5.1
 * 
 * Revision 1.5.5.1  84/03/16  15:49:54  snider
 * It was possible to reference through a null pointer if a package body
 * was null, in dir().  This caused some segmentation faults.
 * 
 * Revision 1.5  84/01/12  14:24:56  ada
 * same as 1.4.2.1
 * 
 * Revision 1.4.2.1  84/01/09  11:52:14  willson
 * fixed up dir command
 * 
 * Revision 1.4  83/06/14  22:45:33  tadman
 * Added list of used packages to dir() output.
 * 
 * Revision 1.3  83/01/12  09:13:47  ada
 * Added copyright notice.
 * 
 * Revision 1.2  83/01/04  14:04:53  willson
 * Fixed bug with pp'ing an object (the output file wasn't init'ed).
 * New function fdirsym fixes the above problem with dirsym for
 * pp'ing objects.
 * 
 * Revision 1.1  82/12/12  21:22:58  tadman
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
#include "pp.h"

static FILE *dirf;
static int max_dir_count;
static enum pp_color_use color_use;
static int symbol_count;

extern color_name pp_count_color();
extern char *calloc();

get_dir_counts(fix)
    register fixp fix;
{
    register int    count;

    if ((fix -> fxf.class == func_id
		|| fix -> fxf.class == proc_id)
	    && fix -> fx.func.int_form)
	max_dir_count =
	    (max_dir_count < (count = get_count (fix->fx.func.int_form)))
	    ? count : max_dir_count;
}

dir_color_init()
{
    switch (color_use) {
	case ppc_count: 
	    max_dir_count = 0;
	    maptab (usrtab, get_dir_counts);
				/* find max count of procedure entries */
	    pp_count_init ();	/* set up to do color  */
	    break;
	case ppc_none: 
	    break;
	default: 
	    error ("That use of color not implemented");
    }
}

color_name dir_color(fix)
    fixp fix;
{
    switch (color_use) {
	case ppc_none: 
	    return samecolor;
	case ppc_count: {
	    int count;

	    if ((fix -> fxf.class == func_id
			    || fix -> fxf.class == proc_id)
			&& (fix -> fx.func.int_form)) {
		count = get_count (fix -> fx.func.int_form);
		fputf(dirf, "[%6d]  ",count);
		return pp_count_color(
			(max_dir_count == 0) ? 0 : 
			    (count * 100 / max_dir_count));
	    }
	    else {
		fputf(dirf, "          ");
	 	return cyan;
	    }
	}
    }
}

/*  Call fdirsym if you want service outside of this file
	(ppcref calls it).
*/

fdirsym(f, fix) FILE *f; fixp fix;
{	dirf _ f;
	dirsym(fix);
}

static dirsym(fix)
    fixp fix;
{
    set_fg(dir_color(fix));

    switch (fix -> fxf.class) {
	case obj_id: 
	    fputf (dirf, "%s: ", fix -> fxf.name);
	    if (fix -> fx.var.is_const) fputf(dirf,"constant ");
	    dopp (fix -> fx.var.int_form, dirf, false, false);
	    fputf (dirf, ";");
	    return;
	case except_id:
	    fputf(dirf, "%s: exception;", fix->fxf.name);
	    return;
	case macro_id: {	/* Castor */
		nodep m = fix -> fx.func.int_form;
		fputf (dirf, "macro {%s};",
			macro_pname (m -> nd.macrox.designator));
		return;
	    }
	case func_id: 
	    fputf (dirf, "function");
	    break;
	case proc_id: 
	    fputf (dirf, "procedure");
	    break;
	case pack_id: 
	    fputf (dirf, "package");
	    break;
	case type_id: 
	    fputf (dirf, "type");
	    break;
	case subtype_id: 
	    fputf (dirf, "subtype");
	    break;
	case undef_id:		/* don't print anything for deleted symbols */
	    fputf (dirf, "<deleted>");
	    break;
	default: 
	    fputf (dirf, "<unimplemented>");
    }
    fputf (dirf, " %s;", fix -> fxf.name);
}

static count_symbols(fix)
    fixp fix;
{
    if (fix->fxf.class != obj_id || ! fix->fx.var.is_enum_lit)
	symbol_count++;
}

static enter_symbols(fix,f)
    fixp fix;
    fixp **f;
{
    if (fix->fxf.class != obj_id || ! fix->fx.var.is_enum_lit)
	*(*f)++ = fix;
}


static int sym_class_order(fix)
    fixp fix;
{
    switch (fix -> fxf.class) {
	case type_id: 
	case subtype_id: 
	    return 1;
	case obj_id: 
	    return 2;
   	case except_id:
	    return 3;
	case func_id: 
	case proc_id: 
	    return 4;
	case macro_id:	/* Castor */
	    return 5;
	case pack_id: 
	    return 6;
	case undef_id:
	    return 7;
	default: 
	    return 0;
    }
}

static int symbol_compare(sym1, sym2)
    fixp *sym1, *sym2;
{
    register int t1, t2;

    t1 = sym_class_order(*sym1);
    t2 = sym_class_order(*sym2);
    if (t1 < t2) return -1;
    if (t1 > t2) return 1;
    return strcmp((*sym1)->fxf.name, (*sym2)->fxf.name);
}

static dodir(tab)
    symtabp tab;
{
    fixp *fixs, *f;
    int i;
    int sym_class, class;

    symbol_count = 0;
    maptab(tab, count_symbols);
    if (symbol_count == 0) {
	fputf(dirf, "  <empty>");
	return;
    }

    fixs = (fixp *)calloc(symbol_count, sizeof(fixp));
    f = fixs;
    maptab(tab, enter_symbols, &f);
    qsort(fixs, symbol_count, sizeof(fixp), symbol_compare);
    f = fixs;
    if (symbol_count > 0) class = sym_class_order(*f);
    fputf(dirf, "  ");
    for (i = 0; i < symbol_count; i++) {
	sym_class = sym_class_order(*f);
	if (sym_class != class) {	/* break on each different kind
					   of symbol */
	    class = sym_class;
	    fputf(dirf, "\n  ");
	}
	dirsym(*f++);
	fputf(dirf, "\n  ");
    }
    fputf(dirf, "\n");
    free(fixs);
}

static int strpcmp(a,b)		/* compare two pointers to strings */
    char **a, **b;
{
    return strcmp(*a, *b);
}

/* douse prints the packages used by this one in alphabetical order */

douse(u)
    usep u;
{
    int use_count = 0;
    usep uu;
    char **packs, **p;

    uu = u;
    while (uu) {
	use_count++;
	uu = uu->link;
    }

    if (use_count == 0) return;

    fputf(dirf, "\n  ");

    packs = (char **)calloc(use_count, sizeof(char *));

    p = packs;
    for (uu = u; uu; uu = uu->link)
	*p++ = uu->table->symtab_owner->fxf.name;

    qsort(packs, use_count, sizeof(char *), strpcmp);

    p = packs;
    while (use_count--) {
	fputf(dirf, "use %s;", *p++);
	fputf(dirf, "\n  ");
    }
    fputf(dirf, "\n");
    free(packs);
}

dir(fix, f, c_use) 
    fixp fix;
    FILE *f;
    enum pp_color_use c_use;
{
    color_pair cp;
    enum symbolclasses class;

    class = fix -> fxf.class;
    dirf = f;
    color_use = c_use;
    if (c_use != ppc_none) 
	what_color(&cp);	      /* save the current color */
    if (class eq pack_id) {
        dir_color_init();
	fputf (dirf, "Package Specification:\n");
	if (fix -> fx.pack.int_form -> nd.packagex.table) {
	    /* this if statement is always true, for some reason */
	    dodir(fix -> fx.pack.int_form -> nd.packagex.table);
	    if (c_use != ppc_none) 
		set_color(cp);     /* return terminal to original setting */
	    douse(fix -> fx.pack.int_form -> nd.packagex.table -> use_list);
	    fputf(dirf, "\n");
	}
	else fputf(dirf, "  <unelaborated>");
	fputf (dirf, "Package Body:\n");
	if (fix -> fx.pack.body_int_form) {
	    if (fix -> fx.pack.body_int_form -> nd.packagex.table) {
		dodir(fix -> fx.pack.body_int_form -> nd.packagex.table);
		if (c_use != ppc_none) 
		    set_color(cp);   /* return terminal to original setting */
		douse(fix -> fx.pack.body_int_form -> 
			nd.packagex.table -> use_list);
	    }
	}
	else fputf(dirf, "  <unelaborated>");
    }
    else if ((class eq proc_id) || (class eq func_id)) {
        dir_color_init();
	fputf (dirf, "Subprogram:\n");
	if (fix -> fx.func.int_form -> nd.subp.table) {
	    /* this if statement is always true, for some reason */
	    dodir(fix -> fx.func.int_form -> nd.subp.table);
	    if (c_use != ppc_none) 
		set_color(cp);     /* return terminal to original setting */
	    douse(fix -> fx.func.int_form -> nd.subp.table -> use_list);
	}
	else fputf(dirf, "  <unelaborated>");
    }
    else
	error ("Sorry, can't give directory of that kind of symbol");
    if (c_use != ppc_none) 
	set_color(cp);         /* return terminal to original setting */
}
