static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/tyint.v  Revision 1.5  84/01/12  16:49:25  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/tyint.v $
 * Revision 1.5  84/01/12  16:49:25  ada
 * same as 1.4.4.1
 * 
 * Revision 1.4.4.1  84/01/06  16:16:05  tadman
 * If type is a calling form, expand it and then call tyint again on
 * the expanded form.
 * 
 * Revision 1.4  83/03/17  14:34:10  ada
 * same as 1.2.40.1  (version 1.3 of this file is for Arcturus release 1.5 -
 * check with tadman for integration of that file when 1.5 is ready).
 * 
 * Revision 1.2.40.1  83/03/17  11:32:18  tadman
 * Deleted check for null range.  This used to cause a constraint_error,
 * but null ranges are legal in for loops and with the "in" operator.
 * 
 * Revision 1.2  83/01/12  09:10:36  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:47:50  tadman
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

typep tyeval();		/* forward declaration */

typep lookuptypeid(id, needcomplete)
    char *id;
{
    register    fixp f;
    register    typep typ;

    f = lookup(currtab, id, null);
    if (f == null)
	error ("type undefined: %s", id);
    typ = f -> fxf.dyna -> sy.var.type;
    if (needcomplete && typ -> tyf.int_form == null)
	error ("no size for type %s", f -> fxf.name);
    return (f -> fxf.dyna -> sy.var.type);
}

int rec_fields(tab, fieldp, recsize)
    symtabp tab;
    nodep fieldp;
    int recsize;
{
    nodep whens;
    fixp ids, temp;
    symp idsym;
    typep fldtyp;

    while (fieldp) {
	if (fieldp -> ndf.op != nop_rec_var) {
	    ids = (fixp) eval(fieldp -> nd.obj_decl.id_list,
		    decl_mode,
		    &dummy,
		    null);
	    fldtyp = tyeval (fieldp -> nd.obj_decl.subtypex,
		    null, false);
	/* worry about init */
	    while (ids) {
		idsym = newsym(varsymbolform, obj_id);
		ids -> fxf.class = field_id;
		idsym -> sy.var.type = fldtyp;
		*(idsym -> sy.var.value) = recsize;
		recsize += fldtyp -> tyf.size;
		temp = ids -> fxf.next;  /* save link */
		enter(tab, ids, idsym);
		ids = temp;
	    }
	}
	else {			/* must be a varient part */
	    int r = recsize;
	    whens = fieldp->nd.casex.when_list;
	    while (whens) {
		int size = rec_fields(tab, whens->nd.whenx.stmt, r);
	        if (size > recsize) recsize = size;
	    	whens = whens->ndf.next;
	    }
	}
	fieldp = fieldp -> ndf.next;
    }
    return recsize;
}


typep tyeval(p, d, new)
    register nodep p;
    register nodep d;	/* pointer to discriminant (or null if none) */
    boolean new;
{
    typep ret;

        if(p==null) return null;

/*      putf("curnode = %s\n",nodename(p));   */ /* debugging aid */

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

        switch(p->ndf.op) {
                case nop_access: {
                        typep t;
                        typep tgt;
                        nodep tgn;
                        t = newtype(accessform);
                        t->tyf.int_form = p;
                        t->tyf.size = sizeof(int *);

                        tgn = p->nd.accessx.target_type;
                        if(tgn->ndf.op == nop_ident)
                                tgt = lookuptypeid(tgn->nd.ident.id, false);
                        else tgt = tyeval(tgn, null, false);
                        t->ty.access.targettype = tgt;

                        ret = t;
                    }
			break;
                case nop_record: {
                        typep t;
                        int recsize;
                        nodep fieldp;
			symtabp tab;

                        t = newtype(recordform);
                        t->tyf.int_form = p;
			tab  = newtab(16, null);
                        t->ty.record.fieldtab = tab;

                        fieldp = p->nd.recordx.field_list;
			recsize = rec_fields(tab, d, 0);
			recsize = rec_fields(tab, fieldp, recsize);
                        t->tyf.size = recsize;
		        ret = t;
                    }
		    break;
                case nop_index: {
			nodep ixnode;
                        typep t;
                        typep inxtyp, lsttyp;
                        int size = 0;
                        boolean unconstrained = false;

                        ixnode = p->nd.index.typex;
                        inxtyp = tyeval(ixnode, null, false);
                        if(ixnode->ndf.op == nop_range &&  /* lookahead */
                        ixnode->nd.rangex.not_constrained)
                             unconstrained = true;
                        else size = rangelength(inxtyp);
                        if(p->ndf.next) {
                          lsttyp = tyeval(p->ndf.next, null, false);
                          size *= lsttyp->tyf.size;
                          if(unconstrained != lsttyp->ty.index.not_constrained)
                            error("can't mix un/constrained indices");
                        }
                        else lsttyp = null;
                        t = newtype(indexform);
                        t->tyf.int_form = p;
                        t->ty.index.range = inxtyp;
                        t->ty.index.next = lsttyp;
                        t->ty.index.not_constrained = unconstrained;
                        t->tyf.size = size;
                        ret = t;
                    }
		    break;
                case nop_array: {
                        typep un_arrtyp;
                        typep con_arrtyp;
                        typep ret_arrtyp;
                        typep inxtyp;
                        typep eletyp;

                        inxtyp = tyeval(p->nd.arrayx.index_list, null, false);
                        eletyp = tyeval(p->nd.arrayx.ele_type, null, false);
                        un_arrtyp = newtype(un_arrayform);
                        un_arrtyp->ty.un_array.ele_type = eletyp;
                        if(inxtyp->ty.index.not_constrained) {
                                un_arrtyp->ty.un_array.index_list = inxtyp;
                                ret_arrtyp = un_arrtyp;
                        }
                        else {
                                con_arrtyp = newtype(con_arrayform);
                                con_arrtyp->ty.con_array.index_list = inxtyp;
                                con_arrtyp->ty.con_array.base_type = un_arrtyp;
                                con_arrtyp->tyf.size = eletyp->tyf.size * inxtyp->tyf.size;
                                ret_arrtyp = con_arrtyp;
                        }
                        ret_arrtyp->tyf.int_form = p;
			ret = ret_arrtyp;
                    }
		    break;
                case nop_call: {  /* index-constraint */
                        typep un_arrtyp;
                        typep inxtyp;
                        typep con_arrtyp;
                        typep rngtyp;   /* temp */

                        un_arrtyp = tyeval(p->nd.call.var, null, false);
                        if(un_arrtyp->tyf.class != un_arrayform)
                                error("not an unconstrained array");
                        /* this should be an arglist (nop_arg) */
                        /* not just a range, arg==indexlist */
                        inxtyp = newtype(indexform);
                        rngtyp = tyeval(p->nd.call.args, null, false);
                        inxtyp->ty.index.range = rngtyp;
                        inxtyp->tyf.size = rangelength(rngtyp);
                        /* check compatibility of range lists */
                        con_arrtyp = newtype(con_arrayform);
                        con_arrtyp->ty.con_array.index_list = inxtyp;
                        con_arrtyp->ty.con_array.base_type = un_arrtyp;
                        con_arrtyp->tyf.size =
                          un_arrtyp->ty.un_array.ele_type->tyf.size
                          * inxtyp->tyf.size;
                        con_arrtyp->tyf.int_form = p;
                        ret = con_arrtyp;
                    }
		    break;
                case nop_range: {
                    typep newtyp, basetype;
                    typedval up, low;
                    symp s;
                    char *name = null;
                    basetype = null;
                    if(p->nd.rangex.type_id)
                        name = p->nd.rangex.type_id->nd.ident.id;  /* assumes id */
                    if(name) {
                            basetype = lookuptypeid(name, true);
                            /* discrete type */
                    }
                    if(p->nd.rangex.not_constrained) ret = basetype;
		    else {
                        newtyp = newtype(srangeform);
                        newtyp->tyf.int_form = p;
                        newtyp->tyf.size = sizeof(int);
                        eval(p->nd.rangex.left, eval_mode, &low, null);
                        if(basetype)     {
                                rangecheck(basetype, &low);
                        }
                        newtyp->ty.srange.lbound = low.val.integer;
                        eval(p->nd.rangex.right, eval_mode, &up, null);
                        newtyp->ty.srange.ubound = up.val.integer;
			if (basetype)
			    rangecheck(basetype, &up);
			else {
                            typecheck(low.typ, up.typ);
                            basetype = base(low.typ);
			}
                        newtyp->ty.srange.basetype = basetype;
                        ret = newtyp;
		    }
                    }
		    break;
                case nop_enum: {
                    nodep id;
		    fixp f;
                    symp s;
                    typep enumtyp;
                    int len;
                    char **nametab;

                    enumtyp = newtype(enumform);
                    enumtyp->tyf.int_form = p;
                    enumtyp->tyf.size = sizeof(int);
                    len = 0;
                    for(id = p->nd.enumtype.ids; id; id = id->ndf.next)
                        len++;
                    nametab = (char **)zalloc(len * sizeof(char *) , 0);
                    len = 0;
		    for(id = p->nd.enumtype.ids; id; id = id->ndf.next) {
			f = newfix(varfixform, obj_id);
                        s = newsym(varsymbolform, obj_id);
                        f->fxf.name = id->nd.id_decl.id;
                        s->sy.var.type = enumtyp;
                        nametab[len] = id->nd.id_decl.id;
                        *s->sy.var.value = len++;
                        f->fx.var.is_const = true;
			f->fx.var.is_enum_lit = true;
                        enter(currtab,f,s);
                    }
                    enumtyp->ty.srange.lbound = 0;
                    enumtyp->ty.srange.ubound = len - 1;
                    enumtyp->ty.enumer.names = nametab;
                    ret = enumtyp;
                    }
		    break;
                case nop_dot: {
                        typedval t;
                        dodot(p, decl_mode, &t);
                        if(t.typ != typetype)
                                error("selected expression is not type");
                        ret = t.val.tptr;
                    }
		    break;
                case nop_quote: {
                        typedval t;
                        doattr(p, &t);
                        if(t.typ != typetype)
                                error("attribute does not return type");
                        ret = t.val.tptr;
                    }
		    break;
                case nop_ident: {
		  	typep typ, newtyp;

                        typ = lookuptypeid(p->nd.ident.id, true);
			if (new) {
			    newtyp = newtype(subtypeform);
			    newtyp->tyf.int_form = p;
			    newtyp->tyf.size = typ->tyf.size;
			    newtyp->ty.subtype.basetype = typ;
			    ret = newtyp;
			}
			else ret = typ;
		    }
		    break;
		case nop_derived_type: {
			typep parent, typ;

			parent = 
			    tyeval(p->nd.derived_type.typex, null, false);
			typ = newtype(derivedform);
			typ->tyf.int_form = p;
			typ->tyf.size = parent->tyf.size;
			typ->ty.derived.parenttype = parent;
			ret = typ;
		    }
		    break;
		case nop_type_callf:
		case nop_name_callf:
		case nop_expr_callf: {
		    boolean defined;
		    typedval result;

		    defined = macro_call(p,decl_mode, &result);
		    if (defined) {
			dyn_macro_expand(result.val.nptr, p);
			p = result.val.nptr;
			ret = tyeval(p, d, new);
		    }
		    else error("macro {%s} is not defined", macro_pname(p));
		}
		    break;
                default:
                    error("Can't evaluate type of %s; try something else", 
			   nodename(p));
	}
    intsp--;
    return ret;
}
