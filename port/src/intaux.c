static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/intaux.v  Revision 1.12  83/12/05  16:53:27  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/intaux.v $
 * Revision 1.12  83/12/05  16:53:27  ada
 * same as 1.11.5.1
 * 
 * Revision 1.11.5.1  83/12/05  16:46:51  snider
 * simulate overloading of 'put' for putsym in docall.
 * 
 * Revision 1.11  83/09/08  15:42:13  ada
 * same as 1.10.9.1
 * 
 * Revision 1.10.9.1  83/08/25  14:29:27  ritter
 * Added code to check for functions that do not return any value.  Raises
 * program error when one is found.
 * 
 * Revision 1.10  83/08/08  17:28:05  ada
 * same as 1.9.5.1
 * 
 * Revision 1.9.5.1  83/08/08  16:17:14  snider
 * corrected two error messages that were reversed ("Attempt to use procedure
 * like a function...").  Also, check after a swapin to see whether the
 * tree for a subprogram is still null  -  prevents a crash if the file
 * usr/filename has been monkeyed with and no longer contains the code
 * for the subprogram.  Both of these in docall().  w. thanks to 
 * brindle@aerospace.
 * 
 * Revision 1.9  83/06/14  16:29:28  ada
 * Same as 1.8.4.1.
 * 
 * Revision 1.8.4.1  83/06/14  14:00:15  tadman
 * Added context specification argument to doscope calls.
 * 
 * Revision 1.8  83/05/09  13:59:37  ada
 * same as 1.6.4.2
 * 
 * Revision 1.6.4.2  83/04/14  08:27:42  tadman
 * Check for at least one symbolform attached to a subprogram before
 * allowing of selection of components within it.  This is by no means
 * a complete test for the validity of selection, but it does eliminate
 * a common source of segmentation faults.
 * 
 * Revision 1.6.4.1  83/04/11  23:13:16  tadman
 * Fixed problem with loops without scopes (bare loop and while) causing
 * an extra layer of symbolforms to be removed from the current scope.
 * 
 * Revision 1.6  83/03/10  15:36:17  ada
 * Same as 1.5.2.1.
 * 
 * Revision 1.5.2.1  83/03/10  14:47:15  willson
 * *** empty log message ***
 * 
 * Revision 1.4  83/01/12  09:16:26  ada
 * Added copyright notice.
 * 
 * Revision 1.3  83/01/11  14:54:08  ada
 * Same as 1.1.1.1.2.1.
 * 
 * Revision 1.1.1.1.2.1  83/01/11  09:53:34  willson
 * Uses new arr.h array dope vector when calling
 * compiled code.
 * 
 * Revision 1.1.1.1  82/12/20  15:21:12  willson
 * Changed subp_swapin (and package_swapin) to swap in code from usr/ to
 * the correct scope.
 * 
 * Revision 1.1  82/12/12  21:28:47  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"
#include "arr.h"
#include "count.h"
#include "oblist.h"

extern nodep intstk[], *intsp, *intstk_end;

extern nodep handler;		/* exception handler -- set by raise */
extern long stack_level;

extern	typep typeonly();

extern symtabp parameter_table;	/* global to pass parameter table to
				   nop_param */

 /* checkrange checks that a typed value (r) is in the range of any
    subrange constraint imposed by type t.  The type of r and t must be
    of the same class of types.  rangecheck first type checks the two
    types for assignment compatibility and then does the rangecheck.

    The reason for the two functions is that checkrange can be used to
    where the types are not assignment compatible, but one is a derived
    type or a subtype of a derived type whos parent type is the other. */

long checkrange(t,r)
    typep t;
    typedval *r;
{
    if(t->tyf.class == srangeform) {
        if(r->val.integer < t->ty.srange.lbound || 
	   r->val.integer > t->ty.srange.ubound)
        sysraise(constraint_error,
	    "constraint_error: value (%d) out of range (%d .. %d)",
            r->val.integer, t->ty.srange.lbound, t->ty.srange.ubound);
        }                                    
}

long rangecheck(t, r)
    typep t;
    typedval *r;
{
    typecheck(t, r->typ);
    checkrange(t, r);
}

/*	Dodot handles the nop_dot node.  The right side of such a node
	is always an identifier.  The left side might be arbitrarily
	complex.  a(i).b(j)  binds like this:

		(call (dot (call a (arg i)) b) (arg j))

	The first call node will handle argument j, so our job is to
	return b's address (or symbol, as required).  We evaluate the
	left hand side (in the appropriate mode) and then look at what
	we get back.  If it's a package, we look in the visible table.
	If it is a function, we look in	the function's symbol table.  
	Otherwise, we assume that we got a record offset.

	Note: if we are called in decl_mode, this is the same as addr_mode
	or symbol_node for packages or functions, but illegal for record
	offsets.  This would be used for something like:
		subtype x is new standard.integer;
*/

long dodot(p, mode, result)                          
nodep p;
enum int_modes mode;        
typedval *result;
{                             
        typedval lhs;   	/* result of eval'ing left hand side */
        typep typ;		/* short hand for type of left side */
        symtabp tab;		/* used for looking in pack and func tables */
	fixp symbol;		/* symbol found */
	word addr;		/* ? */
        char *fldname;		/* name of identifier on right hand side */
	boolean access;		/* true if reference is through an access
				   type */

	/* First we evaluate the left-hand side.  We'll do it in the mode
	we were given, since addr_mode and symbol_mode are almost the same
	except that addr_mode allows some extra evaluation to occur. */


	{enum int_modes temp_mode;

	/* if evaluating in eval_mode, then we'll eval the selector in
	addr_mode because we assume later that we've done that.  If in
	symbol_mode, we want to eval the selector in ovl_mode, since we
	don't want a symbol, we want the type of the selector.  
	*/

	if (mode eq symbol_mode) temp_mode _ ovl_mode;
	else if (mode eq eval_mode) temp_mode _ addr_mode;
	else temp_mode _ mode;

        eval(p->nd.selectx.var, 
	     temp_mode,
	     &lhs, null);
	}

        fldname = p->nd.selectx.field;
        typ = lhs.typ;

	/* first maybe subprogram... */

        if(typ == functype) {
            if(fldname) {          
		nodep iform;

		/* Get the symbol table from the internal form for the
		subprogram. */
		iform _ lhs.val.fptr->fx.func.int_form;
		tab _ iform->nd.subp.table;
			/* This test isn't complete, but it's better
			   than before the second part was added.  */
                if(tab == null || lhs.val.fptr->fxf.dyna == null)
                  error("Attempt to peer inside inactive procedure");
                symbol = lookuptab(tab, fldname, null);
                if(symbol == null)
                    error("%s on rhs of selector not found", fldname);
            }
            else error("illegal use of .all in selector");
            doident(symbol, mode, result);
            return;
        }

	/* or maybe package ... */

        else if(typ == packtype) {
            symp s _ null;
	    fixp f;

            if(fldname) {          
                symbol = lhs.val.fptr;
                if(symbol == null)
                    system_error("Can't find package symbol in selector");
                tab = symbol->fx.pack.int_form->nd.packagex.table;
                if(tab == null)
                   error("Attempt to peer inside inactive package");
                f = lookuptab(tab, fldname, null);
                if(f) {
			doident(f,mode,result);
 			return;
		}

		/* if we can't find it, but the package is on the stack,
		that means stuff in the body is visible (note: this
		comment is out of date, but I don`t know how to make
		an equivalent test [FPT]) */

                if(s eq null) {
                        tab = symbol->fx.pack.body_int_form->
				     nd.packagex.table;
                        if(tab == null)
                            error("Attempt to peer inside inactive package");
                        f = lookuptab(tab, fldname, null);
			if (f) {
				doident(f,mode,result);
                                return;
			}
                }

                if(f == null)
                    error("%s not found in selector", fldname);
            }
	    /* a null rhs means .all */
            else error("illegal use of .all");
            return;  
        }
        else if(mode == decl_mode)
                error("bad selected expression in declaration");

	/* assume record selector */

	addr = lhs.val;		/* points to base address */
                         
	/*  Evaluating in ovl_mode, we return the base of the
	record and the record type, same as for addr_mode. */

	/*  Get down to the base record type.  To do this, it goes
	    through all derived, access, and subtypes to get to the
	    base type of the record. */

	access = false;
        while (typ->tyf.class eq accessform || typ->tyf.class eq subtypeform
		|| typ->tyf.class eq derivedform) {
	    switch (typ->tyf.class) {
		case accessform:
	  	    addr.integer = *addr.ptr;
          	    if(addr.ptr == null)
            	    sysraise(constraint_error,
		     "constraint_error: reference through null access value");
          	    typ = typ->ty.access.targettype;
		    access = true;
		    break;
		case subtypeform:
		    typ = typ->ty.subtype.basetype;
		    break;
		case derivedform:
		    typ = typ->ty.derived.parenttype;
		    break;
	     }
	}

	  /* return pointer to object because .all specified? */
        if(fldname == null)
	    if (access) {
            	if (mode eq eval_mode && not blocktype(typ))
		    addr.integer = *addr.ptr;
            	result->val = addr;       
            	result->typ = typ;
            	return;
            }   
	    else error("illegal use of .all in record selector");

        if (typ->tyf.class neq recordform)
            error("Record type expected in lhs of selector");

        tab = typ->ty.record.fieldtab; /* points to record's symtab */

        symbol = lookuptab(tab, fldname, null);
                    /* lookup offset */
        if (symbol eq null)
            error("Record field %s not found", fldname);      

	addr.integer += *(symbol->fxf.dyna->sy.var.value);
                    /* add offset and base addr */

        typ = symbol->fxf.dyna->sy.var.type;   
        if (mode eq eval_mode) {   
            if(not blocktype(symbol->fxf.dyna->sy.var.type))
		addr.integer = *addr.ptr;
	        result->val = addr;       
	        result->typ = typ;
	        return;
        }
        else if (mode eq addr_mode or mode eq ovl_mode) {
	        result->val = addr;       
	        result->typ = typ;
	        return;
	}
	else if (mode eq symbol_mode) {
		result->typ _ symboltype;
		result->val.fptr _ symbol;
		return;
	}
	else system_error("Can't handle mode (%d) in 'dodot'", mode);
}

long blockmove(dst, src, n)
char *dst, *src;
long n;
{
        /* we must be careful which end we copy from            */
        /* in case we do assignments between slices of          */
        /* the same array:                                      */
        /*      a(1..3) := a(2..4); -- start at low address     */
        /*      a(2..4) := a(1..3); -- start at high address    */
        if(dst > src) {
                /* this is 'moveright' in UCSD terminology */
                dst += n;
                src += n;
                while(n--)
                        *--dst = *--src;
        }
        else {
                /* 'moveleft' */
                while(n--)
                        *dst++ = *src++;
        }
}
                                      
boolean blockcompare(dst, src, n)
char *dst;
char *src;
long n;
{
        while(n--)              
                if(*dst++ != *src++)
                        return false;
        return true;
}

/*	Given an identifier reference we do the right thing based on
	its type.  If we are referencing a subprogram or package, we
	must swap it into memory before using it.  If in symbol mode
	(mainly for pp(x'sym) type stuff) we simply return with the
	result set to the symbol we were passed.  

	Types and subtypes are "typetype", and are simply returned.
	The hard stuff is objects or parameters.  See the big table
	below. */

long doident(f, mode, result)               
    fixp f;
    enum int_modes mode;
    typedval *result;
{
        if(f->fxf.class == func_id || f->fxf.class == proc_id)
                subp_swapin(f);
        else if(f->fxf.class == pack_id)
                package_swapin(f);

        if(mode == symbol_mode) {
                result->typ = symboltype;
                result->val.fptr = f;
                return;
        }

        switch(f->fxf.class) {
            case func_id:
		if (mode eq ovl_mode) {
			result->typ _ f->fxf.dyna->sy.func.type;
			/* value shouldn't matter, but just in case */
			result->val.nptr _ f->fx.func.int_form;
			break;
		}	/* else drop into next case */
            case proc_id:
                result->typ = functype;
                result->val.fptr = f;
                break;
            case type_id:
            case subtype_id:
                result->typ = typetype;
                result->val.tptr = f->fxf.dyna->sy.var.type;
                break;
            case pack_id:
                result->typ = packtype;
                result->val.fptr = f;
                break;          
	    case except_id:
		result->typ = excepttype;
		result->val.fptr = f;
		break;
            case obj_id:
            case param_id: {
 
                 /*
			This is a chart of return values for
			parameters.  The parameter can be an array or
			record (block), or a scalar (integer or enum or
			float), or can be either one passed by
			reference (out param).  We want to return
			either the value or the address of the value
			depending on the mode we are called in.  Note
			that for blocks, addr mode and eval mode are
			effectively the same.


                        mode: addr           eval
                        _____________________________________
                 block  |  addr            |  addr
                        |  (in value slot) |  (in value slot)
                        |                  |
                 scalar |  addr            |  value
                        |  (in value slot) |  (indirect thru value slot)
                        |                  |                 
                 ref    |  addr            |   value
                        |  (in value slot) |   (indirection in value slot)
                        |____________________________________    
                 */
 
                boolean is_ref_param = 
                    f->fxf.class == param_id and f->fx.var.param_is_out;
                switch(mode) {
                  case addr_mode:
		  case ovl_mode:
		      result->val.ptr = f->fxf.dyna->sy.var.value;
                    break;      
                  case eval_mode:
		     if (blocktype(f->fxf.dyna->sy.var.type))
			result->val.ptr = f->fxf.dyna->sy.var.value;
		     else 
			result->val.integer = *(f->fxf.dyna->sy.var.value);
                    break;
                  default:
		    if (mode eq decl_mode)
			error("Can't do that with that identifier");
				/* I think this happens from top level
				evals.  You can get this error by typing
				the identifier for an array at the top level */
                    system_error("bad mode (%d) in eval of ident", mode);
                }
                result->typ = f->fxf.dyna->sy.var.type;
                break;
            }
            default:
                system_error("bad class (%d) for identifier %s",
			f->fxf.class,
			f->fxf.name);
        }
}

/*	For docall, we have something arbitrarily complex on the right as the
	name of the routine we are to call, and an argument list.  We must
	evaluate the call var in the mode we were called in, lest we
	cause unwanted side effects.  The arguments are always evaluated
	first in "typeonly" mode, so that when we finally do eval them,
	we do it once in the correct mode (eval_mode or addr_mode).
*/

static	symtabp docall_save_currtab;
			/* contains value for abort_call below */

static	long docall_abort() 	/* pushed onto control-c stack; otherwise, when
			   checking for control-c's in compiled code, the
			   scoping is all wrong, and Boom!

			   This can work without stacking these because
			   presently once you're in compiled code, that's
			   it, and there is no more recursion at this
			   level.  That's subject to change, tho...
			*/
{	currtab _ docall_save_currtab;
}

long docall(p, mode, result)
    nodep p;
    enum int_modes mode;        
    typedval *result;
{                             
	register long i;
	long nargs;		/* number of arguments to subp */
	typedval lhs;		/* for getting type of call var */
	fixp fix;		/* fixp of subp we're calling */
	typep argtypes[MAXARGS];/* table of types of actual params */
	typep formtypes[MAXARGS];/* table of types of formal params */
	long argtab[MAXARGS];	/* table of eval'd actual params */
	boolean named_param;	/* true if we've seen a named parameter */
        fixp paramlist;		/* pointer to symbols for formals */
        register fixp formals;	/* used to traverse formal params */
        register nodep acts;	/* used to traverse actual params */
	nodep subp_int;		/* internal form for subprogram decls & body */
        long val;		/* used as temp for value of actual param */
        symp s, t;		/* Used to traverse symbol lists */
	symtabp oldscope;	/* saved scope of caller */
	symtabp newscope;	/* saved scope of callee */
        fixp owner;		/* points to owner of subprogram fixform */
        boolean pushed_pack = false;	/* true if subp was located in a
				package not on the stack and we had to push
				the package on the stack first (which means
				we pop if off when we're done. */
	symtabp tab;		/* hash table used to match actuals and
				formals */
	struct symtabform loctab;/* local table: this is an optimization
				so that we don't zalloc a new symbol
				table everytime.  Instead, we make it
				part of the local C stack frame. */
	fixp lochash[16];	/* size of local hash table */
	boolean is_a_slice _ false;	/* set to true if argument to
					call is a range */

	/* setup to save right scope environment if exited prematurely */
	symtabp save_currtab _ currtab;
				/* stack old currtab save */

	docall_save_currtab = currtab;
				/* save for control-c interrupt */

	/* some attributes have call nodes as the top level (see the
	great comment at doattr2), so they get handled via docall.  We
	just pass them to doattr2. */

        if(p->nd.call.var->ndf.op == nop_quote) {
       	        /* something like:  color'succ(red)  */
               	doattr2(p, result);
                return;
       	}

	/*	Evaluate types of arguments statically.  */

	i _ 0;
	acts = p -> nd.call.args;

	while(acts != null) {
		if(acts->ndf.op != nop_arg) {
			is_a_slice _ true;
			if (acts -> ndf.next neq null)
			  error("slices must have one argument only");
			/* i is 0, so nargs will be set to 0, and the */
			/* code for get and put below will be avoided */
			break;
		}
		argtypes[i] = typeonly(acts->nd.arg.exp);
		if (not argtypes[i++]) {
		  putf("Can't determine type of argument ");
		  dopp(acts->nd.arg.exp,current_output(),true,false);
		  error(null);
		}
		acts = acts -> ndf.next;
	}

	nargs _ i;		/* number of arguments to subp */

	/* before anything, see if we are eval'ing "put" or "get", and append
	the appropriate ending to it, to simulate overloading for these
	so as to make the P.I. very happy. So, put(1) => putint(1)... This
	won't catch something like uci_ada_functions.put(5);  */

	if ((nargs > 0) and	/* get, put must have at least one arg */
	    (p->nd.call.var->ndf.op eq nop_ident) and
	    ((p->nd.call.var->nd.ident.id eq intern("put")) or
	     (p->nd.call.var->nd.ident.id eq intern("get")))) {
		char *ending;
		char start[20];
		if (compatible(strtype,argtypes[0])) ending _ "str";
			/* strtype is an unconstrained type, and must be
			the first argument to compatible. */
		 else if (compatible(argtypes[0],symboltype)) ending _ "sym";
		 else if (compatible(argtypes[0],inttype)) ending _ "int";
		 else if (compatible(argtypes[0],chartype)) ending _ "chr";
		 else if (compatible(argtypes[0],realtype)) ending _ "real";
		 else error("Unknown argument type for %s",
			p->nd.call.var->nd.ident.id);
		strcpy(start,p->nd.call.var->nd.ident.id);
		strcat(start, ending);
		fix  _ lookup(currtab, intern(start), null);
		if (not fix) system_error("%s version of %s not found!", start,
			p->nd.call.var->nd.ident.id);
		lhs.val.fptr _ fix;
		if (mode eq symbol_mode) lhs.typ _ symboltype;
		else lhs.typ _ functype;
	}
        else eval(p->nd.call.var, mode, &lhs, null);

	/* if eval'ing in symbol mode or ovl_mode, we've done all we need
	to do, so we can simply return what we've eval'd. */

	if (mode eq symbol_mode or mode eq ovl_mode) {
		/* If we're doing an array, return the type of elements
		of the array; but if we're doing a function, then the
		result type of the function is sufficient. */
		if (lhs.typ -> tyf.class eq con_arrayform)
			/* I am assuming a constrained array always points
			to an underlying unconstrained type.  It seems to
			work this way in doindex. */
			result->typ _ lhs.typ->ty.con_array.base_type->
					ty.un_array.ele_type;
		else result -> typ _ lhs.typ;
		result -> val _ lhs.val;
		return;
	}

	fix _ lhs.val.fptr;	/* will be used later if this is a subp */
	
	/* If the subprogram name is really a type name then the
	   subprogram call is really a type conversion.  Call do_type_conv
	   to handle this case. */

	if (lhs.typ eq typetype) {	/* type conversion: width(5) */
		if (is_a_slice)
		  error("type conversion can't take range argument");
		do_type_conv(lhs.val.tptr,
				mode,
				p->nd.call.args,
				result);
		return;
	}

	/* could be an object or parameter that is a constrained array,
	so let doindex take care of that... */

	if (lhs.typ neq functype) {	/* must be array reference */
		typep arrtype _ lhs.typ;
		doindex(arrtype,
			lhs.val.integer,	/* port: was lhs.val, a word union */
			mode,
			p->nd.call.args,
			result);
		return;
	}

	if (is_a_slice)
		error("illegal slice");

	/* Otherwise must be a subprogram; fix is already set to be
	the symbol value (fixed part) */

        if(fix->fxf.class != func_id && fix->fxf.class != proc_id)
            system_error("call form not a subprogram; yikes!");

	if (p->ndf.op eq nop_proc_call) {
	  if (fix->fxf.class neq proc_id) 
	    error("Attempt to call function like a procedure");
	}
	else if (fix->fxf.class neq func_id)
	  error("Attempt to use procedure like a function"); 

        subp_swapin(fix);

	/* set "subp_int" to point to the subp node for the subprogram.  
	This is	in func.intform. */

        subp_int = (nodep)fix->fx.func.int_form;
	if (subp_int == null) {
	  if (fix->fxf.class == func_id)
	    error("function is missing its body and cannot be executed.");
	  else
	    error("procedure is missing its body and cannot be executed.");
	}

	/* evaluate the formal parameters into a table.  */

	tab _ &loctab;	/* optimization to use local table in C stack frame */
			/* old code: tab _ newtab(16,null); */
	tab->symtab_tab _ lochash;
	{long i; for(i _ 0; i < 16; i++) lochash[i] _ null;};
			/* zero out hash table */
	tab->symtab_size _ 16;
	tab->symtab_owner _ null;
	tab->symtab_static_link = currtab;

	/* The table is passed on the stack to nop_param */

  	parameter_table = tab;  /* global for nop_param */
	paramlist _ (fixp)eval(subp_int->nd.subp.spec, 
				decl_mode, &dummy, null);

	/* Now match the formals and the actuals: the actuals must have
	the positional parameters first followed by any named parameters.
	It is o.k. if an actual parameter is omitted if there is a default
	expression for it. */

	acts _ p->nd.call.args;
	named_param _ false;
	formals _ paramlist;

	/* The intform field will contain the internal form for the
	actual argument; this could be either from the actual
	parameters or from a default formal parameter.  The boolean
	param_has_value is true if it is from a default.  We have to
	know this because actual parameters evaluate in the scope of
	the caller and defaults evaluate in the scope of the callee.
	Note:  we can't use the existing table for the subprogram
	because it might be full of important stuff if this is a
	recursive call. */

	i _ 0;
	while (acts) {
		char *id;
		fixp f;
		symp s;

		f _ formals;		/* current formal param in list */

		if (not f) 
		    error("Too many arguments to subprogram");

		if (acts->nd.arg.name) {
			named_param _ true;
			id _ acts->nd.arg.name;
			f _ lookuptab(tab, id, null);
			if (not f)
			  error("Can't associate named param %s in subp", id);
		}
		else if (named_param) 
			error("Positional params must precede named params");
		
		f->fx.var.int_form _ acts->nd.arg.exp;
		f->fx.var.param_has_value _ false;
		f->fxf.dyna->sy.var.type _ argtypes[i++];
		typecheck(f->fx.var.param_base_type, f->fxf.dyna->sy.var.type);
		acts _ acts->ndf.next;
		formals _ formals->fx.var.param_link;
	}

	/* now create new context for evaluation, but save the old one
	for when we evaluate the actuals */

	oldscope _ currtab;
	newscope _ subp_int->nd.subp.table;

	/* now evaluate arguments */

	formals _ paramlist;

        while(formals) {
		typedval arg;
		typep typ;

		if (formals->fx.var.int_form == null)
		    error("no value for formal parameter: %s in call to %s",
			   formals->fxf.name, fix->fxf.name);

		typ = formals->fx.var.param_base_type;

		/* pass eval the expected typ for the expression being
		evaluated in case something in it is overloaded. 
		Test if should be by addr or value.  Also, if has_value
		is set, we eval in the environment of the caller. */

		if (not formals->fx.var.param_has_value)
			currtab _ oldscope;
		else currtab _ newscope;

                if(blocktype(typ) or formals->fx.var.param_is_out) { 
			eval(formals-> fx.var.int_form, addr_mode, &arg, typ);
			formals-> fxf.dyna->sy.var.value _ arg.val.ptr;
                }
		else {
			eval(formals-> fx.var.int_form, eval_mode, &arg, typ);
			rangecheck(typ, &arg);
			*(formals->fxf.dyna-> sy.var.value) _ arg.val.integer;
                }

		/* unconstrained formals take the type of the 
		constrained actual */

                if(typ->tyf.class == un_arrayform)
		    formals->fxf.dyna->sy.var.type = arg.typ;
			       /* type of actual arg */
                else formals->fxf.dyna->sy.var.type = typ;

		formals _ formals->fx.var.param_link;
        }

	/* Make sure we're in the scoping environment of the callee. */

	currtab _ newscope;

	/* for compiled code, we load the argument (addr's or values)
	into an array that gets pushed on the C stack by "call.a". If the
	actual parameter is an array, then we have to create a little
	array descriptor and make it point at the array so the compiled
	code (which can't look at symbols (yet)) can get the size of the
	array and what not. */

        if(fix->fx.func.code) {        /* compiled C or Ada */ 
	    long argtab[MAXARGS];
	    long argcount _ 0;

	    for (formals _ paramlist; 
		 formals; 
		 formals _ formals -> fx.var.param_link){
		long *val _ formals -> fxf.dyna -> sy.var.value;
                arrayp p;

		if (formals->fx.var.param_base_type->tyf.class eq un_arrayform) {
		    typep typ;
		    /* an arrayp is an array descriptor */
                    p = (arrayp)zalloc(sizeof(*p), false);
                    p->arr_base = val;
		    typ = formals->fxf.dyna->sy.var.type;
		    typ = typ->ty.con_array.index_list;
		    typ = typ->ty.index.range;
		    p -> arr_first _ typ->ty.srange.lbound;
		    p -> arr_last _ typ->ty.srange.ubound;
		    val _ (long *)p;
                }
		if (blocktype(formals->fxf.dyna->sy.var.type) or 
		    formals->fx.var.param_is_out)
		    argtab[argcount++] _ (long)val;
		else
		    argtab[argcount++] _ *val;
            }
	    /* As a kindness to "where" and "break" we pop the current
	    context.  This horribleness makes references to currtab act
	    like dynamic scoping happened. */

	    currtab _ oldscope;

	    /* coerce result to be of appropriate type */
            result->typ = fix->fxf.dyna->sy.func.type;

		/* (should rangecheck result?) */

	    /* call compiled function */
	    pushfunc(docall_abort);
	    result->val.integer = call(&argtab[argcount],
				       argcount,
				       fix->fx.func.code);  /* asm function */
	    popfunc();
            /* restore correct scoping context */
	    {long i; for (i _ 0; i < 16; i++) freelist(lochash[i]);} 
		/* free symps allocated in the local table; this is part
		of the optimization of not zalloc'ing a local table. */
	    currtab _ newscope;
        }                   
        else if(subp_int = (nodep)fix->fx.func.int_form) {
				/* interpreted code */

  /* The following section does the actual calling of the subprogram.
     It pushes a pointer to the subp node of the subprogram on the interpreter
     stack.  This has two purposes: 1) it makes the backtrace more informative
     since the subprogram appears immediately above it's call and 2) the
     Castor macro expansion facility uses the stack to find the parent of
     any macro call.  The parent of the first item in the declaration list
     and also in the body is the subp node.

     The second thing that happens is that an entry for the return stack
     is built.  This stack is used for handling exceptions, return statements,
     and exit statements.  The stack entry is actually a structure local
     to this function, so that if throw (longjmp) is used to get back to
     an earlier level, the return stack is effectively popped and the
     storage (on the stack) is released.  Most of the items in the stack
     entry are obvious.  The in_decls flag is used by the exception handlers
     to tell whether an exception was raised during elaboration of the
     declarations or during execution.  The exception field is used by
     the exception handling code to tell if an exception handler is being
     executed and if so, which one (this is needed for the raise statement
     with no exception name).

     The calls to catch can catch two types of throws.  The return statement
     throws a 1 and the exception hander code throws a 2.  If a 2 is
     thrown, a pointer to the exception handler to use is in the global
     variable handler.		*/

	    struct retstkform rets;
	    long exception;

	    /* enter actual argument values into the symbol table for 
	  	this subprogram. */

	    for(formals _ paramlist; 
		formals; 
		formals _ formals->fx.var.param_link) {
			formals->fxf.dyna->syf.level = 999999;	/* KLUDGE */
				/* otherwise poplevel won't pop params */
			enter(currtab, formals, formals->fxf.dyna);
	    }
		/* note: the symbol is now in TWO tables: "tab" and the one
		for the subprogram, so we can't free them until the subp
		returns back to here. */

	    /* save current spot on the backtrace stack so we can prune 
		it back when we return from the subp */

	    if (intsp++ == intstk_end) {/* reserve space on stack */
        	putf ("\nInterpreter stack overflow\n");
		intsp = intstk;
	    }

	    *intsp = subp_int;
	    if (counting) increment_node_counter(subp_int);

	/* set mark so we can tell in nothing returned by function */
	/* PAR 22-Aug-83 */
	    result -> typ = null;

	/* elaborate locals and execute body */

	    doscope(&rets, subp_int, 
		    subp_int->nd.subp.context,
		    subp_int->nd.subp.decl,
		    subp_int->nd.subp.bodyx,
		    result);

	/* check if we called a function that it did indeed return a value */
	/* PAR 22-Aug-83 */

	    if(p->ndf.op == nop_call && result -> typ == null)
		sysraise(program_error,
			 "Program Error: no value returned by function");

	    /* check result for conformance.  Maybe should do this for
	    compiled code too... */

	    currtab = newscope;
            rangecheck(fix->fxf.dyna->sy.var.type, result);

	    intsp--;		/* pop the interpreter stack */
	}
        else error("no code for function");

	/* note: still need to free fixp's used by lochash. */
	
	poplevel(currtab, stack_level);
	currtab = save_currtab;
}

boolean idlistmatch(p1, p2)
nodep p1, p2;
{
        if(p1 == null && p2 == null)
                return true;
        if(p1 == null || p2 == null)
                return false;
        if(p1->nd.id_decl.id != p2->nd.id_decl.id)
                return false;
        return idlistmatch(p1->ndf.next, p2->ndf.next);
}

boolean matchaux(p1, p2) /* compare two subp specs. needs work */
nodep p1, p2;
{
        nodep t1, t2;
        if(p1 == null && p2 == null)
                return true;
        if(p1 == null || p2 == null)
                return false;
        if(not idlistmatch(p1->nd.param_decl.id_list, p2->nd.param_decl.id_list))
                return false;
        t1 = p1->nd.obj_decl.subtypex;
        t2 = p2->nd.obj_decl.subtypex;
        if(t1->ndf.op != nop_ident || t2->ndf.op != nop_ident)
                error("overloaded function must have simple types");
        if(t1->nd.ident.id != t2->nd.ident.id)
                return false;
        return matchaux(p1->ndf.next, p2->ndf.next);
}

/* this routine is only called indirectly by lookup */
boolean argmatch(f, p2)
    fixp f;
    nodep p2;
{
    return matchaux (f -> fx.func.int_form -> nd.subp.spec, p2);
}

long doblock(p)
    nodep p;
{
    struct retstkform rets;
    long exception;
    typedval *dummy;
    symtabp save_currtab = currtab;

    if (p->nd.subp.table eq null) {
	symtabp tab = newtab(32L,null);

	tab->symtab_static_link = currtab;
	p->nd.subp.table _ tab;
    }
    currtab = p->nd.subp.table;

	/* elaborate declarations and execute body */

    doscope(&rets, p, p->nd.subp.context, 
		p->nd.subp.decl, 
		p->nd.subp.bodyx, 
		dummy);

    poplevel(currtab, stack_level);
    currtab = save_currtab;
}

typep mkarrtype();      /* forward declaration, should re-order */

long doindex(typ, val, mode, p, result)
    typep typ;		/* typep for the array */
    long val;		/* base of the array */
    enum int_modes mode;	/* evaluation mode */
    nodep p;		/* internal form of indices */
    typedval *result;	/* result to pass back to eval */
{
    typedval args[MAXARGS];	/* holds indices */
    nodep index;		/* used to flip through indices */
    word addr;
    typep lsttyp;
    typep eletyp;
    long     n = 0;		/* counts number of indices */
    long     off = 0;		/* offset into array */

    if (p -> ndf.op eq nop_range)
				/* for a slice there is no arg node (some
				   may consider this a parsing flaw)
				   [However some others may not.
				   Nevertheless, if you really want one,
				   I`ll accomodate you.(fpt)] so this arg
				   is evaluated specially. The resulting
				   typedval will be of the form (typ =>
				   typetype, val.tptr => rangetype). */
	eval (p, mode, &args[0], null);
    else
	for (index _ p; index; index _ index -> ndf.next) {
	    if (n >= MAXARGS)
		error ("More than %d array indices! (Amazing)",
			MAXARGS);
	    else
		eval (index -> nd.arg.exp, eval_mode, &args[n++], null);
	}
    n = 0;
 /* for looping through eval`d args */
 /* 	get down to the base type, going to access types, subtypes, and
    derived types. */
    while (typ -> tyf.class eq accessform || typ -> tyf.class eq subtypeform
	    || typ -> tyf.class eq derivedform) {
	switch (typ -> tyf.class) {
	    case accessform: 
		addr.integer = *addr.ptr;
		if (addr.ptr == null)
		    sysraise (constraint_error,
		     "constraint_error: reference through null access value");
		typ = typ -> ty.access.targettype;
		break;
	    case subtypeform: 
		typ = typ -> ty.subtype.basetype;
		break;
	    case derivedform: 
		typ = typ -> ty.derived.parenttype;
		break;
	}
    }

    if (typ -> tyf.class != con_arrayform)
	error ("Indexed object must be constrained array");

    if (args[0].typ == typetype) {/* must be a slice */
	typep index = args[0].val.tptr;
	typep baseindex = typ -> ty.con_array.index_list -> ty.index.range;
	typep basetype = typ -> ty.con_array.base_type;
	typep eletype = basetype -> ty.un_array.ele_type;

    /* make sure index is discrete range type */
    /* make sure index contained in baseindex */
	off = index -> ty.srange.lbound - baseindex -> ty.srange.lbound;
	off *= eletype -> tyf.size;
	result -> val.integer = val + off;
	result -> typ = mkarrtype (basetype, index);
	return;
    }
    eletyp = typ -> ty.con_array.base_type -> ty.un_array.ele_type;
    lsttyp = typ -> ty.con_array.index_list;
    while (lsttyp) {
	typep rngtyp;
	typep nxttyp;
	long     nxtsize;
	long     inx;

	rngtyp = lsttyp -> ty.index.range;
	nxttyp = lsttyp -> ty.index.next;
	nxtsize = (nxttyp ? nxttyp -> tyf.size : 1);
	rangecheck (rngtyp, &args[n]);
	inx = args[n].val.integer;
	inx -= rngtyp -> ty.srange.lbound;
	inx *= eletyp -> tyf.size * nxtsize;
	off += inx;
	n++;
	lsttyp = nxttyp;
    }
    addr.integer = val + off;
    if (mode == eval_mode) {
	if (blocktype (eletyp))
	    result -> val = addr;
	else
	    if (eletyp -> tyf.size == sizeof (long))
		result -> val.integer = *addr.ptr;
	    else
		if (eletyp -> tyf.size == 1)
		    result -> val.integer = *(char *) addr.ptr;
		else
		    system_error ("bug in doindex");
    }
    else
	if (mode == addr_mode)
	    result -> val = addr;
	else
	    system_error ("bad array mode");

    result -> typ = eletyp;
}

 /* do_type_conv handles type conversion of the form type(value).  It
    only does conversions between compatible subtypes and between a
    derived type and its parent type.  It also needs to handle numeric
    conversion */

long do_type_conv(typ, mode, p, result)
    typep typ;
    enum int_modes mode;
    nodep p;
    typedval *result;
{
    typep basetype, parenttype;
    typedval n, arg;

    eval(p->nd.arg.exp, mode, &arg, null);

    if (p->ndf.next)  error("Type conversions take only one argument");

    if (mode != eval_mode )
	if (mode eq addr_mode)
		error("Attempt to assign to type conversion");
	else system_error("Invalid mode for type conversion");

    if (compatible(arg.typ, typ)) {
	checkrange(typ, &arg);
        result->val = arg.val;
        result->typ = typ;
	return;
    }
    basetype = base(arg.typ);
    if (basetype->tyf.class == derivedform) {
	parenttype = basetype->ty.derived.parenttype;
	if (compatible(parenttype, typ)) {
	    checkrange(typ, &arg);
	    result->val = arg.val;
	    result->typ = typ;
	    return;
	}
    }
    basetype = base(typ);
    if (basetype->tyf.class == derivedform) {
	parenttype = basetype->ty.derived.parenttype;
	if (compatible(parenttype, arg.typ)) {
	    checkrange(typ, &arg);
	    result->val = arg.val;
	    result->typ = typ;
	}
    }
    else error("Invalid type conversion");
}

long subp_name(id, class)    /* called by pragmas */
    char *id;
    enum symbolclasses class;
{
    register symp sym;
    register fixp fix;

    fix = newfix(funcfixform, class);
    sym = newsym (funcsymbolform, class);
    fix -> fxf.name = id;
    enter(currtab, fix, sym);
}

/*	caveat: in the following code, currtab is changed.  If we put
	something to check for control-c's while parsing, then we must
	also put in an entry in the control-c exception stack to trap
	here and put currtab back to it's correct value.  (shw)	*/

long subp_swapin(fix)
    fixp fix;
{
    char    name[128];
    symtabp savetab = currtab;	/* save for when we change scope for read in */
    if (fix -> fx.func.int_form) return;
    savename (name, fix);
    currtab = fix -> fxf.static_link;
    include (name, true);
    currtab = savetab;		/* and back to original spot */
}

long package_name(id)        /* called by pragma package_name */
    char *id;
{
    register symp sym;
    register fixp fix;

    fix = newfix(packfixform, pack_id);
    sym = newsym (packsymbolform, pack_id);
    fix -> fxf.name = id;
    enter(currtab, fix, sym);
}

long package_swapin(fix)
    fixp fix;
{
    char    name[128];
    symtabp savetab = currtab;		/* same deal as for subp_swapin */
    if (fix -> fx.pack.int_form) return;
    savename (name, fix);
    currtab = fix->fxf.static_link;
    include (name, true);
    currtab = savetab;
}

static boolean writeout;		/* hack */

long dopackspec(p)                                        
    nodep p;
{
    register symp sym;
    register fixp fix;
    register symtabp tab;
    struct retstkform   rets;
    symtabp save_currtab = currtab;

    writeout = false;
    fix = lookuptab (currtab, p -> nd.packagex.name, null);
    if (fix == null) {
	fix = newfix(packfixform, pack_id);
	sym = newsym (packsymbolform, pack_id);
	fix -> fxf.name = p -> nd.packagex.name;
	enter(currtab, fix, sym);
	writeout = true;
    }
    else
	sym = fix -> fxf.dyna;
    /* make sure possibly re-used fix and symp have right class in case
	of redefinition.
    */
    fix -> fxf.class = pack_id;
    /* fill in internal form */
    fix -> fx.pack.int_form = p;

    if (p->nd.packagex.table == null) {
        tab = newtab (32L, fix);
	tab -> symtab_static_link = currtab;
        p -> nd.packagex.table _ tab;
    }
    currtab = p->nd.packagex.table;

	/* elaborate declarations */

    doscope(&rets, p, 
	p->nd.packagex.context, 
	p->nd.packagex.decl, 
	null, dummy);

    currtab = save_currtab;
}

long dopackbody(p) 
    nodep p;
{
    register fixp fix;		/* fixp of package symbol */
    register symtabp tab;	/* body symbol table for package */
    struct retstkform   rets;	/* return stack entry */
    typedval *dummy;		/* packages don't return a value so ignore
				   this argument to doscope */
    symtabp save_currtab = currtab;	/* currtab of surrounding scope */

    fix = lookuptab (currtab, p -> nd.packagex.name, null);
    if (fix == null)
	error ("no package %s", p -> nd.packagex.name);
    fix -> fx.pack.body_int_form = p;

    if (p->nd.packagex.table == null) {
        tab = newtab (32L, fix);
        tab -> symtab_static_link = fix->fx.pack.int_form->nd.packagex.table;
        p -> nd.packagex.table _ tab;
    }
    currtab = p->nd.packagex.table;

	/* elaborate declarations and execute body */

    doscope(&rets, p, 
	p->nd.packagex.context,
	p->nd.packagex.decl,
	p->nd.packagex.bodyx, dummy);

    currtab = save_currtab;

    if (writeout && currtab == usrtab)
	pack_writeout(fix);
}

typep mkrangetype(basetype, lower, upper) /* should call this in tyint.c */
typep basetype;
long lower, upper;
{
        register typep t = newtype(srangeform);

        t->tyf.size = sizeof(long);
        t->ty.srange.lbound = lower;
        t->ty.srange.ubound = upper;
        t->ty.srange.basetype = basetype;
        return t;
}

long doattr(p, result)
nodep p;
typedval *result;
{
        typedval t;
        char *s;
        typep typ;
	boolean unconstrained _ false;

        /* static id pointers which are interned once */
        static boolean interned;  /* = false */
        static char *first_id, *last_id;
        static char *range_id, *length_id;
        static char *symbol_id;

        if(not interned) {
                first_id = intern("first");
                last_id = intern("last");
                range_id = intern("range");
                length_id = intern("length");
                symbol_id = intern("sym");
                interned = true;
        }

        s = p->nd.selectx.field;
        if(s == symbol_id) {
                eval(p->nd.selectx.var, symbol_mode, result, null);
		result->typ _ symboltype;
                return;
        }
        eval(p->nd.selectx.var, eval_mode, &t, null);
                        /* must side effects be done? */

        if(t.typ == typetype)
                typ = t.val.tptr;   /* if type is typetype, value is a type too */
        else typ = t.typ;

        if(typ->tyf.class == con_arrayform or
	   typ->tyf.class eq un_arrayform)
                typ = typ->ty.con_array.index_list->ty.index.range; 
			/* assumes 1-dim */

        result->typ = typ;

        switch(typ->tyf.class) {
                case enumform:
                case srangeform:
                case intform:
                        if(s == last_id)
                                result->val.integer = typ->ty.srange.ubound;
                        else if(s == first_id)
                                result->val.integer = typ->ty.srange.lbound;
                        else if(s == length_id) {
                                result->val.integer =
                                  typ->ty.srange.ubound - typ->ty.srange.lbound + 1;
                                result->typ = inttype;
                        }
                        else if(s == range_id) {
                                result->val.tptr = typ;
                                result->typ = typetype;
                        }
                        else error("unknown attribute");
                        break;
                default:
                        error("bad type to attribute");
        }
}

long doattr2(p, result)
nodep p;
typedval *result;
{
/*      When this procedure is called, p looks like:            */
/*                      nop_call                                */
/*                     /        \                               */
/*                 nop_quote    id(red)                         */
/*                /     \                                       */
/*          id(color)  "succ"                                   */
/*                                                              */
/*      if the source is:  color'succ(red)                      */

        typedval t;
        register char *s;
        register typep typ;

        static boolean interned;
        static char *pred_id, *succ_id;
        static char *pos_id, *val_id;
        static char *image_id, *value_id;

        if(not interned) {
                pred_id = intern("pred");
                succ_id = intern("succ");
                pos_id = intern("pos");
                val_id = intern("val");
                image_id = intern("image");
                value_id = intern("value");
                interned = true;
        }
        eval(p->nd.call.var->nd.selectx.var, eval_mode, &t, null);
        if(t.typ == typetype)
                typ = t.val.tptr;
        else error("type identifier required");
        switch(typ->tyf.class) {
                case enumform:
                case srangeform:
                case intform:
                        break;
                default:
                        error("bad type in attribute");
        }
        eval(p->nd.call.args, eval_mode, result, null);
        /* should check for exactly one arg */
        /* here, this could be trouble since result[1] doesn't exist */
        s = p->nd.call.var->nd.selectx.field;
        if(s == pred_id) {
                typecheck(result->typ, typ);
                if(result->val.integer == typ->ty.srange.lbound)
                    sysraise(constraint_error, 
			"constraint_error: value out of range in 'pred");
                result->val.integer--;
                result->typ = typ;
        }
        else if(s == succ_id) {
                typecheck(result->typ, typ);
                if(result->val.integer == typ->ty.srange.ubound)
                    sysraise(constraint_error,
			"constraint_error: value out of range in 'succ");
                result->val.integer++;
                result->typ = typ;
        }
        else if(s == pos_id) {
                typecheck(result->typ, typ);
                result->typ = inttype;
        }
        else if(s == val_id) {
                typecheck(result->typ, inttype);
                result->typ = typ;
        }
        else if(s == image_id) {
                char buf[128];
                printimage(buf, result->val.integer, typ);
                dostring(buf, result);
        }
        else error("bad attribute");
}

/* this function is no longer used */
nodep mk_constrained_string_node(n) long n; {
/* this routine returns an internal form which pretty prints as:        */
/*      string(1..n)                                                    */
/* where n is the argument to this function                             */
        /* these can be static since they're always the same */
        static struct node stringid;
        static struct node lowexp;
        /* these must be built each time */
        nodep upexp, range, call;

/* the following initializations could be done staticly if someone 	*/
/* can figure out how to write an initializer to a union type (you	*/
/* have to be able to tell the compiler which varient you are trying	*/
/* to initialize)	- fpt						*/

	stringid.ndf.op = nop_ident;
	stringid.ndf.ag = stringid.ndf.next = null;
	lowexp.ndf.op = nop_int;
	lowexp.ndf.ag = lowexp.ndf.next = null;
	lowexp.nd.integer.val = 1;
	lowexp.nd.integer.prval = null;
		
        if(stringid.nd.ident.id == null)
                stringid.nd.ident.id = intern("string"); /* do only once */
                                                   /* (its static) */
        upexp = newnode(integer_node, nop_int);
        upexp->nd.integer.val = n;
        range = newnode(range_node, nop_range);
        range->nd.rangex.left = &lowexp;
        range->nd.rangex.right = upexp;
        call = newnode(call_node, nop_call);
        call->nd.call.var = &stringid;
        call->nd.call.args = range;
        /* this internal form is the same erroneous one the parser builds */
        return(call);
}

typep mkarrtype(basetype, range)
typep basetype, range;
{
        register typep a = newtype(con_arrayform);
        register typep i = newtype(indexform);
        long len = rangelength(range);

        i->ty.index.range = range;
        i->tyf.size = len;
        a->ty.con_array.index_list = i;
        a->ty.con_array.base_type = basetype;
        a->tyf.size = basetype->ty.un_array.ele_type->tyf.size * len;
        return a;
}

long dostring(s, result)
char *s;
typedval *result;
{
        long len = strlen(s);
        long *p = (long *)zalloc(len * sizeof(long), false);
        register long i;

        result->typ = mkarrtype(strtype, mkrangetype(inttype, 1L, len));
        for(i = 0; i< len; i++)
                p[i] = s[i]&0377;
        result->val.ptr = p;
}

long catenate(l,r,result)
    typedval l,r,*result;
{
    register char  *p,
                   *g;
    register long    len;
    long     low;

    if (l.typ -> tyf.class == con_arrayform) {
	if (r.typ -> tyf.class == con_arrayform) {
	    if (l.typ -> ty.con_array.base_type !=
		    r.typ -> ty.con_array.base_type ||
		    !compatible (l.typ -> ty.con_array.index_list -> 
				ty.index.range,
			r.typ -> ty.con_array.index_list -> ty.index.range))
		error ("Type clash");

		/* This section catenates two arrays.  The length of the
		   result is the sum of the lengths of the  lengths of the
		   two arrays.  The lower bound is the 'first element of the
		   index type of the two arrays. */
	    low = l.typ->ty.con_array.index_list->
			    ty.index.range->ty.srange.basetype->
		 	    ty.srange.lbound;
	    result -> typ = mkarrtype (		/* build the type of the
						   new array */
		    l.typ -> ty.con_array.base_type,
		    mkrangetype (
			l.typ -> ty.con_array.index_list ->
			    ty.index.range -> ty.srange.basetype,
			low,
			low + l.typ -> ty.con_array.index_list -> tyf.size +
			r.typ -> ty.con_array.index_list -> tyf.size - 1));
	    p = (char *) zalloc (result -> typ -> tyf.size); 
				/* allocate storage for the array */
	    result -> val.ptr = (long *) p;
	    g = (char *) l.val.ptr;	/* copy the left operand */
	    for (len = 0; len < l.typ -> tyf.size; len++)
		*p++ = *g++;
	    g = (char *) r.val.ptr;	/* copy the right operand */
	    for (len = 0; len < r.typ -> tyf.size; len++)
		*p++ = *g++;
	}
	else
	    if (l.typ -> ty.con_array.base_type -> ty.un_array.ele_type ==
		    r.typ) {
		/* This section catenates a single element onto the
		   end of an array.  The lowest index is the 'first element
		   of the index type of the array */
		low = l.typ->ty.con_array.index_list->ty.index.
			     range->ty.srange.basetype->ty.srange.lbound;
		result -> typ = mkarrtype (	/* construct the type of the 
						   result */
		        l.typ -> ty.con_array.base_type,
			mkrangetype (
			    l.typ -> ty.con_array.index_list ->
			        ty.index.range -> ty.srange.basetype,
			    low,
			    low + l.typ -> ty.con_array.index_list -> 
				tyf.size));
			/* allocate space for the result */
		p = (char *) zalloc (result -> typ -> tyf.size);
		result -> val.ptr = (long *) p;
		g = (char *) l.val.ptr;		/* copy the array */
		for (len = 0; len < l.typ -> tyf.size; len++)
		    *p++ = *g++;
		if (blocktype (r.typ)) {	/* now copy the single element */
		    g = (char *) r.val.ptr;
		    for (len = 0; len < r.typ -> tyf.size; len++)
			*p++ = *g++;
		}
		else
		    *(long *) p = r.val.integer;
	    }
	    else
		error ("invalid type in catenate");
    }
    else
	if (l.typ == 
	    r.typ -> ty.con_array.base_type -> ty.un_array.ele_type) {
		/* This section catenates a single element onto the
		   beginning of an array.  The lowest index is the 'first 
		   element of the index type of the array */
	    low = r.typ->ty.con_array.index_list->ty.index.
			 range->ty.srange.basetype->ty.srange.lbound;
	    result -> typ = mkarrtype (		/* construct the type of
						   the result */
		    r.typ -> ty.con_array.base_type,
		    mkrangetype (
			r.typ -> ty.con_array.index_list ->
			ty.index.range -> ty.srange.basetype,
			low,
			low + r.typ -> ty.con_array.index_list -> tyf.size));
			/* allocate space for the result */
	    p = (char *) zalloc (result -> typ -> tyf.size);
	    result -> val.ptr = (long *) p;
	    if (blocktype (l.typ)) {		/* copy the single element */
		g = (char *) l.val.ptr;
		for (len = 0; len < l.typ -> tyf.size; len++)
		    *p++ = *g++;
	    }
	    else {  /* anyone have a better idea? */
		register long *pp = (long *)p;
		*pp++ = l.val.integer;
		p = (char *)pp;
	    }
	    g = (char *) r.val.ptr;		/* now copy the array */
	    for (len = 0; len < r.typ -> tyf.size; len++)
		*p++ = *g++;
	}
	else
	    error ("invalid type in catenate");
}
	  

long agg_eval(p, result, expected_type)
nodep p;
typedval *result;
typep expected_type;
{
	register long len;
	register typep res_type;
	register typep ele_type;
	long *agg_space;
	long agg_len = 0;
	typedval a;

	switch(expected_type->tyf.class) {
	    case con_arrayform:		/* constrained array */
		len = expected_type -> ty.con_array.index_list -> tyf.size;
		/* size field of index type is length of index range */
		/* this code assumes one-dimensional array */
		ele_type = expected_type -> ty.con_array.base_type
				-> ty.un_array.ele_type;
		res_type = expected_type;
		break;
	    case un_arrayform: {	/* unconstrained array type */
		register nodep q;
		len = 0;	/* compute length from int. form. */
		for(q = p; q != null; q = q -> ndf.next)
		    len++;
		ele_type = expected_type -> ty.un_array.ele_type;
		res_type = mkarrtype(expected_type,
				     mkrangetype(expected_type
						  -> ty.un_array.index_list
						  -> ty.index.range,
						 1L,
						 len)); 
			/* above range should start at min of type */
		break;
	    }
	    case recordform:
		error("record aggregates unimplemented");
	    default:
		error("illegal type for aggregate");
	}
	/* now that we know length and result type, process aggregate */

	agg_space = (long *)zalloc(len * ele_type -> tyf.size, false);

        while(p != null) {
                if(p->ndf.op != nop_agg_comp)
                        system_error("ill-formed aggregate internal form");
                if(p->nd.agg_comp.choice != null)
                        error("choices for aggregates, unimplemented");
                eval(p->nd.agg_comp.expr, eval_mode, &a, null);
		typecheck(ele_type, a.typ);
		if(agg_len > len)
			error("aggregate too long");
		agg_space[agg_len++] = a.val.integer;	/* block types? */
                p = p->ndf.next;
        }
	if(agg_len != len)
		error("aggregate too short");
	result->typ = res_type;
	result->val.ptr = agg_space;
}
