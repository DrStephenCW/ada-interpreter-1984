static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/dump.v  Revision 1.7  83/04/05  15:06:20  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/dump.v $
 * Revision 1.7  83/04/05  15:06:20  ada
 * changed inputtype and outputtype to filetype to reflect ada '82 io.
 * 
 * Revision 1.6  83/04/05  14:31:35  ada
 * same as 1.4.8.1
 * 

 * Revision 1.5  83/03/10  15:27:27  ada
 * Same as 1.4.2.1.
 *
 * Revision 1.4.8.1  83/03/10  15:56:05  sca
 * Added putsym so it prints out any data type in an Ada aggregate like
 * fashion. Currently it will only print out array of one dimension with
 * an integer index.
 * 
 * Revision 1.4.2.1  83/03/10  14:44:15  willson
 * *** empty log message ***
 * 
 * Revision 1.3  83/01/12  09:13:58  ada
 * Added copyright notice.
 * 
 * Revision 1.2  82/12/30  13:58:00  ada
 * Same as 1.1.1.1
 * 
 * Revision 1.1.1.1  82/12/20  15:16:23  willson
 * Changed to save use list for usr table.
 * 
 * Revision 1.1  82/12/12  21:23:24  tadman
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

#define willson_version 1

/* 	This file contains routines to dump the execution environment
	to a file.
*/

char *savename();

static FILE *dumpfile;

/* stuff for managing list of top level type declarations */
static nodep type_list_head;
static nodep type_list_tail;
static boolean comma;
static boolean newline;

putsym(thing)
fixp thing;
{
    register    typep typ = thing -> fxf.dyna -> sy.var.type;
    register    enum typeforms t;
    boolean hasval = true;

    if (typ -> tyf.class == srangeform)
	typ = base (typ);
    t = typ -> tyf.class;
 /* type always swapped in. (?) */
    if (t == enumform && thing -> fx.var.is_const)
	return;			/* should have enum_id */

    if (typ == texttype)
	hasval = false;

    else
	if (typ == filetype) {
	    if (thing -> fxf.dyna -> sy.var.value != null)
		sysputf ("closing %s", thing -> fxf.name);
	    hasval = false;
	}

	else
	    if (thing -> fx.var.int_form == null) {
		sysputf ("can't put %s\n", thing -> fxf.name);
		return;
	    }

    comma = false; /* Don't print commas or newline the first time through */
    newline = false;
    if (hasval)
	putname (current_output(), mkidnode (thing -> fxf.name), typ);
				/* should free this */
}


putname(f, p, typ)
   FILE *f;
   nodep p;
   typep typ;
{
        typedval e;
        typ = base(typ);
        switch(typ->tyf.class) {
            case intform:
            case realform:
            case enumform:
                eval(p, eval_mode, &e, null);
                typecheck(typ, e.typ);
                typedprint(f, e.val.integer, e.typ);
                break;
            case undefform:
		fputf(f,"undefform ");
                break;
            case strform:
		fputf(f,"strform ");
                break;
            case indexform:
		fputf(f,"indexform ");
                break;
            default:
                sysputf("wierd case %d in putsym", typ->tyf.class);
                break;
            case con_arrayform:
                putarray(f, p, typ);
                break;
            case recordform:
                putrecord(f, p, typ);
                break;
            case accessform:
                fputf(f,"(access object)");
                break;
        }
}


putarray(f, p, typ)
FILE *f;
nodep p;
typep typ;
{
        /* assumes one-dimensional array for now */
        int i,l,u;
        nodep const, arg, call;
        nodep attr, arg2, call2;
        typedval e;
        typep ele_type, index_type;

        ele_type = typ->ty.con_array.base_type->ty.un_array.ele_type;
        index_type = typ->ty.con_array.index_list->ty.index.range;
        const = newnode(integer_node, nop_int);
        const->nd.integer.prval = null;   /* let prettypriner do conversion */
        if(not compatible(index_type, inttype)) {
                attr = newnode(select_node, nop_quote);
                attr->nd.selectx.var = base(index_type)->tyf.int_form;
                attr->nd.selectx.field = intern("val");
                arg2 = newnode(arg_node, nop_arg);
                arg2->nd.arg.exp = const;
                call2 = newnode(call_node, nop_call);
                call2->nd.call.var = attr;
                call2->nd.call.args = arg2;
        }
        else call2 = const;
        arg = newnode(arg_node, nop_arg);
        arg->nd.arg.exp = call2;
        call = newnode(call_node, nop_call);
        call->nd.call.var = p;
        call->nd.call.args = arg;

        l = index_type->ty.srange.lbound;
        u = index_type->ty.srange.ubound;
	if (base(ele_type)==chartype)
            {
            fputf(f,"\"");
            for(i = l; i <= u; i++)
	        {
                const->nd.integer.val = i;
                /* putname(f, call, ele_type); */
                /* typ = base(typ); */
                eval(call, eval_mode, &e, null);
                /* typecheck(typ, e.typ); */
                /* typedprint(f, e.val.integer, e.typ); */
                if (e.val.integer > 0) {
                    if (e.val.integer < ' ' || e.val.integer == 127)
                        fputf (f, "character^val(%d)", e.val.integer);
                    else
                        fputf (f, "%c", e.val.integer);
                    } }
            fputf(f,"\"");
            }
            else
	    {
	    comma = false;
            fputf(f,"(");
            for(i = l; i <= u; i++) 
                {
                const->nd.integer.val = i;/* what about array(color) of ... */
                if (comma) fputf(f,","); /* print commas after the first */
                comma = true;            /* invokation */
                fputf(f,"%d=>",i);
                putname(f, call, ele_type);
                }
             fputf(f,")");
             }
        free(call);
        free(arg);
        free(const);
        /* should free attr, arg2, and call2 */
}

enter_save_list(p)
    nodep p;
{
        if(type_list_head == null)
                type_list_head = p;
        else 
                type_list_tail->ndf.next = p;
        type_list_tail = p;
}

dump_type() 
{
    register    nodep p;
    for (p = type_list_head; p != null; p = p -> ndf.next)
	dopp (p, dumpfile, true, true);
}

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
         
char *enumimage(typ, val) 
    typep typ;
    int val;
{
    if (val < typ -> ty.srange.lbound || val > typ -> ty.srange.ubound)
	error ("enum value out of bounds");
    else
	return (typ -> ty.enumer.names[val]);
}


printimage(res, val, typ) /* put print image in res based on typ */
    char *res;
    int val;                /* only works for scalars */
    typep typ;
{
    typ = base (typ);
    switch (typ -> tyf.class) {
	case intform: 
	    sprintf (res, "%d", val);
	    break;
	case realform: 
	    sprintf (res, "%g", val);/* can print integer */
	    break;		/* which is a major flaw */
	case enumform: 
	    if (typ == chartype)
		if (val < ' ' || val == 127)
		    sprintf (res, "character^val(%d)", val);
		else
		    sprintf (res, "'%c'", val);
	    else
		sprintf (res, "%s", enumimage (typ, val));
	    break;
	default: 
	    sprintf (res,
 "Don't know how to print result (address: 8#%o) of that expression;\n\(Maybe missing ';' above) ",
		    val);

    }
}

typedprint(f, val, typ)
    FILE *f;
    int val;
    typep typ;
{
    char    buf[128];
    printimage (buf, val, typ);
    fputf (f, buf);
}

dumpobj(f, fix)
    FILE *f;
    fixp fix;
{
    register    typep typ = fix -> fxf.dyna -> sy.var.type;
    register    enum typeforms t;
    boolean hasval = true;

    if (typ -> tyf.class == srangeform)
	typ = base (typ);
    t = typ -> tyf.class;
 /* type always swapped in. (?) */
    if (t == enumform && fix -> fx.var.is_const)
	return;			/* should have enum_id */

    if (typ == texttype)
	hasval = false;

    else
	if (typ == filetype) {
	    if (fix -> fxf.dyna -> sy.var.value != null)
		sysputf ("closing %s", fix -> fxf.name);
	    hasval = false;
	}

	else
	    if (fix -> fx.var.int_form == null) {
		sysputf ("can't save %s\n", fix -> fxf.name);
		return;
	    }

    fputf (f, "%s: ", fix -> fxf.name);
    dopp (fix -> fx.var.int_form, f, false, false);
    fputf (f, ";\n");
    if (hasval)
	dumpname (f, mkidnode (fix -> fxf.name), typ);
				/* should free this */
}

dumpname(f, p, typ)
   FILE *f;
   nodep p;
   typep typ;
{
        typedval e;
        typ = base(typ);
        switch(typ->tyf.class) {
            case intform:
            case realform:
            case enumform:
                eval(p, eval_mode, &e, null);
                typecheck(typ, e.typ);
                if(e.val.integer != 0) {
                        dopp(p, f, false, false);
                        fputf(f, " := ");
                        typedprint(f, e.val.integer, e.typ);
                        fputf(f, ";\n");
                }
                break;
            case undefform:
            case strform:
            case indexform:
            default:
                sysputf("wierd case %d in dumpobj", typ->tyf.class);
                break;
            case con_arrayform:
                dumparray(f, p, typ);
                break;
            case recordform:    
                dumprecord(f, p, typ);
                break;
            case accessform:    
                sysputf("can't save value of object:");
                dopp(p, current_output(), false, false);
                putf("\n");
                break;
        }
}

dumparray(f, p, typ)
FILE *f;
nodep p;
typep typ;
{
        /* assumes one-dimensional array for now */
        int i,l,u;
        nodep const, arg, call;
        nodep attr, arg2, call2;
        typep ele_type, index_type;

        ele_type = typ->ty.con_array.base_type->ty.un_array.ele_type;
        index_type = typ->ty.con_array.index_list->ty.index.range;
        const = newnode(integer_node, nop_int);
        const->nd.integer.prval = null;   /* let prettypriner do conversion */
        if(not compatible(index_type, inttype)) {
                attr = newnode(select_node, nop_quote);
                attr->nd.selectx.var = base(index_type)->tyf.int_form;
                attr->nd.selectx.field = intern("val");
                arg2 = newnode(arg_node, nop_arg);
                arg2->nd.arg.exp = const;
                call2 = newnode(call_node, nop_call);
                call2->nd.call.var = attr;
                call2->nd.call.args = arg2;
        }
        else call2 = const;
        arg = newnode(arg_node, nop_arg);
        arg->nd.arg.exp = call2;
        call = newnode(call_node, nop_call);
        call->nd.call.var = p;
        call->nd.call.args = arg;

        l = index_type->ty.srange.lbound;
        u = index_type->ty.srange.ubound;
        for(i = l; i <= u; i++) {
                const->nd.integer.val = i;  /* what about array(color) of ... */
                dumpname(f, call, ele_type);
        }
        free(call);
        free(arg);
        free(const);
        /* should free attr, arg2, and call2 */
}

dumpfield(fix, f, p)  /* called indirectly by maptab */
    fixp fix;
    FILE *f;
    nodep p;
{
    p -> nd.selectx.field = fix -> fxf.name;
    dumpname(f, p, fix -> fxf.dyna -> sy.var.type);
}

putfield(fix, f, p)  /* called indirectly by maptab */
    fixp fix;
    FILE *f;
    nodep p;
{
    p -> nd.selectx.field = fix -> fxf.name;
    if (comma) fputf(f,","); /* print commas after the first */
    comma = true;            /* invokation of putname        */
    fputf(f,fix -> fxf.name);
    fputf(f,"=>");
    putname(f, p, fix -> fxf.dyna -> sy.var.type);
}

putrecord(f, p, typ)
    FILE *f;
    nodep p;
    typep typ;
{
    nodep select;

    select = newnode (select_node, nop_dot);
    select -> nd.selectx.var = p;
    putf("(");
    comma = false;
    maptab (typ -> ty.record.fieldtab, putfield, f, select);
    putf(")");
    free (select);
}

dumprecord(f, p, typ)
    FILE *f;
    nodep p;
    typep typ;
{
    nodep select;

    select = newnode (select_node, nop_dot);
    select -> nd.selectx.var = p;
    maptab (typ -> ty.record.fieldtab, dumpfield, f, select);
    free (select);
}

dumptype(fix)
    fixp fix;
{
    FILE * d;
    char    name[256];
    char   *keyw;

    if (fix -> fxf.class == type_id)
	keyw = "type";
    else
	if (fix -> fxf.class == subtype_id)
	    keyw = "subtype";
	else
	    return;
    fputf (dumpfile, "%s %s;\n", keyw, fix -> fxf.name);
    if (fix -> fx.var.int_form) {
	savename (name, fix);
	d = fopen (name, "w");
	if (d == NULL) {
	    sysputf ("can't create %s\n", name);
	    return;
	}
	fputf (d, "%s %s is\n   ", keyw, fix -> fxf.name);
	dopp (fix -> fx.var.int_form, d, false, false);
	fputf (d, ";\n");
	fclose (d);
    }
}

dumpsym(fix)
    fixp fix;
{
    switch (fix -> fxf.class) {
	case func_id: 
	case proc_id: 
/*
                dopp(fix->fx.func.int_form, dumpfile, true, true);
*/
	    fputf (dumpfile, "pragma %s_name(%s);\n",
		    fix -> fxf.class == func_id ? "function" : "procedure",
		    fix -> fxf.name);
	    break;
	case obj_id: 
	    dumpobj (dumpfile, fix);
	    break;
	case pack_id: 
	    fputf (dumpfile, "pragma package_name(%s);\n", fix -> fxf.name);
	    break;
    }
}

char *savename(name, fix)
    char *name;
    fixp fix;
{
    strcpy (name, "user/");
    strcat (name, fix -> fxf.name);
    return (name);
}

#ifdef willson_version

dump_uses_list()
{	usep	use_list _ currtab -> use_list;
	fputf(dumpfile,"-- Uses list (Dangerous to re-arrange manually)\n");
	while (use_list) {
		fputf(dumpfile, "use %s;\n", 
			use_list -> table -> symtab_owner -> fxf.name);
			/* This doesn't cover the case where the symbol
			   used was not directly visible and was selected
			   via dot notation; the read of ADA.SAVE on
			   startup will die. (shw)
			*/
		use_list _ use_list -> link;
	}
	fputf(dumpfile,"-- End of uses list\n");
}

dump_subps_and_packs_only(fix)
    fixp fix;
{
    switch (fix -> fxf.class) {
	case func_id: 
	case proc_id: 
	    fputf (dumpfile, "pragma %s_name(%s);\n",
		    fix -> fxf.class == func_id ? "function" : "procedure",
		    fix -> fxf.name);
	    break;
	case pack_id: 
	    fputf (dumpfile, "pragma package_name(%s);\n", fix -> fxf.name);
	    break;
	default: fputf(dumpfile, "-- object %s not saved\n", fix -> fxf.name);
    }
}

#endif willson_version

dumpusr() {
        dumpfile = fopen(SWAPNAME, "w");
        if(dumpfile == null) {
                sysputf("can't save functions");
                return;
        }
#ifdef willson_version
	fputf(dumpfile,"-- Edit this file at your own risk!\n");
	fputf(dumpfile,"-- This file was created automatically by Arcturus\n");
	maptab(usrtab, dump_subps_and_packs_only);
	dump_uses_list();
#else
        dump_type();
        maptab(usrtab, dumpsym);
#endif willson_version
        fclose(dumpfile);
}

FILE *savefile(fix)
    symp fix;
{
    register    FILE *f;
    char    name[256];

    savename (name, fix);
    f = fopen (name, "w");
    if (f == null) {
	f = fopen ("user", "r");
	if (f != null) {
	    fclose (f);
	    f = null;
	}
	else
	    system ("mkdir user");
	f = fopen (name, "w");
    }
    if (f == null)
	error ("can't open %s for writing", name);
    return f;
}

subp_writeout(fix) 
    fixp fix;
{
    register  FILE* savef;

    savef = savefile(fix);
    if (fix -> fx.func.int_form)
	dopp (fix -> fx.func.int_form, savef, true, true);
    else
        error ("error in subp_writeout");
    fclose (savef);
}

pack_writeout(fix)
    fixp fix;
{
    register    FILE * savef;
    savef = savefile (fix);
    if (fix -> fx.pack.int_form) {
	dopp (fix -> fx.pack.int_form, savef, true, true);
	if (fix -> fx.pack.body_int_form)
	    dopp (fix -> fx.pack.body_int_form, savef, true, true);
    }
    else
	error ("error in pack_writeout");
    fclose (savef);
}

