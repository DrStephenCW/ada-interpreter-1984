static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/casint.v  Revision 1.7  84/01/12  16:41:48  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/casint.v $
 * Revision 1.7  84/01/12  16:41:48  ada
 * same as 1.6.4.1
 * 
 * Revision 1.6.4.1  84/01/06  16:12:30  tadman
 * Changes for static symbol tables.  The symbol tables were made static
 * over a year ago, but castor was never updated until now.
 * 
 * Revision 1.6  83/11/30  13:32:05  ada
 * same as 1.5.5.1
 * 
 * Revision 1.5.5.1  83/11/29  15:45:48  snider
 * Some changes in domacro to bring things up to date with the current
 * symbol table.
 * 
 * Revision 1.5  83/06/14  16:43:19  ada
 * Same as 1.3.40.1.
 * Version 1.4 is from a failed experiment.  Ignore it.
 * 
 * Revision 1.3.40.1  83/06/14  13:45:27  tadman
 * "Elaborate" context specifications attached to macros.
 * 
 * Revision 1.3  83/01/12  09:11:48  ada
 * Added copyright notice.
 * 
 * Revision 1.2  83/01/11  14:44:37  ada
 * Same as 1.1.2.1.
 * 
 * Revision 1.1.2.1  83/01/11  09:45:22  willson
 * Altered to use new array dope vector (arr.h)
 * 
 * Revision 1.1  82/12/12  21:18:03  tadman
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
/* port: block-scope externs, repeated at file scope */
extern char *macro_pname();
extern long map();
extern long nill();

extern char *macro_pname(), *macro_hash_name();
extern nodep ada_copytree();
extern typep tyeval();

boolean enterargs();
boolean macro_match();		/* forward */
boolean macro_check();
boolean macro_args();
boolean macro_symbol_lookup();
fixp macro_params();

extern nodep *intsp, *intstk_end, intstk[];    
		/* interpreter stack & stack pointer */

extern symtabp parameter_table;
extern nodep handler;	/* exception handler -- set by raise */
extern nodep stack_level;	/* return stack level */

static nodep calling_form;

typep type_intnode = null, 
	type_stmtnode, type_namenode, type_exprnode,
	type_declnode, type_typenode;

typep look_for_type(ty)
    char *ty;
{
    fixp fix;

    fix = lookup(currtab, intern(ty),null);
    if (fix == null) 
	error("%s is not defined -- load package castor",ty);
    return fix->fxf.dyna->sy.var.type;
}

long lookup_types()
{
    if (type_intnode) return;
    type_intnode = look_for_type("intnode");
    type_stmtnode = look_for_type("stmtnode");
    type_namenode = look_for_type("namenode");
    type_exprnode = look_for_type("exprnode");
    type_declnode = look_for_type("declnode");
    type_typenode = look_for_type("typenode");
}

long domacro(p,mode)
    nodep p;
    enum int_modes mode;
{
    fixp f;
    symp s;
    char *hash_name;
    typep ty;
    boolean writeout = false;

    if (mode == decl_mode) {
	fixp owner _ currtab->symtab_owner;
	lookup_types();
        hash_name = macro_hash_name (p -> nd.macrox.designator);

	if (owner and
	    owner->fxf.class eq pack_id and
	    owner->fx.pack.int_form->nd.packagex.table and
	    (f _ lookuptab(
		    owner->fx.pack.int_form->nd.packagex.table,
		    hash_name, 
		    macro_match,
		    p->nd.macrox.designator))) {

/*
Should do this check, but the way packages are represented in the symbol
table (the pointer to the spec internal form of a macro is
placed in the fx.func.int_form field when the spec is defined; when
the body is later defined, that field is replaced by a pointer to
the internal form of the body) prevents this currently.

	    if (f->fx.func.int_form) 
		error("Attempt to redefine %s in package spec",
		      hash_name);
*/
	    writeout _ true;
	}
        else if ((f = lookuptab(currtab,
		        hash_name,
		        macro_match,
		        p -> nd.macrox.designator)) == null) {
		f = newfix(funcfixform, macro_id);
		s = newsym (funcsymbolform, macro_id);
    		f -> fxf.name = hash_name;
		enter(currtab,f,s);
		f->fxf.class _ macro_id;
		writeout = true;
            }
            else if (f -> fx.func.int_form eq null) {  
			/* do nothing; reuse old symbol which is
			presumably from a package spec; either way,
			it doesn't have any internal form, so it
			gets this one.  Note:  this alternative must
			precede the next one... */
	    }  
	    else if (currtab == usrtab) {
	    	    sysputf ("{%s} redefined in package 'usr'",
		    	     macro_pname(p->nd.macrox.designator));
		    writeout = true;
	    }
	    else {  /* make new local subp sym */
	        f = newfix(funcfixform, macro_id);
                s = newsym (funcsymbolform, macro_id);
                f -> fxf.name = hash_name;
                enter(currtab, f, s);
	    }
        f -> fxf.class = macro_id;

/*        if (p -> nd.macrox.bodyx)     */
	    f -> fx.func.int_form = p;

        f->fxf.dyna->sy.func.type = ty = 
	    tyeval(p -> nd.macrox.ret_type, null, false);
	if (ty != type_stmtnode && ty != type_namenode &&
	    ty != type_declnode && ty != type_typenode &&
	    ty != type_exprnode)
	    error("Invalid return type for macro");
	if (writeout && currtab == usrtab)
	    subp_writeout(f);

	p -> nd.macrox.table = newtab(32L, f);
 	p -> nd.macrox.table -> symtab_static_link = currtab;
    }
    else system_error("Bad mode in eval in nop_macro");
}

boolean macro_symbol_lookup(fix)
    fixp fix;
{
    register    nodep ip;
    register nodep p = calling_form;

    if (fix == null || fix -> fxf.class != macro_id) return false;
    ip = fix -> fx.func.int_form;
    return macro_check (ip -> nd.macrox.designator, p);
}

 /*	Handle a macro call.  A macro call is vary similar to a function
	call, the main difference being the parameter passing mechanism.
	This code is therefore similar to the docall function. 
	macro_call returns true if the macro called if defined and
	false if it is not */

boolean macro_call(p, mode, result)
    nodep p;
    enum int_modes mode;        
    typedval *result;
{
    symp sym;			/* symbol of subp we're calling */
    fixp fix;			/* fixed part of subp we're calling */
    fixp paramlist;		/* pointer to symbols for formals */
    register fixp formals;	/* used to traverse formal params */
    register nodep ip;		/* pointer to subp node from symbol */
    nodep macro_int;		/* internal form for subprogram decls &
				   body */
    symtabp tab;		/* hash table used to match actuals and
				   formals */
    char *hash_name;		/* hash name of macro */
    symtabp save_currtab = currtab;	/* current table */

   /* lookup the symbol in the symbol table.  If the mode is symbol
      mode then we just want a pointer to the symbol table entry.
      Since macros are hashed on their first identifier prefixed by
      an at sign, macro_symbol_lookup is used to do a full compare
      of the calling form with the candidates chosen by lookup */

    calling_form = p;
    hash_name = macro_hash_name(p);
    fix = lookup(currtab, hash_name, macro_symbol_lookup, result);

    if (fix == null)
	if (mode == symbol_mode)
            error("undefined macro: {%s}", macro_pname(p));
        else return false;

    sym = fix->fxf.dyna;

	/* set stattab */

 /* Must be a macro */

    if (fix -> fxf.class != macro_id)
	system_error ("Not a macro in macro_call!");

   /* subp_swapin (sym); */

    if (mode == symbol_mode) {
	result->typ = symboltype;
	result->val.fptr = fix;
	return true;
    }

 /* set "ip" to point to the macrox node for the macro.  This is in
    func.intform. */

    ip = (nodep) fix -> fx.func.int_form;

 /* evaluate the formal parameters into a table.  */

    tab = newtab (16L, null);

    paramlist = macro_params(ip->nd.macrox.designator, p, tab);


    currtab = ip -> nd.macrox.table;

 /* for compiled code, we load the argument (addr's or values) into an
    array that gets pushed on the C stack by "call.a". If the actual
    parameter is an array, then we have to create a little array
    descriptor and make it point at the array so the compiled code (which
    can't look at symbols (yet)) can get the size of the array and what
    not. */

	/* no compiled macros for now */
# ifdef COMPILED
    if (sym -> sy.func.code) {	/* compiled C or Ada */
	long     argtab[MAXARGS];
	long     argcount = 0;

	for (formals = paramlist; formals; formals = formals -> sy.var.param_link) {
	    long    *val = formals -> sy.var.value;
	    arrayp p;

	    if (formals -> sy.var.param_base_type -> tyf.class 
		eq un_arrayform) {
		typep typ;
	    /* an arrayp is an array descriptor */
		p = (arrayp) zalloc (sizeof (*p), false);
		p -> arr_base = val;
		typ = formals -> sy.var.type;
		typ = typ -> ty.con_array.index_list;
		typ = typ -> ty.index.range;
		p -> arr_first _ typ->ty.srange.lbound;
		p -> arr_last _ typ->ty.srange.ubound;
		val = (long *) p;
	    }
	    if (blocktype (formals -> fxf.dyna -> sy.var.type) or
		    formals -> fx.var.param_is_out)
		argtab[argcount++] = (long) val;
	    else
		argtab[argcount++] = * val;
	}
    /* As a kindness to "where" and "break" we pop the current context. 
       This horribleness makes references to currtab act like dynamic
       scoping happened. */

	currtab = oldscope;

    /* coerce result to be of appropriate type */
	result -> typ = sym -> sy.func.type;

    /* (should rangecheck result?) */

    /* call compiled function */
	result -> val.integer = call (&argtab[argcount],
		argcount,
		sym -> sy.func.code);/* asm function */
    /* restore correct scoping context */
	currtab = newscope;
    }
    else
# endif COMPILED
	if (macro_int = (nodep) fix -> fx.func.int_form) {
				/* interpreted code */

	/* The following section does the actual calling of the
	   macro. It pushes a pointer to the macrox node of the
	   macro on the interpreter stack.  This has two purposes: 1)
	   it makes the backtrace more informative since the macro
	   appears immediately above it's call and 2) the Castor macro
	   expansion facility uses the stack to find the parent of any
	   macro call.  The parent of the first item in the declaration
	   list and also in the body is the macrox node. The second thing
	   that happens is that an entry for the return stack is built. 
	   This stack is used for handling exceptions, return statements,
	   and exit statements.  The stack entry is actually a structure
	   local to this function, so that if throw (longjmp) is used to
	   get back to an earlier level, the return stack is effectively
	   popped and the storage (on the stack) is released.  Most of the
	   items in the stack entry are obvious.  The in_decls flag is
	   used by the exception handlers to tell whether an exception was
	   raised during elaboration of the declarations or during
	   execution.  The exception field is used by the exception
	   handling code to tell if an exception handler is being executed
	   and if so, which one (this is needed for the raise statement
	   with no exception name). The calls to catch can catch two types
	   of throws.  The return statement throws a 1 and the exception
	   hander code throws a 2.  If a 2 is thrown, a pointer to the
	   exception handler to use is in the global variable handler.	
	   */

	    struct retstkform   rets;

	/* enter actual argument values into the symbol table for this
	   macro. */

	    for (formals = paramlist; 
		 formals; 
		 formals = formals -> fx.var.param_link)
		enter (currtab, formals, formals->fxf.dyna);
	/* note: the symbol is now in TWO tables: "tab" and the one for
	   the macro, so we can't free them until the macrox returns
	   back to here. */

	/* save current spot on the backtrace stack so we can prune it
	   back when we return from the macrox */

	    if (intsp++ == intstk_end) {/* reserve space on stack */
		putf ("\nInterpreter stack overflow\n");
		intsp = intstk;
	    }

	    *intsp = macro_int;

	    if (counting)
		increment_node_counter (macro_int);

	/* evaluate the locals for the macro into currtab */

	    doscope(&rets, macro_int,
		    macro_int -> nd.macrox.context,
		    macro_int -> nd.macrox.decl,
		    macro_int->nd.macrox.bodyx,
		    result);

	/* check result for conformance.  Maybe should do this for
	   compiled code too... */

	    rangecheck (sym -> sy.var.type, result);

	    retval = rets.retvalsav;
	    retstk = rets.link;
	    intsp = rets.intsp_save;

	    intsp--;
	}
	else
	    error ("no code for macro");

 /* Lastly, pop off the dynamic symbols for the execution of the macrox */

    poplevel(currtab, stack_level);
    currtab = save_currtab;
    return true;
}

	/* copy_argument_list copies an entire argument list (without
	   the top level arg nodes) for an array parameter.  It doesn't
	   check types -- that is done later when the list is used */

nodep copy_argument_list(l)
    nodep l;
{
    register nodep n;	/* points to the new list */

    if (l == null) return null;
    n = ada_copytree (l -> ndf.op == nop_arg ? 
	    l -> nd.arg.exp :
	    l);
    n->ndf.next = copy_argument_list(l->ndf.next);
    return n;
}

	/* do_one_param list takes a parameter list and it's corresponding
	   argument list and sets the parameters to their proper values */

fixp do_one_param_list(params,args,tab)
    nodep params;	/* pointer to internal form for params */
    nodep args;		/* pointer to internal form for arguments */
    symtabp tab;	/* table for parameters */
{
    typedval dummy;
    typedval arg;	/* place to evaluate arguments into */
    fixp paramlist;
    typep typ;		/* type of current set of parameters */
    nodep acts;		/* pointer to internal form of actuals */
    fixp formals;	/* pointer to current formal */

 /* The table is passed on the stack to nop_param */   /* fix this */

    parameter_table = tab;
    paramlist =
	(fixp) eval (params -> nd.macro_list.list,
	    decl_mode, &dummy, null);

 /* Now match the formals and the actuals */

    acts = args -> nd.macro_list.list;
    formals = paramlist;

 /* Note we cant use the default existing table for the macro because it
    might be full of important stuff if this is a recursive call. */

    while (acts) {
		/* evaluated parameter have a type specified in the
		   macro header (just like an ADA subprogram) except
		   that no defaults are allowed and there are no out
		   parameters (where would the value go?) */
	typ = formals -> fx.var.param_base_type;
	if (typ) {
	    if (acts->ndf.op != nop_arg)
		error("evaluated macro argument must be an expression");
	    formals ->fx.var.int_form _ acts->nd.arg.exp;
	    if (blocktype(typ)) {
		eval (formals -> fx.var.int_form, addr_mode, &arg, typ);
		formals -> fxf.dyna -> sy.var.value = arg.val.ptr;
	    }
	    else {
		eval (formals -> fx.var.int_form, eval_mode, &arg, typ);
		rangecheck (typ, &arg);
		*(formals -> fxf.dyna -> sy.var.value) = arg.val.integer;

	    }
	/* unconstrained formals take the type of the constrained actual */

	    if (typ -> tyf.class == un_arrayform)
		formals -> fxf.dyna -> sy.var.type = arg.typ;
	/* type of actual arg */
	    else
		formals -> fxf.dyna -> sy.var.type = typ;
	}
		/* unevaluated parameters have no type specified.
		   The value of the parameter is set to the internal
		   form for the parameter and the type is always intnode */
	else {			/* unevaluated parameter */
		/* there are no out parameters, so for now the
		   param_is_out flag means that this is an array parameter.
		   An array parameter is bound to the entire argument list
		   which can be variable length. */
	    if (formals->fx.var.param_is_out) {
		formals->fx.var.int_form = copy_argument_list(acts);
	    	*(formals -> fxf.dyna -> sy.var.value) = 
		    (long)formals -> fx.var.int_form;
	    	formals -> fxf.dyna -> sy.var.type = type_intnode;
		return paramlist;
	    }
	    else {
	    	formals->fx.var.int_form =
	            ada_copytree (acts -> ndf.op == nop_arg ? 
			     	  acts -> nd.arg.exp :
		    	     	  acts);
	    	*(formals -> fxf.dyna -> sy.var.value) = 
		    (long)formals -> fx.var.int_form;
	    	formals -> fxf.dyna -> sy.var.type = type_intnode;
	    }
	}

	acts = acts -> ndf.next;
	formals = formals -> fx.var.param_link;
	if (acts && formals == null)
	    error("Too many arguments to macro");
    }
    if (formals)  error("Too few arguments to macro");
    return paramlist;
}

fixp macro_params(dp, cp, tab)
    nodep dp;		/* pointer to internal form for macro designator */
    nodep cp;		/* pointer to internal form for calling form */
    symtabp tab;	/* table for parameters */
{
    fixp paramlist = null,	/* pointer to parameters */
	 new_params, end_params;

    dp = dp -> nd.call_form.list;
    cp = cp -> nd.call_form.list;
    while (dp != null) {	/* lists must have already been checked
				   for length by macro_check */
	if (dp -> ndf.op != nop_callf_ident) {
	    end_params = new_params = do_one_param_list(dp,cp,tab);
	    while (end_params->fx.var.param_link != null)  
		end_params = end_params -> fx.var.param_link;
	    end_params->fx.var.param_link = paramlist;
	    paramlist = new_params;
   	}
	dp = dp -> ndf.next;
	cp = cp -> ndf.next;
    }
    return paramlist;
}


 /* macro_check checks to see if this is really the right calling form.
    It checks that all identifiers match and that argument and parameter
    lists are in the same position.  It does not check for a match
    between argument and parameter lists.				*/

boolean macro_check(dp, cp) /* compare a macro designator and a
				  calling form (identifiers only) */
    nodep dp, cp;
{

    dp = dp->nd.call_form.list;
    cp = cp->nd.call_form.list;
    while (dp != null && cp != null) {
	if (dp -> ndf.op == nop_callf_ident &&
	        (cp -> ndf.op != nop_callf_ident ||
		 dp -> nd.id_decl.id != cp -> nd.id_decl.id))
	    return false;
        dp = dp -> ndf.next;
	cp = cp -> ndf.next;
    }
    return (dp == null && cp == null);
}



boolean macro_idlistmatch(p1, p2)
    nodep p1, p2;
{
        if(p1 == null && p2 == null)
                return true;
        if(p1 == null || p2 == null)
                return false;
        if(p1->nd.id_decl.id != p2->nd.id_decl.id)
                return false;
        return macro_idlistmatch(p1->ndf.next, p2->ndf.next);
}

    
boolean macro_matchaux(p1, p2) /* compare two macro designators */
    nodep p1, p2;
{
    if (p1 == null && p2 == null) return true;
    if (p1 == null || p2 == null) return false;
    if (p1->ndf.op == nop_callf_ident) {
	if (p2->ndf.op != nop_callf_ident ||
	    p1->nd.id_decl.id != p2->nd.id_decl.id) return false;
    }
    else if (p2->ndf.op == nop_callf_ident) return false;

    return macro_matchaux(p1->ndf.next, p2->ndf.next);
}

/* this routine is only called indirectly by lookup */

boolean macro_match(f, p2)
    fixp f;
    nodep p2;
{
    extern char *macro_pname();
    if (f->fxf.class != macro_id) return false;
    return macro_matchaux(
	f->fx.func.int_form->nd.macrox.designator->nd.call_form.list, 
	p2->nd.call_form.list);
}

  /*  The following strange concoction is to permanently expand a macro at
      run time.  dyn_macro_expand() is the main function.  It locates the
      parent of the current node via the interpreter stack and then uses
      maptree to search the pointers of the parent for a pointer to the
      calling form for the macro.  The calling form is attached to the
      ag list of the expansion of the macro and the parent is changed to
      point to the expansion.  */

static nodep mac_result, call_form;
static boolean parentf;

static long macro_expand_pre(n,p)
    nodep n, *p;
{
    if (n == call_form) *p = mac_result;
}

static long macro_expand_map(p)
    nodep *p;
{
    extern long map();
    map(p);		/* wasteful, but works for now */
# ifdef FRANK
    if (parentf) {		/*  prevents smaptree from searching past
				    the children of the parent */
	parentf = false;
	map(p);
    }
# endif FRANK
}

long dyn_macro_expand(p, c)
    nodep p, c;		/* p pointer to result of macro,
			   c pointer to calling form */
{
    nodep parent;
    register nodep q;
    long len;
    extern long nill();
    
    call_form = c;	/* put into globals */
    mac_result = p;

	/* splice in result of macro */

    len = 1;
    for (q=p; q->ndf.next != null; q = q->ndf.next)
	len++;
    q->ndf.next = c->ndf.next;

    c->nd.call_form.items = len;
    c->nd.call_form.refined_to = p;

    insert_attach(p,c);
    if (intsp <= intstk) return;     /*  no parent */
    parent = intsp[-1];
    parentf = true;
    smaptree(parent, macro_expand_map, macro_expand_pre, nill, nill, nill);
}
