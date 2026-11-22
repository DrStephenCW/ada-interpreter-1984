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

extern symtabp gettab(), poptab();	/* should be in tab.h */

extern nodep intstk[], *intsp, *intstk_end;

extern nodep handler;		/* exception handler -- set by raise */
extern	typep typeonly();

extern nodep *intsp;            /* interpreter stack pointer */

 /* checkrange checks that a typed value (r) is in the range of any
    subrange constraint imposed by type t.  The type of r and t must be
    of the same class of types.  rangecheck first type checks the two
    types for assignment compatibility and then does the rangecheck.

    The reason for the two functions is that checkrange can be used to
    where the types are not assignment compatible, but one is a derived
    type or a subtype of a derived type whos parent type is the other. */

checkrange(t,r)
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

rangecheck(t, r)
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
	(If it's not in the visible table, then it is o.k. to look in
	body if pack.on_stack is true.)  If it is a function, we look in
	the function's symbol table.  Otherwise, we assume that we
	got a record offset.

	Note: if we are called in decl_mode, this is the same as addr_mode
	or symbol_node for packages or functions, but illegal for record
	offsets.  This would be used for something like:
		subtype x is new standard.integer;
*/

dodot(p, mode, result)                          
nodep p;
enum int_modes mode;        
typedval *result;
{                             
        typedval lhs;   	/* result of eval'ing left hand side */
        typep typ;		/* short hand for type of left side */
        symtabp tab;		/* used for looking in pack and func tables */
        symp symbol;		/* symbol found in above table */
	word addr;		/* ? */
        char *fldname;		/* name of identifier on right hand side */

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

		if (mode neq symbol_mode and mode neq addr_mode and
			mode neq decl_mode and mode neq ovl_mode)
			system_error("Bad mode %d in dodot", mode);

		/* Get the symbol table from the internal form for the
		subprogram. */
		if (not (iform _ lhs.val.sptr->sy.func.intform))
			iform _ lhs.val.sptr->sy.func.spec;
		tab _ iform->nd.subp.table;
  /*            tab = gettab(lhs.val.ptr);	*/
                if(tab == null)
                  system_error("Symbol table for subp in lhs of selector not found");
                symbol = lookuptab(tab, fldname, null);
                if(symbol == null)
                    error("Right side of selector not found", fldname);
            }
            else error("illegal use of .all in selector");
            doident(symbol, mode, result);
            return;
        }

	/* or maybe package ... */

        else if(typ == packtype) {
            symp s _ null;

	    if (mode neq symbol_mode and mode neq addr_mode and
		mode neq decl_mode and mode neq ovl_mode)
		system_error("Bad mode %d in dodot", mode);

            if(fldname) {          
                symbol = lhs.val.sptr;
                if(symbol == null)
                    system_error("Can't find package symbol in selector");
                tab = symbol->sy.pack.visible_table;
                if(tab == null)
                   system_error("No symbol table in package for lhs of selector");
                s = lookuptab(tab, fldname, null);
		doident(s,mode,result);
                if(s neq null)
 			return;

		/* if we can't find it, but the package is on the stack,
		that means stuff in the body is visible */

                if(s eq null and symbol->sy.pack.on_stack) {
                        tab = symbol->sy.pack.body_table;
                        if(tab == null)
                                system_error("Can't find package table in selector");
                        s = lookuptab(tab, fldname, null);
			doident(s,mode,result);
                        if(s != null)
                                return;
                }
                if(s == null)
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

	/* It could be record selection indirectly through an access
	type.  If the rhs is null, this means .all, which means return the
	pointer to the object, not the actual object. */

        if(typ->tyf.class eq accessform) {
	  addr.integer = *addr.ptr;
          if(addr.ptr == null)
            sysraise(constraint_error,
		"constraint_error: reference through null access value");
          typ = typ->ty.access.targettype;

	  /* return pointer to object? */
          if(fldname == null) {      
            if (mode eq eval_mode) {   
                if(not blocktype(typ))
		    addr.integer = *addr.ptr;
            }
            result->val = addr;       
            result->typ = typ;
            return;
          }   
        }

        if (typ->tyf.class neq recordform)
            error("Record type expected in lhs of selector");

        if(fldname eq null)
            error("illegal use of .all in record selector");

        tab = typ->ty.record.fieldtab; /* points to record's symtab */

        symbol = lookuptab(tab, fldname, null);
                    /* lookup offset */
        if (symbol eq null)
            error("Record field %s not found", fldname);      

	addr.integer += *(symbol->sy.var.value);
                    /* add offset and base addr */

        typ = symbol->sy.var.type;   
        if (mode eq eval_mode) {   
            if(not blocktype(symbol->sy.var.type))
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
		result->val.sptr _ symbol;
		return;
	}
	else system_error("Can't handle mode (%d) in 'dodot'", mode);
}

blockmove(dst, src, n)
char *dst, *src;
int n;
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
int n;
{
        while(n--)              
                if(*dst++ != *src++)
                        return false;
        return true;
}

fixp lookupfixp();

/*	Given an identifier reference we do the right thing based on
	its type.  If we are referencing a subprogram or package, we
	must swap it into memory before using it.  If in symbol mode
	(mainly for pp(x'sym) type stuff) we simply return with the
	result set to the symbol we were passed.  

	Types and subtypes are "typetype", and are simply returned.
	The hard stuff is objects or parameters.  See the big table
	below. */

doident(s, mode, result)               
    symp s;
    enum int_modes mode;
    typedval *result;
{
        if(s->syf.class == func_id || s->syf.class == proc_id)
                subp_swapin(s);
        else if(s->syf.class == pack_id)
                package_swapin(s);

        if(mode == symbol_mode) {
                result->typ = symboltype;
                result->val.sptr = s;
                return;
        }

        switch(s->syf.class) {
            case func_id:
            case proc_id:
                result->typ = functype;
                result->val.sptr = s;
                break;
            case type_id:
            case subtype_id:
                result->typ = typetype;
                result->val.tptr = s->sy.var.type;
                break;
            case pack_id:
                result->typ = packtype;
                result->val.sptr = s;
                break;          
	    case except_id: {
			/* yes, I know that we've already done a lookup
			   on this thing but I don't really know what to
			   do about it for now.  (FT) */
		fixp f = lookupfixp(s->syf.name,null);

		result->typ = excepttype;
		result->val.fptr = f;
		}
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
                    s->syf.class == param_id and s->sy.var.param_is_out;
                switch(mode) {
                  case addr_mode:
		  case ovl_mode:
		      result->val.ptr = s->sy.var.value;
                    break;      
                  case eval_mode:
		     if (blocktype(s->sy.var.type))
			result->val.ptr = s->sy.var.value;
		     else 
			result->val.integer = *(s->sy.var.value);
                    break;
                  default:
                    system_error("bad mode (%d) in eval of ident", mode);
                }
                result->typ = s->sy.var.type;
                break;
            }
            default:
                system_error("bad class (%d) for identifier %s",
			s->syf.class,
			s->syf.name);
        }
}

/*	For docall, we have something arbitrarily complex on the as the
	name of the routine we are to call, and an argument list.  We must
	evaluate the call var in the mode we were called in, lest we
	cause unwanted side effects.  The arguments are always evaluated
	first in "typeonly" mode, so that when we finally do eval them,
	we do it once in the correct mode (eval_mode or addr_mode).
*/

docall(p, mode, result)
nodep p;
enum int_modes mode;        
typedval *result;
{                             
	register int i;
	nodep arglist;
	typedval lhs;		/* for getting type of call var */
	symp sym;		/* symbol of subp we're calling */
	typep argtypes[MAXARGS];/* table of types of actual params */
	typep formtypes[MAXARGS];/* table of types of formal params */
	int argtab[MAXARGS];	/* table of eval'd actual params */
	boolean named_param;	/* true if we've seen a named parameter */
        symp paramlist;		/* pointer to symbols for formals */
        register symp formals;	/* used to traverse formal params */
        register nodep acts;	/* used to traverse actual params */
	register nodep currform;/* current id in formals */
        register nodep ip;	/* pointer to subp node from symbol */
        register typep formtyp;	/* used while traversing formal type array */
	register typep acttyp;	/* used while traversing actual type array */
	nodep subp_int;		/* internal form for subprogram decls & body */
        int val;		/* used as temp for value of actual param */
        symp s, t;		/* Used to traverse symbol lists */
        scopep stattab;		/* local copy of static_scope global */
	scopep oldscope;	/* saved scope of caller */
	scopep newscope;	/* saved scope of callee */
        symp owner;		/* points to owner of subprogram symtab */
        boolean pushed_pack = false;	/* true if subp was located in a
				package not on the stack and we had to push
				the package on the stack first (which means
				we pop if off when we're done. */
	symtabp tab;		/* hash table used to match actuals and
				formals */

	/* some attributes look have call nodes as the top level (see the
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
	if(acts->ndf.op != nop_range) {	/* slice hack */
		while(acts != null) {
			if(acts->ndf.op != nop_arg)
				error("Can't do slices yet");
			argtypes[i++] = typeonly(acts->nd.arg.exp);
			acts = acts -> ndf.next;
		}
	}
	else error("Can't do slices yet");

	/* before anything, see if we are eval'ing "put" or "get", and append
	the appropriate ending to it, to simulate overloading for these
	so as to make the P.I. very happy. So, put(1) => putint(1)... This
	won't catch something like uci_ada_functions.put(5);  */

	if ((i > 0) and
	    (p->nd.call.var->ndf.op eq nop_ident) and
	    ((p->nd.call.var->nd.ident.id eq intern("put")) or
	     (p->nd.call.var->nd.ident.id eq intern("get")))) {
		char *ending;
		char start[20];
		if (compatible(argtypes[0],strtype)) ending _ "str";
		 else if (compatible(argtypes[0],inttype)) ending _ "int";
		 else if (compatible(argtypes[0],chartype)) ending _ "chr";
		 else if (compatible(argtypes[0],realtype)) ending _ "real";
		 else error("Unknown argument type for %s",
			p->nd.call.var->nd.ident.id);
		strcpy(start,p->nd.call.var->nd.ident.id);
		strcat(start, ending);
		sym _ lookup(intern(start), null);
		if (not sym) system_error("%s version of %s not found!", start,
			p->nd.call.var->nd.ident.id);
		lhs.val.sptr _ sym;
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
					un_array.ele_type;
		else result -> typ _ lhs.typ;
		result -> val _ lhs.val;
		return;
	}

	sym _ lhs.val.sptr;	/* will be used later if this is a subp */

	/* save a local copy of the scopep that contained the result of
	the lookup that we just did.  This is how the
	static link gets set. */

        stattab = static_scope;
        if(stattab==null)
              system_error("No static link for call in enterargs! Yikes!");

	/* could be an object or parameter that is a constrained array,
	so let doindex take care of that... */

	if (lhs.typ eq typetype) {	/* type conversion: width(5) */
		do_type_conv(lhs.val.tptr,
				mode,
				p->nd.call.args,
				result);
		return;
	}

	if (lhs.typ neq functype) {	/* must be array reference */
		typep arrtype _ lhs.typ;
		if (arrtype->tyf.class eq con_arrayform) {
			doindex(arrtype,
				lhs.val,
				mode,
				p->nd.call.args,
				result);
			return;
		}
		else error("Object must be constrained array");
	}

	/* Otherwise must be a subprogram; sym is already set to be
	the symbol value */

        if(sym->syf.class != func_id && sym->syf.class != proc_id)
            system_error("call form not a subprogram; yikes!");

	if (p->ndf.op eq nop_proc_call) {
	  if (sym->syf.class neq proc_id)
	    error("Attempt to use procedure like a function");
	}
	else if (sym->syf.class neq func_id)
	  error("Attempt to call function like a procedure"); 

        subp_swapin(sym);

	/* set "ip" to point to the subp node for the subprogram.  This is
	in func.intform for interpreted code and in func.spec for compiled
	code.  */

        if((ip = (nodep)sym->sy.func.intform) == null)
                ip = (nodep)sym->sy.func.spec;

	/* evaluate the formal parameters into a table.  */

	tab _ newtab(16,null);

	/* The table is passed on the stack to nop_param */

	pushtab(tab, currtab);
	paramlist _ (symp)eval(ip->nd.subp.spec, decl_mode, &dummy, null);
	poptab();

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
		symp s;

		s _ formals;		/* current formal param in list */

		if (not s) 
		    error("Too many arguments to subprogram");

		if (acts->nd.arg.name) {
			named_param _ true;
			id _ acts->nd.arg.name;
			s _ lookuptab(tab, id, null);
			if (not formals)
				error("Can't associate named param in subp");
		}
		else if (named_param) 
			error("Positional params must precede named params");
		
		s->sy.var.intform _ acts->nd.arg.exp;
		s->sy.var.param_has_value _ false;
		s->sy.var.type _ argtypes[i++];
		typecheck(s->sy.var.param_base_type, s->sy.var.type);
		acts _ acts->ndf.next;
		formals _ formals->sy.var.param_link;
	}

	/* now create new context for evaluation, but save the old one
	for when we evaluate the actuals */

	oldscope _ currtab;

	/* Get the owner: if it's a package, then we have to see if we
	should push it on the stack. */

        owner = stattab->table->symtab_owner;

        if(owner->syf.class == pack_id) {
          if(not owner->sy.pack.on_stack) {
            if(owner->sy.pack.body_table == null ||
               owner->sy.pack.visible_table == null)
                      error("%s: no package body", owner->syf.name);
            pushtab(owner->sy.pack.visible_table,
		    stattab->symtab_stat_link);
                              /* owner of package function belongs to */
            pushtab(owner->sy.pack.body_table, currtab);

	    /* for something in a package, the static link will point to
	    the package that encloses it. */

            stattab = currtab;

	    /* remember that we've pushed the package on the stack 
	    so we can pop it off later; also set pack.on_stack so that
	    if this subprogram calls someone else inside the package we
	    won't push it on the stack again.  */

            owner->sy.pack.on_stack = true;
            pushed_pack = true;
          }
        }                                    


	/* If there is no table create one and push it, otherwise just
	push the old table. */

	if (ip->nd.subp.table eq 0) {
            pushtab(newtab(32, sym), stattab);
	    ip->nd.subp.table _ currtab -> table;
        }
	else
	    pushtab(ip->nd.subp.table, stattab);

	/* save in newscope the scope of the callee */

	newscope _ currtab;

	currtab _ oldscope;	/* scope of caller */

	/* now evaluate arguments */

	formals _ paramlist;

        while(formals) {
		typedval arg;
		typep typ;

                typ = formals->sy.var.param_base_type;

		/* pass eval the expected typ for the expression being
		evaluated in case something in it is overloaded. 
		Test if should be by addr or value.  Also, if has_value
		is set, we eval in the environment of the caller. */

		if (not formals->sy.var.param_has_value)
			currtab _ oldscope;
		else currtab _ newscope;

                if(blocktype(typ) or formals->sy.var.param_is_out) { 
			eval(formals-> sy.var.intform, addr_mode, &arg, typ);
			formals-> sy.var.value _ arg.val.ptr;
                }
		else {
			eval(formals-> sy.var.intform, eval_mode, &arg, typ);
			rangecheck(typ, &arg);
			*(formals-> sy.var.value) _ arg.val.integer;
                }

		/* unconstrained formals take the type of the 
		constrained actual */

                if(typ->tyf.class == un_arrayform)
		    formals->sy.var.type = arg.typ;
			       /* type of actual arg */
                else formals->sy.var.type = typ;

		formals _ formals->sy.var.param_link;
        }

	/* Make sure we're in the scoping environment of the callee. */

	currtab _ newscope;

	/* for compiled code, we load the argument (addr's or values)
	into an array that gets pushed on the C stack by "call.a". If the
	actual parameter is an array, then we have to create a little
	array descriptor and make it point at the array so the compiled
	code (which can't look at symbols (yet)) can get the size of the
	array and what not. */

        if(sym->sy.func.code) {        /* compiled C or Ada */ 
	    int argtab[MAXARGS];
	    int argcount _ 0;

	    for (formals _ paramlist; formals; formals _ formals -> sy.var.param_link){
		int *val _ formals -> sy.var.value;
                arrayp p;

		if (formals->sy.var.param_base_type->tyf.class eq un_arrayform) {
		    typep typ;
		    /* an arrayp is an array descriptor */
                    p = (arrayp)malloc(sizeof(*p));
                    p->arr_base = val;
		    typ = formals->sy.var.type;
		    typ = typ->ty.con_array.index_list;
		    typ = typ->ty.index.range;
		    p->arr_size = rangelength(typ);
		    val _ (int *)p;
                }
		if (blocktype(formals->sy.var.type) or 
		    formals->sy.var.param_is_out)
		    argtab[argcount++] _ (int)val;
		else
		    argtab[argcount++] _ *val;
            }
	    /* As a kindness to "where" and "break" we pop the current
	    context.  This horribleness makes references to currtab act
	    like dynamic scoping happened. */

	    currtab _ oldscope;

	    /* coerce result to be of appropriate type */
            result->typ = sym->sy.func.type;

		/* (should rangecheck result?) */

	    /* call compiled function */
	    result->val.integer = call(&argtab[argcount],
				       argcount,
				       sym->sy.func.code);  /* asm function */
            /* restore correct scoping context */
	    currtab _ newscope;
        }                   
        else if(subp_int = (nodep)sym->sy.func.intform) {/* interpreted code */

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
	    int exception;

	    /* enter actual argument values into the symbol table for 
	  	this subprogram. */

	    for(formals _ paramlist; formals; formals _ formals->sy.var.param_link)
		enter(formals);		
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

	    rets.link = retstk;
	    retstk = &rets;
            rets.retvalsav = retval;
	    retval _ result;

	    if (counting) increment_node_counter(subp_int);	

	    /* evaluate the locals for the subp into currtab->table */

	    rets.in_decls = true;
	    eval(subp_int->nd.subp.decl, decl_mode, &dummy, null);

	    rets.p = subp_int;		/* save pointer to internal form */

            rets.intsp_save = intsp;     /* save interpreter stack position */
                                         /* around catch & throw */
	    rets.currtab = currtab;	/* remember dynamic stack position */
	    rets.in_decls = false;
	    rets.exception = null;
            exception = catch(&rets.retenv, 
                              eval, 
                              subp_int->nd.subp.bodyx,
                              eval_mode, 
			      &dummy,
			      null);

   	    if (exception > 1) {
		poptabs(rets.currtab);
		intsp = rets.intsp_save;
		retval = rets.retvalsav;
		retstk = &rets;
                exception = catch(&rets.retenv, 
                              eval, 
                              handler,	/* global set by raise */
                              eval_mode, 
			      &dummy,   /* was 'ovlargs', don't know why */
			      null);
			/* execute handler */
	    }
	    poptabs(rets.currtab); /* throw away any extra tables */

	    /* check result for conformance.  Maybe should do this for
	    compiled code too... */

            rangecheck(sym->sy.var.type, result);

            retval = rets.retvalsav;
	    retstk = rets.link;
            intsp = rets.intsp_save;

	    intsp--;
	}
        else error("no code for function");

	/* Lastly, pop off the dynamic symbols for the execution of the
	subp, and if we pushed package, remove it gently from the stack. */

 bottom:
	poplevel();
	poptab();
	/*	freetab(tab);	*/
        if(pushed_pack) {
                owner->sy.pack.on_stack = false;
                poptab();
                poptab();
        }
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
boolean argmatch(s, p2)
symp s;
nodep p2;
{
        return matchaux(((nodep)(s->sy.func.spec))->nd.subp.spec, p2);
}

doblock(p)
    nodep p;
{
    struct retstkform rets;
    int exception;

    if (p->nd.subp.table eq 0) {
	pushtab(newtab(32,null), currtab);  /* push local tab (no owner) */
	p->nd.subp.table _ currtab -> table;
    }
    else pushtab(p->nd.subp.table, currtab);

   /* The following section of code is identical to a section of enterargs
      which has a large comment attached.  This comment will not be repeated
      here.  */

    rets.link = retstk;
    retstk = &rets;

    rets.in_decls = true;

    eval(p->nd.subp.decl, decl_mode, &dummy, null);

    rets.p = p;		/* save pointer to internal form */

    rets.intsp_save = intsp;     /* save interpreter stack position */
                                 /* around catch & throw */
    rets.currtab = currtab;	/* remember dynamic stack position */
    rets.in_decls = false;
    rets.exception = null;
    exception = catch(&rets.retenv, 
                      eval, 
                      p->nd.subp.bodyx,
                      eval_mode, 
		      & dummy,	/* was 'ovlargs', don't know why */
		      null);
    if (exception > 1) {
	poptabs(rets.currtab);
	intsp = rets.intsp_save;
	retval = rets.retvalsav;
	retstk = &rets;
        exception = catch(&rets.retenv, 
                          eval, 
                          handler,	/* global set by raise */
                          eval_mode, 
		          &dummy,   /* was 'ovlargs', don't know why */
			  null);
			/* execute handler (Bang! He's dead.) */
    }
    poptabs (rets.currtab); /* throw away any extra tables */

    retstk = rets.link;
    intsp = rets.intsp_save;

    poplevel();
    poptab();
}

typep mkarrtype();      /* forward declaration, should re-order */

doindex(typ, val, mode, p, result)
typep typ;		/* typep for the array */
int val;		/* base of the array */
enum int_modes mode;	/* evaluation mode */
nodep p;		/* internal form of indices */
typedval *result;	/* result to pass back to eval */
{
	typedval args[MAXARGS];	/* holds indices */
	nodep index;		/* used to flip through indices */
	word addr;
        typep lsttyp;
        typep eletyp;
        int n = 0;		/* counts number of indices */
        int off = 0;		/* offset into array */
                             
	if (p->ndf.op eq nop_range) 
		/* for a slice there is no arg node (some may consider
		this a parsing flaw) so this arg is evaluated specially.
		The resulting typedval will be of the form
		(typ=>typetype,val.tptr=>rangetype).  */
		
		eval(p, mode, &args[0], null);

	else for (index _ p; index; index _ index->ndf.next) {
	    if (n >= MAXARGS) error("More than %d array indices!  (Amazing)",
					MAXARGS);
	    else eval(index->nd.arg.exp, eval_mode, &args[n++], null);
	}

        n = 0;			/* for looping through eval'd args */

        while(typ->tyf.class == accessform) {
                if(val == null)
                   error("reference through null pointer");
                val = *((int *)val);
                typ = typ->ty.access.targettype;
        }
        if(typ->tyf.class == con_arrayform) {
          if(args[0].typ == typetype) {  /* must be a slice */
            typep index = args[0].val.tptr;
            typep baseindex = typ -> ty.con_array.index_list -> ty.index.range;
            typep basetype = typ->ty.con_array.base_type;
            typep eletype = basetype->ty.un_array.ele_type;

            /* make sure index is discrete range type */
            /* make sure index contained in baseindex */
            off = index->ty.srange.lbound - baseindex->ty.srange.lbound;
            off *= eletype->tyf.size;
            result->val.integer = val + off;
            result->typ = mkarrtype(basetype, index);
            return;
          }
          eletyp = typ->ty.con_array.base_type->ty.un_array.ele_type;
          lsttyp = typ->ty.con_array.index_list;
          while(lsttyp) {
            typep rngtyp;
            typep nxttyp;
            int nxtsize;
            int inx;

            rngtyp = lsttyp->ty.index.range;
            nxttyp = lsttyp->ty.index.next;
            nxtsize = (nxttyp ? nxttyp->tyf.size : 1);
            rangecheck(rngtyp, &args[n]);   
            inx = args[n].val.integer;
            inx -= rngtyp->ty.srange.lbound;
            inx *= eletyp->tyf.size * nxtsize;
            off += inx;
            n++;
            lsttyp = nxttyp;
          }
          addr.integer = val + off;
          if(mode == eval_mode) { 
            if(blocktype(eletyp))
              result->val = addr;
            else if(eletyp->tyf.size == sizeof(int))
              result->val.integer = *addr.ptr;
            else if(eletyp->tyf.size == 1)
              result->val.integer = *(char *)addr.ptr;
            else system_error("bug in doindex");
          }
          else if(mode == addr_mode)
            result->val = addr;
          else system_error("bad array mode");
          result->typ = eletyp;
        }
        else error("illegal array reference");
}

 /* do_type_conv handles type conversion of the form type(value).  It
    only does conversions between compatible subtypes and between a
    derived type and its parent type.  It also needs to handle numeric
    conversion */

do_type_conv(typ, mode, p, result)
    typep typ;
    enum int_modes mode;
    nodep p;
    typedval *result;
{
    typep basetype, parenttype;
    typedval n, arg;

    eval(p->nd.arg.exp, eval_mode, &arg, null);

    if (p->ndf.next)  error("Type conversions take only one argument");

    if (mode != eval_mode ) 
	system_error("Invalid mode for type conversion");
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

subp_name(id, class)    /* called by pragmas */
char *id;
enum symbolclasses class;
{
        register symp sym;
  
        sym = newsym(funcsymbolform, class);
        sym->syf.name = id;
        enter(sym);
}

subp_swapin(sym)
symp sym;
{
        char name[128];
        if(sym->sy.func.intform || sym->sy.func.spec)
                return;
        savename(name, sym);
	/* should set up sym tab stack so visibility is right */
        include(name, true);
}

package_name(id)        /* called by pragma package_name */
char *id;
{
        register symp sym;

        sym = newsym(packsymbolform, pack_id);
        sym->syf.name = id;
        enter(sym);
}

package_swapin(sym)
symp sym;
{
        char name[128];

        if(sym->sy.pack.int_form)
                return;
        savename(name, sym);
	/* should set up sym tab stack so visibility is right */
        include(name, true);
}

static boolean writeout;		/* hack */

dopackspec(p)                                        
nodep p;
{
        register    symp sym;
        register    symtabp tab;

	writeout = false;
	sym = lookuptab (currtab -> table, p -> nd.packagex.name, null);
	if (sym == null) {
	    sym = newsym (packsymbolform, pack_id);
	    sym -> syf.name = p -> nd.packagex.name;
	    enter (sym);
	    writeout = true;
	}
	/* should check for redef */
	sym -> sy.pack.int_form = p;
	sym -> sy.pack.on_stack = true;
	tab = newtab (32, sym);
	pushtab (tab, currtab);
	eval (p -> nd.packagex.decl, decl_mode, &dummy, null);
	sym -> sy.pack.visible_table = poptab ();
	p -> nd.packagex.visible_table _ sym -> sy.pack.visible_table;
	sym -> sy.pack.on_stack = false;
}

dopackbody(p) 
nodep p;
{
        register    symp sym;
        register    symtabp tab;

	sym = lookuptab (currtab -> table, p -> nd.packagex.name, null);
	if (sym == null)
	    error ("no package %s", p -> nd.packagex.name);
	sym -> sy.pack.body_int_form = p;
	pushtab (sym -> sy.pack.visible_table, currtab);
	sym -> sy.pack.on_stack = true;
	tab = newtab (32, sym);
	pushtab (tab, currtab);
	eval (p -> nd.packagex.decl, decl_mode, &dummy, null);
	eval (p -> nd.packagex.bodyx, eval_mode, &dummy, null);
	sym -> sy.pack.body_table = poptab ();
	p -> nd.packagex.body_table _ sym -> sy.pack.body_table;
	poptab ();
	sym -> sy.pack.on_stack = false;
	if(writeout && currtab == usrtab)
		pack_writeout(sym);
}

typep mkrangetype(basetype, lower, upper) /* should call this in tyint.c */
typep basetype;
int lower, upper;
{
        register typep t = newtype(srangeform);

        t->tyf.size = sizeof(int);
        t->ty.srange.lbound = lower;
        t->ty.srange.ubound = upper;
        t->ty.srange.basetype = basetype;
        return t;
}

doattr(p, result)
nodep p;
typedval *result;
{
        typedval t;
        char *s;
        typep typ;

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

        if(typ->tyf.class == con_arrayform)
                typ = typ->ty.con_array.index_list->ty.index.range; /* assumes 1-dim */

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

doattr2(p, result)
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
nodep mk_constrained_string_node(n) {
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
        int len = rangelength(range);

        i->ty.index.range = range;
        i->tyf.size = len;
        a->ty.con_array.index_list = i;
        a->ty.con_array.base_type = basetype;
        a->tyf.size = basetype->ty.un_array.ele_type->tyf.size * len;
        return a;
}

dostring(s, result)
char *s;
typedval *result;
{
        int len = strlen(s);
        int *p = (int *)malloc(len * sizeof(int));
        register int i;

        result->typ = mkarrtype(strtype, mkrangetype(inttype, 1, len));
        for(i = 0; i< len; i++)
                p[i] = s[i]&0377;
        result->val.ptr = p;
}

catenate(l,r,result)
    typedval l,r,*result;
{
    register char  *p,
                   *g;
    register int    len;
    int     low;

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
	    result -> val.ptr = (int *) p;
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
		result -> val.ptr = (int *) p;
		g = (char *) l.val.ptr;		/* copy the array */
		for (len = 0; len < l.typ -> tyf.size; len++)
		    *p++ = *g++;
		if (blocktype (r.typ)) {	/* now copy the single element */
		    g = (char *) r.val.ptr;
		    for (len = 0; len < r.typ -> tyf.size; len++)
			*p++ = *g++;
		}
		else
		    *(int *) p = r.val.integer;
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
	    result -> val.ptr = (int *) p;
	    if (blocktype (l.typ)) {		/* copy the single element */
		g = (char *) l.val.ptr;
		for (len = 0; len < l.typ -> tyf.size; len++)
		    *p++ = *g++;
	    }
	    else {  /* anyone have a better idea? */
		register int *pp = (int *)p;
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
	  

agg_eval(p, result, expected_type)
nodep p;
typedval *result;
typep expected_type;
{
	register int len;
	register typep res_type;
	register typep ele_type;
	int *agg_space;
	int agg_len = 0;
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
						 1,
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
	agg_space = (int *)malloc(len * ele_type -> tyf.size);
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
