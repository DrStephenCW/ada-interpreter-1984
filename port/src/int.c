static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/int.v  Revision 1.9  83/11/30  14:06:23  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/int.v $
 * Revision 1.9  83/11/30  14:06:23  ada
 *
 * [RECONSTRUCTED 2026.  The original int.c did not survive on tape.
 *  This file was rebuilt from the compiled code and debugging symbols in
 *  the Arcturus 1.6-A executable "ada" (January 5, 1984).  Every line
 *  that generates code is placed on the line number the 1984 compiler
 *  recorded for it.  Names of functions, parameters, locals and globals
 *  are the original ones.  Comments, blank lines, the label "again",
 *  the names of the three operator macros, and lines 922-927 (compiled
 *  out of the binary) are not recoverable and are written fresh.
 *  The earlier RCS log entries are lost.]
 */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"
#include "adaio.h"
#include "count.h"
#include "handler.h"
#include "oblist.h"

/*	The interpreter.  eval() walks the internal form produced by the
	parser.  Declarations enter symbols in the current symbol table,
	statements are executed for their effect, and expressions leave a
	typed value in *result.

	intstk is a stack of the nodes being evaluated, used by the
	debugger and by the break loop to say where we are.	*/

extern	nodep goto_p;		/* set by a goto to the statement to resume at */

nodep	intstk[5000];
nodep	*intsp = intstk - 1;
nodep	*intstk_end = &intstk[5000];
retsp	retstk = null;

extern	long stack_level;	/* depth of the dynamic symbol stack */

symtabp	parameter_table;	/* formal parameters are entered here */

fixp	eval();
extern	typep tyeval();
extern	char *zalloc();
extern	nodep mkidnode();
extern	nodep dogoto();
extern	char *macro_pname();

/*	Binary operators.  All of them evaluate both operands first, so
	they are written once here.					*/

#define arith_op(OP) \
	eval(p->nd.binop.left, eval_mode, &l, null); \
	eval(p->nd.binop.right, eval_mode, &r, null); \
	typecheck(r.typ, l.typ); \
	if (compatible(r.typ, inttype)) \
		result->val.integer = l.val.integer OP r.val.integer; \
	else if (compatible(r.typ, realtype)) \
		result->val.real = l.val.real OP r.val.real; \
	else error("non-numeric type in arithmetic operator"); \
	result->typ = base(r.typ)

#define rel_op(OP) \
	eval(p->nd.binop.left, eval_mode, &l, null); \
	eval(p->nd.binop.right, eval_mode, &r, null); \
	typecheck(r.typ, l.typ); \
	if (compatible(r.typ, inttype)) \
		result->val.integer = l.val.integer OP r.val.integer; \
	else if (compatible(r.typ, realtype)) \
		result->val.integer = l.val.real OP r.val.real; \
	else error("non-numeric type in relational operator"); \
	result->typ = booltype

#define bool_op(OP) \
	eval(p->nd.binop.left, eval_mode, &l, null); \
	eval(p->nd.binop.right, eval_mode, &r, null); \
	typecheck(booltype, l.typ); \
	typecheck(booltype, r.typ); \
	result->val.integer = l.val.integer OP r.val.integer; \
	result->typ = booltype








































long clearinterp() {
	intsp = intstk - 1;
	retstk = null;
}


long copy(dst, src, n)
char *dst, *src;
long n;
{
	while (n--)
		*dst++ = *src++;
}


long interp(p) nodep p;
{
	eval(p, decl_mode, &dummy, null);
}


long exprinterp(p) nodep p;
{
	eval(p, eval_mode, &dummy, null);
}

/*	eval -- evaluate node p in the given mode.  Returns a fixp for
	declarations (the list of names declared) and null otherwise.	*/








fixp eval(p, mode, result, expected_type)
register nodep p;
enum int_modes mode; typedval *result;
typep expected_type;
{
	if (intsp++ == intstk_end) {
		putf("\nInterpreter stack overflow\n");
		intsp = intstk;
	}
	/* statements in a list are done by going back to the
	   top with p set to the next one */
again:	check_control_c();

	/* an empty list: nothing to do.  The node stack is
	   popped on every way out of eval */


	if (p == null) {

		intsp--;
		return null;
	}
	if (counting)
		increment_node_counter(p);

	/* trace */
	if (intdebug)
		putf("[node %s]  ", nodename(p));
	*intsp = p;

	/* each case either breaks out of the switch (an expression,
	   whose value is in *result), returns (a declaration of names
	   that the caller will enter), or goes back to "again" with
	   p set to the next statement in the list.



	   Case labels are in the order of the tokentypes enumeration
	   wherever that does not matter.  */


	switch (p->ndf.op) {

	case nop_expr: {
		typedval e;

		/* an expression typed at top level: print its value */
		eval(p->nd.expr.expr, eval_mode, &e, null);
		typedprint(adaout, e.val.integer, e.typ);
		fflush(adaout);
		break;
	}
	case nop_with:
		/* with clauses: every library unit is
		   always visible, so there is nothing
		   to do here.  */




		break;
	case nop_use:
		eval(p->nd.usex.list, decl_mode, &dummy, null);
		p = p->ndf.next;
		goto again;
	case nop_name: {
		typedval pack;
		eval(p->nd.name_list.name, symbol_mode, &pack, null);
		if (!compatible(pack.typ, symboltype))
			error("illegal name in use declaration");
		use_sym(pack.val.fptr, currtab);
		p = p->ndf.next;
		goto again;
	}
	case nop_type:
	case nop_subtype: {
		register symp s;
		register fixp f;
		typep t;
		enum symbolclasses class;
		class = (p->ndf.op == nop_type) ? type_id : subtype_id;

		/* look for an earlier, incomplete declaration */

		f = lookuptab(currtab, p->nd.type_decl.id, false);

		if (p->nd.type_decl.typex == null) {
			/* an incomplete type: "type t;".  Make
			   an undefined type for it, unless it
			   is already declared at this
			   level. */






			if (f == null || f->fxf.dyna == null || f->fxf.dyna->syf.level != stack_level) {

				f = newfix(varfixform, class);
				s = newsym(varsymbolform, class);
				f->fxf.name = p->nd.type_decl.id;
				s->sy.var.type = newtype(undefform);
				enter(currtab, f, s);
			}
			else {
				s = f->fxf.dyna;
				if (f->fx.var.int_form)
					/* it had a full declaration */
					sysputf("%s undefined", f->fxf.name);
			}
			f->fx.var.int_form = null;
		}
		/* a full type declaration */
		else {

			t = tyeval(p->nd.type_decl.typex, p->nd.type_decl.disc, true);
			t->tyf.int_form = mkidnode(p->nd.type_decl.id);
			/* if there was an incomplete
			   declaration, the new type is
			   copied over the old one so
			   that everything which points
			   to the old type now sees the
			   full one.  Otherwise make a
			   new symbol.  */


			if (f == null || f->fxf.dyna == null || f->fxf.dyna->syf.level != stack_level) {

				f = newfix(varfixform, class);
				s = newsym(varsymbolform, class);
				f->fxf.name = p->nd.type_decl.id;
				s->sy.var.type = t;
				enter(currtab, f, s);
			}
			else {
				if (f->fx.var.int_form)
					sysputf("%s redefined", f->fxf.name);

				copy(f->fxf.dyna->sy.var.type, t, maxtypesize);

				f->fxf.class = class;
			}
			f->fx.var.int_form = p->nd.type_decl.typex;
		}
		p = p->ndf.next;
		goto again;
	}
	case nop_id_decl: {
		register fixp f;
		/* a list of names being declared: make
		   a fixform for each and return the
		   list to the declaration that
		   contains it */
		if (mode != decl_mode)
			system_error("bad mode for id_decl");
		f = newfix(fixform, obj_id);
		f->fxf.name = p->nd.id_decl.id;
		/* the rest of the list */
		f->fxf.next = eval(p->ndf.next, decl_mode, &dummy, null);
		intsp--;
		return f;
	}
	case nop_obj_decl: {
		typep typ;
		register fixp f, temp;
		register symp s;
		typedval init;

		/* get the names, then the type,
		   then the initial value */
		f = eval(p->nd.obj_decl.id_list, decl_mode, &dummy, null);

		typ = tyeval(p->nd.obj_decl.subtypex, null, false);

		/* the initial value is evaluated once,
		   and copied into each object */
		if (p->nd.obj_decl.initexp) {
			eval(p->nd.obj_decl.initexp, eval_mode, &init, typ);
			rangecheck(typ, &init);
		}
		else
			init.val.integer = 0;
		if (typ->tyf.class == un_arrayform) {
			if (p->nd.obj_decl.initexp)
				typ = init.typ;
			else
				error("object must be constrained");
		}
		while (f) {
			s = newsym(varsymbolform, obj_id);
			s->sy.var.type = typ;
			/* remember the declaration */
			f->fx.var.int_form = p->nd.obj_decl.subtypex;
			if (typ == texttype) {
				char name[128];
				savename(name, f);
				*s->sy.var.value = (long)strsav(name);
			}
			else
				if (blocktype(typ)) {
					/* arrays and records get
					   their own space */
					s->sy.var.value = (long*)zalloc(typ->tyf.size, true);
					if (p->nd.obj_decl.initexp)
						blockmove(s->sy.var.value,
						    init.val.ptr,
						    s->sy.var.type->tyf.size);
				}
				else
					*s->sy.var.value = init.val.integer;
			/* enter frees nothing, so save the link */
			temp = f->fxf.next;
			enter(currtab, f, s);
			f = temp;
		}
		p = p->ndf.next;
		goto again;
	}
	case nop_except_decl: {
		register fixp f, e, temp;
		register symp s;

		/* names come back as object fixforms */
		f = eval(p->nd.except_decl.id_list, decl_mode, &dummy, null);
		/* replace each with an exception
		   fixform */

		while (f) {
			temp = f->fxf.next;
			/* smaller than an object's */
			e = newfix(exceptfixform, except_id);
			/* keep the name */
			e->fxf.name = f->fxf.name;
			free(f);
			s = newsym(exceptsymbolform, except_id);
			enter(currtab, e, s);
			f = temp;
		}
		p = p->ndf.next;
		goto again;
	}
	case nop_param:
	case nop_macro_param: {
		register fixp f, temp;
		register symp s;
		typep typ;
		fixp front, paramlist;
		nodep initval;
		boolean hasval;
		/* A formal parameter declaration, e.g.
			a, b : in out integer := 0

		   is a list of names with a mode, a type
		   and a default value.  Each name is entered
		   in parameter_table and linked through
		   param_link to the next one, so that docall
		   can match actuals to formals in order.
		   The later declarations in the formal part
		   are evaluated first, so that the last name
		   here can be linked to them.

		   Returns the first name.
		*/

		paramlist = eval(p->ndf.next, decl_mode, &dummy, null);

		/* the names */
		front = f = eval(p->nd.param_decl.id_list, decl_mode, &dummy, null);

		/* the type */
		typ = tyeval(p->nd.param_decl.subtyp, null, false);

		hasval = false;
		initval = null;
		if (p->nd.param_decl.initexp) {
			if (p->nd.param_decl.is_out)
				error("default for out param");
			/* evaluated at call time */
			hasval = true;
			initval = p->nd.param_decl.initexp;
		}
		/* the list of names is never empty,
		   so test at the bottom */
		for (;;) {
			s = newsym(varsymbolform, param_id);
			f->fxf.class = param_id;
			s->sy.var.type = typ;
			/* what docall needs to know */
			f->fx.var.param_has_value = hasval;
			f->fx.var.int_form = initval;
			f->fx.var.param_base_type = typ;
			f->fx.var.param_is_in = p->nd.param_decl.is_in;
			f->fx.var.param_is_out = p->nd.param_decl.is_out;
			f->fx.var.param_link = f->fxf.next;
			enter(parameter_table, f, s);
			if (f->fx.var.param_link == null) {
				f->fx.var.param_link = paramlist;
				intsp--;
				return front;
			}
			f = f->fx.var.param_link;
		}
	}
	case nop_in: {
		typedval l;
		typep rtyp;
		long resval;
		/* "x in a..b" and "x not in a..b" */


		eval(p->nd.inop.expr, eval_mode, &l, null);

		rtyp = tyeval(p->nd.inop.rangex, null, false);

		typecheck(l.typ, rtyp);
		/* only scalar ranges */

		resval = rtyp->ty.srange.lbound <= l.val.integer && l.val.integer <= rtyp->ty.srange.ubound;

		if (p->nd.inop.not_in)
			resval = !resval;
		result->val.integer = resval;
		result->typ = booltype;

		break;
	}
	case nop_asgn: {
		typedval l, r;
		fixp f;
		symp s;
		nodep lp, rp;
		/* An assignment.  At top level an
		   assignment to an undeclared name
		   declares it, with the type of the
		   value assigned.  */
		lp = p->nd.asgn_stmt.lhs;
		rp = p->nd.asgn_stmt.rhs;
		if (lp->ndf.op == nop_ident) {
			f = lookup(currtab, lp->nd.ident.id, false);
			if (f == null) {
				if (currtab == usrtab) {
					/* autodefine */
					eval(rp, eval_mode, &r, null);
					f = newfix(varfixform, obj_id);
					s = newsym(varsymbolform, obj_id);
					f->fxf.name = lp->nd.ident.id;
					if (r.typ->tyf.int_form == null)
						error("%s undefined", lp->nd.ident.id);
					l.typ = s->sy.var.type = r.typ;
					if (blocktype(r.typ))
						error("Can't autodefine non-scalar type");
					else
						l.val.ptr = s->sy.var.value;
					/* tell the user what was declared */
					f->fx.var.int_form = r.typ->tyf.int_form;
					enter(currtab, f, s);
					putf("\n(%s: ", f->fxf.name);
					dopp(f->fx.var.int_form, adaout, 0L, 0L);
					putf(")\n");
				}
				else
					error("%s undefined as lhs of assignment", lp->nd.ident.id);
			}
			else {
				s = f->fxf.dyna;
				eval(rp, eval_mode, &r, s->sy.var.type);
				doident(f, addr_mode, &l);
			}
		}
		else {
			/* the right side is evaluated
			   first, so that the address of
			   the left side can't be changed
			   by it */

			eval(rp, eval_mode, &r, null);
			eval(lp, addr_mode, &l, null);
		}
		rangecheck(l.typ, &r);
		if (blocktype(l.typ))
			blockmove(l.val.ptr, r.val.ptr, l.typ->tyf.size);
		else
			*l.val.ptr = r.val.integer;
		p = p->ndf.next;
		goto again;
	}
	/* constants */
	case nop_int:

		if (mode != eval_mode)
			error("Can't use integer constant where address req'd");
		result->val.integer = p->nd.integer.val;
		result->typ = inttype;
		break;
	case nop_char:
		if (mode != eval_mode)
			error("Can't use character constant where address req'd");
		result->val.integer = p->nd.integer.val;
		result->typ = chartype;
		break;
	case nop_real:
		if (mode != eval_mode)
			error("Can't use float constant where address req'd");
		result->val.real = p->nd.real.val;
		result->typ = realtype;
		break;
	case nop_string: {
		register typedval *str;

		/* A string constant is converted
		   once per command (generation) and
		   the result hung on the node, the
		   same trick that nop_ident uses for
		   symbols.  */

		str = (typedval*)p->nd.ident.curr_binding;
		if (p->nd.ident.gen_number != curr_generation || str == null) {
			str = (typedval*)zalloc(sizeof(typedval), false);
			dostring(p->nd.ident.id, str);
			/* remember it */
			p->nd.ident.curr_binding = (long*)str;
			p->nd.ident.gen_number = curr_generation;
		}
		result->typ = str->typ;
		result->val = str->val;
		break;
	}
	case nop_ident: {
		register fixp f;
		register symp s;
		/* the binding is good for one command */
		f = (fixp)p->nd.ident.curr_binding;
		if (p->nd.ident.gen_number != curr_generation || f == null) {
			if ((f = lookup(currtab, p->nd.ident.id, false)) == null)
				error("undefined: %s", p->nd.ident.id);
			/* lookup is slow, so remember
			   what it found */
			p->nd.ident.curr_binding = (long*)f;
			p->nd.ident.gen_number = curr_generation;
		}
		doident(f, mode, result);
		break;
	}
	case nop_null:
		/* the null access value, compatible
		   with every access type */
		result->typ = nulltype;
		result->val.ptr = null;
		break;
	case nop_new: {
		typep ptr;

		/* allocator */
		ptr = newtype(accessform);
		ptr->tyf.size = sizeof(long*);
		ptr->ty.access.targettype =
			lookuptypeid(p->nd.newx.type_id->nd.ident.id, true);
		result->typ = ptr;
		/* new object, cleared */
		result->val.ptr =
			(long*)zalloc(ptr->ty.access.targettype->tyf.size, true);
		break;
	}
	case nop_lp:
		/* parenthesized expression */

		eval(p->nd.uop.child, mode, result, expected_type);
		break;
	/* arithmetic */

	case nop_plus:
		{ typedval l, r; arith_op(+); break; }
	case nop_minus:
		{ typedval l, r; arith_op(-); break; }
	case nop_star:
		{ typedval l, r; arith_op(*); break; }
	case nop_div:
		{ typedval l, r; arith_op(/); break; }

	case nop_mod: {
		typedval l, r;
		eval(p->nd.binop.left, eval_mode, &l, null);
		eval(p->nd.binop.right, eval_mode, &r, null);
		typecheck(r.typ, inttype);
		typecheck(l.typ, inttype);
		result->val.integer = l.val.integer % r.val.integer;
		result->typ = inttype;
		break;
	}
	/* comparison */
	case nop_equal:
	case nop_nequal: {
		typedval l, r;
		long res;

		eval(p->nd.binop.left, eval_mode, &l, null);
		eval(p->nd.binop.right, eval_mode, &r, null);
		typecheck(l.typ, r.typ);
		if (blocktype(l.typ))
			res = blockcompare(l.val.ptr, r.val.ptr, l.typ->tyf.size);
		else
			res = l.val.integer == r.val.integer;
		result->typ = booltype;
		result->val.integer = (res == (p->ndf.op == nop_equal));
		break;
	}
	case nop_lt:
		{ typedval l, r; rel_op(<); break; }
	case nop_le:
		{ typedval l, r; rel_op(<=); break; }
	case nop_gt:
		{ typedval l, r; rel_op(>); break; }
	case nop_ge:
		{ typedval l, r; rel_op(>=); break; }
	case nop_and:
		{ typedval l, r; bool_op(&); break; }
	case nop_or:
		{ typedval l, r; bool_op(|); break; }
	/* short circuit forms */
	case nop_andthen: {
		typedval r;

		eval(p->nd.binop.left, eval_mode, &r, null);
		typecheck(r.typ, booltype);
		if (r.val.integer == true) {
			eval(p->nd.binop.right, eval_mode, &r, null);
			typecheck(r.typ, booltype);
		}
		result->val.integer = r.val.integer;
		result->typ = booltype;
		break;
	}
	case nop_orelse: {
		typedval r;

		eval(p->nd.binop.left, eval_mode, &r, null);
		typecheck(r.typ, booltype);
		if (!r.val.integer) {
			eval(p->nd.binop.right, eval_mode, &r, null);
			typecheck(r.typ, booltype);
		}
		result->val.integer = r.val.integer;
		result->typ = booltype;
		break;
	}
	case nop_amper: {
		typedval l, r;
		/* catenation of arrays (strings) */



		eval(p->nd.binop.left, eval_mode, &l, null);
		eval(p->nd.binop.right, eval_mode, &r, null);
		catenate(l, r, result);
		/* catenate sets result->typ */
		break;
	}
	case nop_not: {
		typedval r;
		/* unary operators */
		eval(p->nd.uop.child, eval_mode, &r, null);
		typecheck(r.typ, booltype);
		result->val.integer = !r.val.integer;
		result->typ = booltype;
		break;
	}
	case nop_neg: {
		typedval r;

		eval(p->nd.uop.child, eval_mode, &r, null);
		if (compatible(r.typ, inttype))
			result->val.integer = -r.val.integer;
		else
			if (compatible(r.typ, realtype))
				result->val.real = -r.val.real;
			else
				error("numeric type expected in negation");
		result->typ = r.typ;
		break;
	}
	case nop_pos: {
		typedval r;

		eval(p->nd.uop.child, eval_mode, &r, null);
		if (!compatible(r.typ, inttype) &&
		    !compatible(r.typ, realtype))
			error("numeric type expected in unary plus");
		result->val.integer = r.val.integer;
		result->typ = r.typ;
		break;
	}
	/* statements */
	case nop_null_stmt:

		p = p->ndf.next;
		goto again;
	case nop_if:
		/* a goto out of the statement
		   leaves goto_p set */
		doif(p);
		if (goto_p) {
			p = goto_p;
			goto_p = null;
			goto again;
		}
		p = p->ndf.next;
		goto again;
	case nop_elsif: {	typedval r;
		/* called from doif with the elsif parts
		   (the else part has no condition) */
		while (p) {
			/* the else part */
			if (p->nd.elsifx.expr == null) {
				/* stmts in the then_part
				   are executed in the
				   context of the if */
				eval(p->nd.elsifx.then_part, eval_mode, &dummy, null);
				intsp--;
				return;
			}
			eval(p->nd.elsifx.expr, eval_mode, &r, null);
			typecheck(r.typ, booltype);
			if (r.val.integer) {
				/* the first true condition
				   wins
				*/
				eval(p->nd.elsifx.then_part, eval_mode, &dummy, null);
				intsp--;
				return;
			}
			p = p->ndf.next;
		}
		break;
	}
	case nop_while:
		dowhile(p);
		if (goto_p) {
			p = goto_p;
			goto_p = null;
			goto again;
		}
		p = p->ndf.next;
		goto again;
	case nop_loop:

		doloop(p);
		if (goto_p) {
			p = goto_p;
			goto_p = null;
			goto again;
		}
		p = p->ndf.next;
		goto again;
	case nop_for:

		dofor(p);
		if (goto_p) {
			p = goto_p;
			goto_p = null;
			goto again;
		}
		p = p->ndf.next;
		goto again;
	case nop_exit:
		/* doexit does not return if the exit is taken */
		doexit(p);
		p = p->ndf.next;
		goto again;
	case nop_return: {
		retsp rets;
		rets = retstk;
		/* find the subprogram or macro being returned from */
		while (rets) {
			enum tokentypes op = rets->p->ndf.op;
			if (op == nop_function ||
			    op == nop_procedure ||
			    op == nop_macro) break;
			rets = rets->link;
		}
		if (rets == null)
			error("return is not in a subprogram or macro");
		if (p->nd.returnx.val)
			eval(p->nd.returnx.val, eval_mode, retval, null);
		unwind_retstk(rets, true);
	}
		/* unwind_retstk does not return */

	case nop_goto:
		p = dogoto(p);
		p = p->ndf.next;
		goto again;
	case nop_lb:
		/* a label */
		p = p->ndf.next;
		goto again;
	/* packages */

	case nop_package:
		dopackspec(p);
		/* the spec is a declaration */
		p = p->ndf.next;
		goto again;
	case nop_body:

		dopackbody(p);
		p = p->ndf.next;
		goto again;
	case nop_function:
	case nop_procedure: {
		fixp f;
		symp s;
		enum symbolclasses class;
		/* A subprogram declaration (with or without
		   a body).  Subprograms declared at top level
		   are written out to usr so that they can be
		   edited and saved.  */

		if (mode == decl_mode) {	boolean write_out; fixp owner;
			write_out = false;
			owner = currtab->symtab_owner;

			if (p->ndf.op == nop_function)
				class = func_id;
			else
				class = proc_id;
			/* A body for a subprogram that was
			   declared in a package specification
			   goes with the declaration in the
			   package's table.  */








			if (owner && owner->fxf.class == pack_id && owner->fx.pack.int_form->nd.packagex.table && (f = lookuptab(owner->fx.pack.int_form->nd.packagex.table, p->nd.subp.name, false))) {
				s = f->fxf.dyna;
			}
#ifdef notdef
			/* (six lines here were compiled out
			    of the 1984 binary; their text is
			    lost)
			*/


#endif
			/* a subprogram redeclared at top
			   level replaces the old one */

			else

				if (currtab == usrtab && (f = lookuptab(currtab, p->nd.subp.name, false))) {
					s = f->fxf.dyna;
					f->fxf.class = class;
					if (f->fx.func.int_form) {
						/* free the old tree */
						putf("Warning: %s redefined in 'usr'\n", p->nd.subp.name);
						free_tree(f->fx.func.int_form);
						f->fx.func.int_form = null;
					}
					write_out = true;
				}
				else {
					if (currtab == usrtab) write_out = true;
					/* a new subprogram */
					f = newfix(funcfixform, class);
					s = newsym(funcsymbolform, class);
					f->fxf.name = p->nd.subp.name;
					enter(currtab, f, s);
				}
			/* the internal form is the
			   subprogram: it is evaluated
			   again for each call */
			f->fxf.class = class;
			f->fx.func.int_form = p;
			if (write_out)
				subp_writeout(f);
			if (class == func_id) {
				/* the return type */
				s->sy.func.type = tyeval(p->nd.subp.ret_type, null, false);
				f->fxf.class = func_id;
			}
			else
				s->sy.func.type = undeftype;
			if (p->nd.subp.table == null)
				p->nd.subp.table = newtab(32L, f);
			/* scope for the body */
			p->nd.subp.table->symtab_static_link = currtab;
			p = p->ndf.next;
			goto again;
		}
		else
			system_error("bad mode in eval in nop_func");
	}
	/* calls */
	case nop_proc_call:
		docall(p, mode, result);
		p = p->ndf.next;
		goto again;
	case nop_call:

		if (p->nd.call.var->ndf.op == nop_quote)
			/* an attribute with an argument */
			doattr2(p, result);
		/* a function call or an indexed name */
		else	docall(p, mode, result);
		break;
	case nop_arg: {
		long i;

		/* evaluate actual parameters into result[] */
		i = 0;
		/* the node stack shows the
		   argument being evaluated */

		while (p) {
			eval(p->nd.arg.exp, eval_mode, &result[i++], null);
			if (i >= MAXARGS-1)
				error("too many arguments");
			p = p->ndf.next;
			*intsp = p;
		}
		break;
	}
	case nop_declare:
		/* a block */

		doblock(p);
		if (goto_p) {
			p = goto_p;
			goto_p = null;
			goto again;
		}
		p = p->ndf.next;
		goto again;
	case nop_dot:
		/* selected component, or a name
		   in a package */
		dodot(p, mode, result);
		break;
	case nop_quote:

		doattr(p, result);
		break;
	case nop_pragma:

		dopragma(p);
		p = p->ndf.next;
		goto again;
	case nop_range:
		result->typ = typetype;
		result->val.tptr = tyeval(p, null, false);
		break;
	case nop_qual_expr: {
		typep t;
		t = tyeval(p->nd.aggregate.qualifier, null, false);
		eval(p->nd.aggregate.agg, eval_mode, result, t);
		rangecheck(t, result);
		result->typ = t;
		break;
	}
	case nop_aggregate: {
		typep t;
		if (p->nd.aggregate.qualifier)
			t = tyeval(p->nd.aggregate.qualifier, null, false);
		else t = expected_type;
		if (t == null)
			error("type of aggregate not given by context");
		agg_eval(p->nd.aggregate.agg, result, t);
		break;
	}
	case nop_case:
		/* case statement */
		docase(p, mode, result);
		if (goto_p) {
			p = goto_p;
			goto_p = null;
			goto again;
		}
		p = p->ndf.next;
		goto again;
	case nop_obj_ren: {
		typedval old;
		fixp f;
		symp s;
		typep typ;

		f = eval(p->nd.obj_rename.id_list, decl_mode, &dummy, null);
		s = newsym(varsymbolform, obj_id);
		typ = tyeval(p->nd.obj_rename.typex, null, false);
		eval(p->nd.obj_rename.old_name, addr_mode, &old, null);
		typecheck(typ, old.typ);
		s->sy.var.value = old.val.ptr;
		s->sy.var.type = old.typ;
		/* the new name shares the old object */
		f->fx.var.int_form = p;
		enter(currtab, f, s);
	}
		p = p->ndf.next;
		goto again;
	case nop_raise:
		/* raise does not return */

		raise(p->nd.raisex.except);
	case nop_macro:
		/* macro declaration (Castor) */




		domacro(p, mode);
		p = p->ndf.next;
		goto again;
	case nop_macro_desig:
		system_error("Macro designator");
	case nop_macro_param_list:
		system_error("Macro parameter list");
	case nop_macro_arr_param: {
		fixp paramlist;
		symp paramsym;


		paramlist = eval(p->nd.param_decl.id_list, decl_mode, &dummy, null);
		paramlist->fxf.class = param_id;
		paramlist->fx.var.param_is_out = true;
		paramsym = newsym(varsymbolform, obj_id);
		enter(parameter_table, paramlist, paramsym);
		intsp--;
		return paramlist;
	}
	case nop_expr_callf:
	case nop_name_callf:
	case nop_type_callf: {
		long defined;
		/* macro calls in expressions */
		defined = macro_call(p, mode, result);
		if (mode != macro_mode && mode != symbol_mode) {
			if (defined) {
				dyn_macro_expand(result->val.nptr, p);
				p = result->val.nptr;
				eval(p, mode, result, null);
			}
			else error("macro {%s} is not defined", macro_pname(p));
		}
		intsp--;
		return (fixp)defined;
	}
	case nop_decl_callf:
	case nop_stmt_callf:
	case nop_top_callf: {
		long defined;
		/* macro calls in declarations and
		   statements: the expansion is spliced
		   into the list in place of the call,
		   so evaluation carries on with it */
		defined = macro_call(p, mode, result);
		if (mode != macro_mode && mode != symbol_mode) {
			if (defined) {
				dyn_macro_expand(result->val.nptr, p);
				p = result->val.nptr;
				goto again;
			}
			else {
				/* let the user define it */
				putf("macro {%s} is not defined", macro_pname(p));
				ada_break(true);
				p = p->ndf.next;
				goto again;
			}
		}
		intsp--;
		return (fixp)defined;
	}
	case nop_callf_arg_list:
		system_error("Calling form argument list.");
	case nop_callf_ident:
		system_error("Calling form identifier.");
	case nop_callf_stmt_arg:
		system_error("Statement arguments not implemented.");
	case nop_decl_subst:
	case nop_ident_subst:
	case nop_name_subst:
	case nop_stmt_subst:
	case nop_subtype_subst:
	case nop_type_subst:
		error("Substitution point not in macro.");




	case nop_abort:
	case nop_accept:
	case nop_access:
	case nop_addr_spec:
	case nop_agg_comp:
	case nop_array:
	case nop_at:
	case nop_char_enum:
	case nop_choice:
	case nop_code:
	case nop_cond_ent:
	case nop_delay:
	case nop_delta:
	case nop_derived_type:
	case nop_digits:
	case nop_disc_type:
	case nop_discriminant:
	case nop_entry:
	case nop_enum:
	case nop_err:
	case nop_except_ren:
	case nop_expon:
	case nop_func_ginst:
	case nop_func_ren:
	case nop_gen_with:
	case nop_generic:
	case nop_index:
	case nop_instant:
	case nop_len_spec:
	case nop_limited:
	case nop_oper_string:
	case nop_pack_ginst:
	case nop_pack_ren:
	case nop_private:
	case nop_proc_ginst:
	case nop_proc_ren:
	case nop_rec_spec:
	case nop_rec_var:
	case nop_rec_when:
	case nop_record:
	case nop_rem:
	case nop_sel_when:
	case nop_select:
	case nop_subunit:
	case nop_task:
	case nop_task_body:
	case nop_task_ren:
	case nop_task_type:
	case nop_terminate:
	case nop_timed_ent:
	case nop_when:
	case nop_xor:
		error("You've used an unimplemented construct whose internal name is %s", nodename(p));
	default:
		system_error("invalid node at %o in eval", p);
	}
	intsp--;
}
