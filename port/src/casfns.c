static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/casfns.v  Revision 1.4  84/01/12  16:35:59  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/casfns.v $
 * Revision 1.4  84/01/12  16:35:59  ada
 * same as 1.3.4.1
 * 
 * Revision 1.3.4.1  84/01/06  16:11:43  tadman
 * Add symbol table parameter to macro_lookup call.
 * 
 * Revision 1.3  83/03/10  15:23:25  ada
 * Same as 1.2.2.1.
 * 
 * Revision 1.2.2.1  83/03/10  14:42:40  willson
 * *** empty log message ***
 * 
 * Revision 1.2  83/01/12  09:11:38  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:17:51  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"
#include "oblist.h"
#include "castor.h"
/* port: block-scope externs, repeated at file scope */
extern long nill();
extern nodep maptreelist();
extern nodep maptree();

extern fixp macro_lookup();
extern nodep ada_copytreelist();
extern nodep extract_attach();
extern typep type_intnode, 
	type_stmtnode, type_namenode, type_exprnode,
	type_declnode, type_typenode;

    /*  check_macro_return_type verifies that the type of the calling
	form matches the return type of the macro that it calls.  The
	type of the calling form is determined by the parser by its
	syntatic location.  */
	
long check_macro_return_type(call_form, typ)
    nodep call_form;		/* the calling form for the macro (internal
				   form) */
    typep typ;			/* the return type of the macro */
{
    switch(call_form -> ndf.op) {
	case nop_decl_callf:
	    if (typ != type_declnode)
		error("Macro must return a declaration");
	    break;
	case nop_expr_callf:
	    if (typ != type_exprnode)
		error("Macro must return an expression");
	    break;
	case nop_name_callf:
	    if (typ != type_namenode)
		error("Macro must return a name");
	    break;
	case nop_stmt_callf:
	    if (typ != type_stmtnode)
		error("Macro must return a statement");
	    break;
	case nop_type_callf:
	    if (typ != type_declnode)
		error("Macro must return a type");
	    break;
	case nop_top_callf:
	    /* unrestricted, so do no checking */
	    break;
	default:
	    system_error("not a calling form in check_macro_return_type");
    }
}


     /* The following routines do substitution of macro variables in
	the genxxx functions.  The main procedure is in this group is
	macro_substitute.  It searches (via maptree) the given tree for
  	all macro substitution nodes and replaces them with the value
	of the macro variable.  Before doing so, it checks that the
	subtree to be spliced fits syntatically in the location that it
	will occupy. 

	The four is_??? functions check the root of the given tree to
	determine if it is in the appropriate syntatic category. */

boolean is_decl(tree)
    nodep tree;
{
    if (tree == null) return false;
    switch (tree->ndf.op) {
	case nop_addr_spec:
	case nop_body:
	case nop_decl_callf:
	case nop_except_decl:
	case nop_except_ren:
	case nop_func_ginst:
	case nop_func_ren:
	case nop_function:
	case nop_gen_with:
	case nop_generic:
	case nop_instant:
	case nop_len_spec:
	case nop_limited:
	case nop_macro:
	case nop_obj_decl:
	case nop_obj_ren:
	case nop_pack_ginst:
	case nop_pack_ren:
	case nop_package:
	case nop_proc_ginst:
	case nop_proc_ren:
	case nop_procedure:
	case nop_record:
	case nop_task:
	case nop_task_body:
	case nop_task_ren:
	case nop_task_type:
	case nop_type:
	case nop_use:
	    return true;
default:
    return false; 
    }
}

boolean is_expr(tree)
    nodep tree;
{
    if (tree == null) return false;
    switch (tree->ndf.op) {
	case nop_aggregate:
	case nop_amper:
	case nop_and:
	case nop_andthen:
	case nop_call:
	case nop_char:
	case nop_div:
	case nop_dot:
	case nop_equal:
	case nop_expr:
	case nop_expr_callf:
	case nop_ge:
	case nop_gt:
	case nop_ident:
	case nop_in:
	case nop_index:
	case nop_int:
	case nop_le:
	case nop_lp:
	case nop_lt:
	case nop_minus:
	case nop_mod:
	case nop_neg:
	case nop_nequal:
	case nop_new:
	case nop_not:
	case nop_null:
	case nop_oper_string:
	case nop_or:
	case nop_orelse:
	case nop_plus:
	case nop_pos:
	case nop_qual_expr:
	case nop_quote:
	case nop_range:
	case nop_real:
	case nop_rem:
	case nop_star:
	case nop_string:
	case nop_xor:
	    return true;
 	default:
	    return false;
    }
}

boolean is_name(tree)
    nodep tree;
{
    if (tree == null) return false;
    switch (tree->ndf.op) {
	case nop_aggregate:
	case nop_call:
	case nop_char:
	case nop_div:
	case nop_dot:
	case nop_ident:
	case nop_index:
	case nop_int:
	case nop_name_callf:
	case nop_null:
	case nop_oper_string:
	case nop_qual_expr:
	case nop_quote:
	case nop_range:
	case nop_real:
	case nop_string:
	    return true;
 	default:
	    return false;
    }
}


static boolean is_stmt(tree)
    nodep tree;
{
    if (tree == null) return false;
    switch (tree->ndf.op) {
	case nop_abort:
	case nop_accept:
	case nop_asgn:
	case nop_case:
	case nop_code:
	case nop_cond_ent:
	case nop_declare:
	case nop_delay:
	case nop_entry:
	case nop_exit:
	case nop_expon:
	case nop_for:
	case nop_goto:
	case nop_if:
	case nop_loop:
	case nop_null_stmt:
	case nop_private:
	case nop_proc_call:
	case nop_raise:
	case nop_return:
	case nop_select:
	case nop_stmt_callf:
	case nop_terminate:
	case nop_timed_ent:
	case nop_when:
	case nop_while:
	    return true;
	default:
	    return false;
    }
}

boolean is_type(tree)
    nodep tree;
{
    if (tree == null) return false;
    switch (tree->ndf.op) {
	case nop_access:
	case nop_array:
	case nop_delta:
	case nop_derived_type:
	case nop_digits:
	case nop_disc_type:
	case nop_enum:
	case nop_subtype:
	case nop_type_callf:
	    return true;
	default:
	    return false;
    }
}


static boolean is_ident(tree)
    nodep tree;
{
    if (tree == null) return false;
    if (tree->ndf.op != nop_ident) return false;
    else return true;
}

long macro_splice(p,v)
    nodep *p;
    register nodep v;
{
    register nodep e;

    e = v = ada_copytreelist (v);

 /* splice the code in as a list */

    while (e -> ndf.next != null)
	e = e -> ndf.next;
    e -> ndf.next = (*p) -> ndf.next;

    *p = v;
}

static nodep macro_variable_value(n)
    nodep n;
{
    fixp f;

    f = macro_lookup(n->nd.ident.id, null);
    if (f == null)
	error("undefined macro variable: %s", n->nd.ident.id);
    if (f->fxf.class != obj_id && f->fxf.class != param_id)
	error("macro variable is not an object: %s", n->nd.ident.id);
    return (nodep) *(f -> fxf.dyna -> sy.var.value);
}

static long pre_macro_subst(n,p)
    nodep n, *p;
{
    nodep v;

    switch (n->ndf.op) {
	case nop_decl_subst:
	    v = macro_variable_value(n);
	    if (! is_decl(v))
		error("Value of $%s is not a declaration", n->nd.ident.id);
	    macro_splice(p,v);
	    break;
	case nop_name_subst:
	    v = macro_variable_value(n);
	    if (! is_name(v))
		error("Value of $%s is not a name", n->nd.ident.id);
	    macro_splice(p,v);
	    break;
	case nop_expr_subst:
	    v = macro_variable_value(n);
	    if (! is_expr(v))
		error("Value of $%s is not an expression", n->nd.ident.id);
	    macro_splice(p,v);
	    break;
	case nop_stmt_subst:
	    v = macro_variable_value(n);
	    if (! is_stmt(v))
		error("Value of $%s is not a statement", n->nd.ident.id);
	    macro_splice(p,v);
	    break;
	case nop_type_subst:
	    v = macro_variable_value(n);
	    if (! is_type(v))
		error("Value of $%s is not a type", n->nd.ident.id);
	    macro_splice(p,v);
	    break;
	case nop_subtype_subst:
	    v = macro_variable_value(n);
	    if (! is_type(v))
		error("Value of $%s is not a subtype", n->nd.ident.id);
	    macro_splice(p,v);
	    break;
	case nop_ident_subst:
	    {
	        nodep vv;

	    	v = macro_variable_value(n);
	    	if (! is_ident(v))
		    error("Value of $%s is not an identifier", 
			n->nd.ident.id);
	        vv = newnode(id_decl_node, nop_id_decl);
		vv->nd.id_decl.id = v->nd.ident.id;
	        macro_splice(p,vv);
	    }
	    break;
    }
}

long macro_substitute(p)
    nodep *p;
{
    extern long nill();
    extern nodep maptreelist();

    *p = maptreelist(*p, pre_macro_subst, nill, nill, nill);
}

static long expand_pre(n,p)
    nodep n, *p;
{
    typedval result;
    long len;
    register nodep ex, q, tail;
    boolean defined;			/* true if macro is defined */        
    switch (n->ndf.op) {
	case nop_stmt_callf:
	case nop_decl_callf:
	case nop_expr_callf:
	case nop_type_callf:
	case nop_name_callf:
	case nop_top_callf:
	    tail = n->ndf.next;
	    defined = eval(n, macro_mode, &result, null);
	    if (!defined) return;
	    ex = result.val.nptr;

	    len = 1;
	    for (q=ex; q->ndf.next != null; q = q->ndf.next)
	    len++;
            q->ndf.next = tail;

    	    n->nd.call_form.items = len;
	    n->nd.call_form.refined_to = ex;
	    n->ndf.next = null;

    	    insert_attach(ex,n);
	    *p = ex;
    }
}

static long do_macro_expansion(p)
    nodep *p;
{
    extern nodep maptree();
    extern long nill();
    if (*p == null) system_error("No internal form");
    *p = maptree(*p, expand_pre, nill, nill, nill);
}

long expand(f)
    register fixp f;
{
    switch (f -> fxf.class) {
	case func_id: 
	case proc_id: 
	case macro_id:
	    do_macro_expansion(&(f->fx.func.int_form));
	    break;
	case type_id: 
	case subtype_id: 
	    do_macro_expansion(&(f->fx.var.int_form));
	    break;
	case obj_id: 
	    do_macro_expansion(&(f->fx.var.int_form));	    
	    break;
	case pack_id: 
 	    if (&(f->fx.pack.int_form)) {
		do_macro_expansion(&(f->fx.pack.int_form));
		if (&(f->fx.pack.body_int_form))
		    do_macro_expansion(&(f->fx.pack.body_int_form));
	    }
	    break;
	default: 
	    error("can't expand this class");
    }
}

  /* The following routines unexpand all macro expansions in an internal
     form tree.  The strange mixture of recursion and iteration in 
     unexpand_pre is there because nested calling forms must be undone
     in the right order.  If all expansions on a list (of statements, 
     declarations, etc. were undone starting from the end of this list,
     everything would work.  Unfortunately, maptree does not provide a
     way to do this, and to do so would make maptree unusable for ifp.
     Therefore the method chosen for now is to recursively check for
     expansions inside of each other expansion.   Since each expansion
     ultimately reduces to a single node, little is lost in efficiency
     but the algorithm is more complex.  */

       long unexpand_pre(n,p)
    nodep n, *p;
{
    register long i;
    register nodep c, *q;

    while (true) {
	if ((c = extract_attach (n, nop_stmt_callf)) == null)
	    if ((c = extract_attach (n, nop_decl_callf)) == null)
		if ((c = extract_attach (n, nop_type_callf)) == null)
		    if ((c = extract_attach (n, nop_name_callf)) == null)
			if ((c = extract_attach(n,nop_expr_callf)) == null)
			return;
    				/* could be more efficient */

	q = p;
	for (i = 1; i < c -> nd.call_form.items; i++)
	    if (*q) {
		q = &((*q) -> ndf.next);
	        unexpand_pre (*q, q);
	    }
	c -> ndf.next = (*q) -> ndf.next;
	*p = n = c;
    }
}

static long do_macro_unexpand(p)
    nodep *p;
{
    extern nodep maptree();
    extern long nill();

    if (*p == null) system_error("No internal form");
    *p = maptree(*p, unexpand_pre, nill, nill, nill);
}

long unexpand(f)
    register fixp f;
{
    switch (f -> fxf.class) {
	case func_id: 
	case proc_id: 
	case macro_id:
	    do_macro_unexpand(&(f->fx.func.int_form));
	    break;
	case type_id: 
	case subtype_id: 
	    do_macro_unexpand(&(f->fx.var.int_form));
	    break;
	case obj_id: 
	    do_macro_unexpand(&(f->fx.var.int_form));	    
	    break;
	case pack_id: 
 	    if (&(f->fx.pack.int_form)) {
		do_macro_unexpand(&(f->fx.pack.int_form));
		if (&(f->fx.pack.body_int_form))
		    do_macro_unexpand(&(f->fx.pack.body_int_form));
	    }
	    break;
	default: 
	    error ("can't unexpand this class");
    }
}
