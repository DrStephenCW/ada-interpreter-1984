static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/pp.v  Revision 1.7  83/11/30  14:29:55  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/pp.v $
 * Revision 1.7  83/11/30  14:29:55  ada
 * same as 1.6.5.1
 * 
 * Revision 1.6.5.1  83/11/29  16:14:13  snider
 * Replaced a = with a ==
 * 
 * Revision 1.6  83/06/14  16:34:57  ada
 * Merged 1.5.4.1 and 1.5.5.1.
 * 
 * Revision 1.5.5.1  83/05/26  14:10:22  snider
 * Fixed a problem with prettyprinting the reserved word "range".
 * 
 * Revision 1.5.4.1  83/06/14  14:12:59  tadman
 * Deleted case for comp_unit_node.  Added code to print context
 * specifications.
 * 
 * Revision 1.5  83/03/10  16:01:50  ada
 * same as 1.4.2.1
 * 
 * Revision 1.4.2.1  83/03/10  14:55:10  willson
 * Export stuff added.
 * 
 * Revision 1.3  83/01/12  09:07:51  ada
 * Added copyright notice.
 * 
 * Revision 1.2  83/01/04  14:21:31  willson
 * Added new parameter "export"; if true, the empty paren's around
 * function calls w/o arguments are removed for compatibility
 * with ada82.
 * 
 * Revision 1.1  82/12/12  21:40:28  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "count.h"
#include "adaio.h"
#include "pp.h"

 
extern FILE  *ppfile;
extern boolean pp_first;
extern pp_tok(), pp_cut();
extern color_name pp_color();

static int subplevel;
static int indent;			/* number of columns to indent each 
					   level */
static boolean tracefl;
static nodep q;
static boolean export;
static boolean nroffl;
static enum pp_color_use color_use;
static color_name current_color;
static enum pp_capitalize cap_resv;	/* type of capitalization for
					   reserved words */
static enum pp_capitalize cap_idnt;	/* type of capitalization of
					   identifiers */
int pp_depth;

/****************************************************************/
/*								*/
/*  prprint is the main entry point to the prettyprinter.	*/
/*  The parameters are:						*/
/*								*/
/*  p	pointer to the internal form to be prettyprinted.	*/
/*								*/
/*  f   output file						*/
/*								*/
/*  nrf true if output to be piped through nroff.  Generates	*/
/*	page headings and table of contents			*/
/*								*/
/*  vbf true if vertical bars connecting the parts of each	*/
/*	construct are to be included				*/
/*								*/
/*  cmf true if comments are to be printed			*/
/*								*/
/*  atf true if attachments (other than comments) are to be	*/
/*	listed							*/
/*								*/
/*  wid	maximum width of an output line (not strictly true	*/
/*	the prettyprinter will not attempt to split tokens	*/
/*	across lines so a long string or identifier can cause	*/
/*	a line longer than wid to be generated.			*/
/*								*/
/*  atc column in which attachments (including comments) are	*/
/*	to start.  Actually this is the leftmost column, since	*/
/*	some attachments are listing following a line which 	*/
/*	extends beyond column atc.				*/
/*								*/
/*  enf	true if the output is to end with a newline		*/
/*								*/
/*  clr a member of the enumeration type pp_color_use (see pp.h)*/
/*	Describes how color is to be used in the prettyprinter. */
/*	Default is no_color.					*/
/*								*/
/* list true if p points to a list of items.  false if to a	*/
/*	single element. (if false, the ndf.next field in the	*/
/*	root node is ignored					*/
/*								*/
/* idnt a member of the enumeration type pp_capitalize (see	*/
/*	pp.h).  Determines how identifiers are capitalized.	*/
/*	Default is ppcap_lower.					*/
/*								*/
/* resv a member of the enumeration type pp_capitalize (see     */
/*	pp.h).  Determines how keywords are capitalized.  	*/
/*	Default is ppcap_lower.					*/   
/*								*/
/* numf true if the output lines are to be numbered.		*/
/*								*/
/* ind  the number of columns to indent each level (default 4)	*/
/*								*/
/* expt true if the output is targeted for an Ada 82 compiler	*/
/*	(removes empty parens from function calls, prints
/*	forward declarations for subprograms)			*/
/*								*/
/****************************************************************/

prprint(p, f, nrf, vbf, cmf, atf, wid, atc, enf, clr, list, idnt, resv, numf,
	ind, expt)
    nodep p;
    FILE *f;
    boolean nrf, vbf, cmf, atf, enf, list, numf, expt;
    int wid, atc, ind;
    enum pp_color_use clr;
    enum pp_capitalize idnt, resv;
{
    export _ expt;
    nroffl = nrf;
    tracefl = false;
    color_use = clr;
    cap_idnt = idnt;
    cap_resv = resv;
    indent = ind;
    ppfile = f;
    subplevel = 1;
    pp_depth = 0;
    if ((atf || cmf) && (wid < atc+10))
        error("no room for attachments (width < attach_column + 10)");
    if (expt and (clr neq ppc_none))
	error("Can't export and use color too");
    if (expt and nrf)
	error("Can't export and use nroff too");
    if (expt and vbf)
	error(
         "Exporting verticle bars will kill the compiler for sure, for sure");
    if (nrf && clr != ppc_none)
	error("nroff can't handle color");
    pp_init(wid, atc, atf, cmf, vbf, numf);
    pp_color_init(clr);
    current_color = (color_name)cur_color.fg;
    if (nroffl)
	fputf (ppfile, ".so rpp.titles\n");
    if (list) pp_top_list(p); else pp(p);
    pp_cleanup (enf);
    if (nroffl)
	fputf (ppfile, ".fi\n");
				/* hack (otherwise no table of contents)
				   may be bug in tmac.X macros for nroff 
				*/
}


dopp(p, f, endnl, comments)
    nodep p;
    FILE *f;
    boolean endnl, comments;
{
    prprint(p,f,false,false,comments,false,70,40,endnl,ppc_none,false,
	    ppcap_lower, ppcap_lower, false, 4, false);
}

tracepp(p, qq,  f)
    nodep p, qq;
    FILE *f;
{
	nroffl = false;
	tracefl = true;
	ppfile = f;
	q = qq;
	pp_init(70, 40, false, false, false);
	pp_begin(null,0,0,f,f,f);
        current_color = (color_name)cur_color.fg;
	pp(p);
	pp_end();
	pp_cleanup(true);
}

# define pp_put(format, item)  {	\
	char s[80];			\
					\
	sprintf(s, format, item);	\
	pp_tok(s);			\
}

# define t true
# define f false

entertoc(type, name, level)
    char *type;	/* procedure, function, task, package */
    char *name;
    int level;  /* static nesting level */
{
    if (nroffl) {
	if (pp_first) pp_first = false;
	else putc('\n',ppfile);
	fputf(ppfile, ".tc \"%s %s\" %d", type, name, level);
    }
}

static keyword(k) char *k;
{
    if (nroffl)
	pp_stok (pp_capitalize(k, cap_resv), "\\f2", "\\f1");
    else
	pp_stok (pp_capitalize(k,cap_resv), "", "");
}

static identifier(id)
    char *id;
{
    pp_tok(pp_capitalize(id, cap_idnt));
}

static pp_top_list(p) nodep p;
{
    if (p == null) fputf(ppfile, "pointer is null\n");
    else {
        pp_begin(null,0,0,f,f,f);
        while (true) {
	    pp(p);
	    if ((p = p->ndf.next) == null) break;
	    pp_cut(3);
	}
        pp_end();
    }
}

static pp_vlist(p) nodep p;
{
    if (p != null) {
        pp_begin(null,4,0,f,f,f);
	if (tracefl) {
	    nodep pp;

	    if (p != q) {
		pp_tok("...");
	    }
	    for (pp = p; pp != q && pp != null; pp = pp->ndf.next);
	    if (pp != null) {
		if (p != q) pp_cut(2);
		pp_tok("&");
		if (pp->ndf.next != null) {
		    pp_cut(2);
		    pp_tok("...");
		}
	    }
	}
	else {
            while (true) {
	        pp(p);
	        if ((p = p->ndf.next) == null) break;
	        pp_cut(2);
            }
	}
        pp_end();
    }
}

/*	print a forward declaration of a procedure or function; called by
	pp_for_export below	(shw)
*/

static pp_a_forward(p) nodep p;	
			/* p is a procedure or function header */
{
	/*  entertoc (tokname (p -> ndf.op), p -> nd.subp.name, subplevel++);
	*/
	if ((p->nd.subp.bodyx eq null) or
	    (p->nd.subp.is_separate)) return;
			/* no need to print if already a stub	*/
	subplevel++;
	pp_begin(p, 0, 1, f, t, f);
	pp_begin(null, indent, 0, f, f, t);
	pp_begin(null, 0, 0, f, f, t);
	keyword(tokname (p -> ndf.op));
	pp_cut(1);
	if (p -> nd.subp.is_oper)
		pp_tok("\"");
	identifier(p -> nd.subp.name);
	if (p -> nd.subp.is_oper)
		pp_tok("\"");
	pp_end();
	if (p -> nd.subp.spec) {
		pp_tok("(");
		pp_list (p -> nd.subp.spec, ";");
		pp_tok(")");
	}
	if (p -> nd.subp.ret_type) {
		pp_cut(1);
		keyword("return");
		pp_space();
		pp(p->nd.subp.ret_type);
	}
	pp_end();
	pp_tok(";");
	pp_end();
	subplevel--;
}

/*	print forward declarations when in export mode (shw)	*/

static pp_for_export(p) nodep p;
{
    nodep savep _ p;
    if (p != null) {
        pp_begin(null,4,0,f,f,f);
	while (true) {		/* everything but subp's first	*/
	    if ((p -> ndf.op neq nop_procedure) and
		(p -> ndf.op neq nop_function))
	    		pp(p);
	    if ((p = p->ndf.next) == null) break;
	    pp_cut(2);
	}
	p _ savep;
        while (true) {		/* now forward decls of subp's */
	    if ((p -> ndf.op eq nop_procedure) or 
		(p -> ndf.op eq nop_function))
	    		pp_a_forward(p);
	    if ((p = p->ndf.next) == null) break;
	    pp_cut(2);
        }
	p _ savep;
        while (true) {		/* now forward bodies of subp's */
	    if ((p -> ndf.op eq nop_procedure) or 
		(p -> ndf.op eq nop_function))
	    		pp(p);
	    if ((p = p->ndf.next) == null) break;
	    pp_cut(2);
        }
        pp_end();
    }
}

static pp_list(p,del) 
    nodep p;
    char *del;
{
    if (p != null) {
        pp_begin(null,indent,0,t,f,t);
        while (true) {
	    pp(p);
	    if ((p = p->ndf.next) == null) break;
	    pp_tok(del);
	    pp_cut(1);
        }
        pp_end();
    }
}

static pp_expr(p)
    nodep p;
{
    pp_begin(p,indent,0,t,f,t);
    pp(p);
    pp_end();
}

static pp(p)  
     register nodep p; 
{
    color_name parent_color;

    check_control_c();		/* may leave open file... if I knew where,
				   I would put in a pushfunc(closepp) where
				   I knew that what files (pipes) were open.
				   But maybe wait until control-c is an 
				   exception that can be handled anywhere?? 
				*/

    if (p == null)
	return;

    pp_depth++;
    parent_color = current_color;
    current_color = pp_color(p,color_use);
    pp_set_color(current_color);

    if (tracefl && p == q)
	pp_tok("&");
    else switch (p -> ndf.op) {
	case nop_ident: 
	    pp_begin(p,0,0,f,f,t);
	    identifier(p -> nd.ident.id);
	    pp_end();
	    break;
	case nop_int: 
	    pp_begin(p,0,0,f,f,t);
	    if (p -> nd.integer.prval)
		pp_tok(p -> nd.integer.prval);
	    else
		pp_put("%d", p -> nd.integer.val);
	    pp_end();
	    break;
	case nop_char: 
	    pp_begin(p,0,0,f,f,t);
	    pp_put("'%c'", p -> nd.integer.val);
	    pp_end();
	    break;
	case nop_real: 
	    pp_begin(p,0,0,f,f,t);
	    if (p -> nd.real.prval)
		pp_tok(p -> nd.real.prval);
	    else
		pp_put("%g", p -> nd.real.val);
	    pp_end();
	    break;
	case nop_string: {
	        char str[256], *s = str;
	        char *g = p->nd.ident.id;
	    
	        pp_begin(p,0,0,f,f,t);
	        *s++ = '"';	/* copy the string, adding delimiting quotes
				   and replicating any quotes internal to the
				   string */
	        while (*g) {
		    if (*g == '"') *s++ = '"';
		    *s++ = *g++;
	        }
	        *s++ = '"';
	        *s = '\0';
	        pp_tok(str);
	        pp_end();
	    }
	
	    break;
	case nop_new: 
	    pp_begin(p, 2, 0, t, f, t);
	    keyword("new");
	    pp_cut(1);
	    pp(p->nd.newx.type_id);
	    pp_end();
	    break;
	case nop_null: 
	    pp_begin(p,0,0,f,f,t);
	    pp_tok("null");
	    pp_end();
	    break;
	case nop_lp: 
	    pp_begin(p, 2, 0, t, f, t);
	    pp_tok("(");
	    pp(p->nd.uop.child);
	    pp_tok(")");
	    pp_end();
	    break;
	case nop_and: 
	case nop_or: 
	case nop_xor: 
	case nop_andthen: 
	case nop_orelse: 
	case nop_equal: 
	case nop_nequal: 
	case nop_lt: 
	case nop_le: 
	case nop_gt: 
	case nop_ge: 
	case nop_plus: 
	case nop_minus: 
	case nop_amper: 
	case nop_star: 
	case nop_div: 
	case nop_mod: 
	case nop_rem: 
	case nop_expon: 
	    pp(p->nd.binop.left);
	    pp_space();
	    pp_tok(nodename (p));
	    pp_cut(1);
	    pp_expr (p -> nd.binop.right);
	    break;
	case nop_neg: 
	case nop_pos: 
	case nop_not: 
	    pp_tok(nodename (p));
	    pp_cut(1);
	    pp_expr (p -> nd.uop.child);
	    break;
	case nop_dot: 
	    pp_begin(p, indent, 0, t, f, t);
	    pp(p->nd.selectx.var);
	    if (p -> nd.selectx.field != null) {
		pp_tok(".");
		pp_cut(0);
		if (p -> nd.selectx.is_oper)
		    pp_tok("\"");
		identifier(p -> nd.selectx.field);
		if (p -> nd.selectx.is_oper)
		    pp_tok("\"");
	    }
	    else
		pp_tok(".all");
	    pp_end();
	    break;
	case nop_quote: 
	    pp_begin(p, indent, 0, t, f, t);
	    pp(p->nd.selectx.var);
	    pp_tok("'");
	    pp_cut(0);
	    identifier(p -> nd.selectx.field);
	    pp_end();
	    break;
	case nop_call: 
	    pp_begin(p, 2, 0, t, f, t);
	    pp(p->nd.call.var);
	    pp_cut(0);
	    if (p -> nd.call.args or (not export)) {
		/* only print paren's if arguments present or if for local
		   version of Ada; export version (Ada 82) doesn't allow
		   empty paren's for function calls with no arguments.
		*/
		    pp_tok("(");
		    pp_list (p -> nd.call.args, ",");
		    pp_tok(")");
	    }
	    pp_end();
	    break;
	case nop_arg: 
	    pp_begin(p, 2, 0, t, f, t);
	    if (p -> nd.arg.name != null)
		pp_put("%s => ", p -> nd.arg.name);
	    pp_cut(0);
	    pp(p->nd.arg.exp);
	    pp_end();
	    break;
	case nop_asgn: 
	    pp_begin(p, 0, 0, f, f, f);
	    pp(p->nd.asgn_stmt.lhs);
	    pp_space();
	    pp_tok(":=");
	    pp_space();
	    pp_expr (p -> nd.asgn_stmt.rhs);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_proc_call: 
	    pp_begin(p, 0, 0, f, f, f);
	    pp(p->nd.call.var);
	    if (p -> nd.call.args) {
		pp_tok("(");
		pp_list (p -> nd.call.args, ",");
		pp_tok(")");
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_if: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("if");
	    pp_space();
	    pp_expr (p -> nd.elsifx.expr);
	    pp_space();
	    keyword("then");
	    pp_cut(2);
	    pp_vlist (p -> nd.elsifx.then_part);
	    if (p -> nd.ifx.elsif_list) {
		nodep q = p -> nd.ifx.elsif_list;
		for (; q; q = q -> ndf.next) {
		    pp_cut(2);
		    pp (q);
		    pp_cut(2);
		}
	    }
	    else
		pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("if");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_elsif: 
	    if (p -> nd.elsifx.expr) {
		pp_begin(p, indent, 0, f, f, t);
		keyword("elsif");
		pp_cut(2);
		pp_expr (p -> nd.elsifx.expr);
		pp_cut(2);
		keyword("then");
		pp_end();
	    }
	    else
		keyword("else");
	    pp_cut(2);
	    pp_vlist (p -> nd.elsifx.then_part);
	    break;
	case nop_loop: 
	    pp_begin(p, 0, 0, f, t, f);
	    if (p->nd.whilex.label) {
		identifier(p->nd.whilex.label);
		pp_tok(":");
		pp_space();
	    }
	    keyword("loop");
	    pp_cut(2);
	    pp_vlist (p -> nd.whilex.bodyx);
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("loop");
	    if (p->nd.whilex.label) {
		pp_space();
		identifier(p->nd.whilex.label);
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_while: 
	    pp_begin(p, 0, 0, f, t, f);
	    if (p->nd.whilex.label) {
		identifier(p->nd.whilex.label);
		pp_tok(":");
		pp_space();
	    }
	    keyword("while");
	    pp_space();
	    pp_expr (p -> nd.whilex.expr);
	    pp_space();
	    keyword("loop");
	    pp_cut(2);
	    pp_vlist (p -> nd.whilex.bodyx);
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("loop");
	    if (p->nd.whilex.label) {
		pp_space();
		identifier(p->nd.whilex.label);
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_return: 
	    pp_begin(p, indent, 0, f, f, t);
	    keyword("return");
	    if (p -> nd.returnx.val) {
		pp_space();
		pp_expr (p -> nd.returnx.val);
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_id_decl: 
	    pp_begin(p,0,0,f,f,t);
	    identifier(p -> nd.id_decl.id);
	    pp_end();
	    break;
	case nop_char_enum: 
	    pp_begin(p, 0, 0, t, f, t);
	    pp_put("'%c'", p -> nd.char_enum.ch);
	    pp_end();
	    break;
	case nop_type: 
	case nop_subtype: 
	    pp_begin(p, indent, 0, f, f, t);
	    pp_begin(null, indent, 0, f, f, t);
	    keyword(tokname (p -> ndf.op));
	    pp_cut(1);
	    identifier(p -> nd.type_decl.id);
	    if (p -> nd.type_decl.disc) {
		pp_tok("(");
		pp_list (p -> nd.type_decl.disc, ";");
		pp_tok(")");
	    }
	    pp_cut(1);
	    if (p -> nd.type_decl.typex) {
		keyword("is");
		pp_end();
		pp_cut(1);
		pp(p->nd.type_decl.typex);
	    }
	    else
		pp_end();
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_obj_decl: 
	    pp_begin(p, indent, 0, f, f, t);
	    pp_list (p -> nd.obj_decl.id_list, ",");
	    pp_tok(":");
	    pp_cut(1);
	    if (p -> nd.obj_decl.is_constant) {
		keyword("constant");
		pp_space();
	    }
	    if (p -> nd.obj_decl.subtypex)
		pp_expr (p -> nd.obj_decl.subtypex);
	    if (p -> nd.obj_decl.initexp) {
		pp_space();
		pp_tok(":=");
		pp_cut(1);
		pp_expr (p -> nd.obj_decl.initexp);
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_except_decl: 
	    pp_begin(p, indent, 0, f, f, t);
	    pp(p->nd.except_decl.id_list);
	    pp_tok(":");
	    pp_cut(1);
	    keyword("exception");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_discriminant: 
	case nop_param: 
	    pp_begin(p, indent, 0, t, f, t);
	    pp_list (p -> nd.param_decl.id_list, ",");
	    pp_tok(":");
	    if (p -> nd.param_decl.is_in) {
		pp_space();
		keyword("in");
	    }
	    if (p -> nd.param_decl.is_out) {
		pp_space();
		keyword("out");
	    }
	    pp_cut(1);
	    pp(p->nd.param_decl.subtyp);
	    if (p -> nd.param_decl.initexp) {
		pp_cut(1);
		pp_tok(":=");
		pp_space();
		pp_expr (p -> nd.param_decl.initexp);
	    }
	    pp_end();
	    break;
	case nop_array: 
	    pp_begin(p, indent, 0, t, f, t);
	    keyword("array");
	    pp_cut(1);
	    pp_tok("(");
	    pp_list (p -> nd.arrayx.index_list, ",");
	    pp_tok(")");
	    pp_space();
	    keyword("of");
	    pp_cut(1);
	    pp_expr (p -> nd.arrayx.ele_type);
	    pp_end();
	    break;
	case nop_index: 
	    pp_begin(p, 0, 0, f, f, f);
	    pp_expr (p -> nd.index.typex);
	    pp_end();
	    break;
	case nop_record: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("record");
	    pp_cut(2);
	    if (p -> nd.recordx.field_list == null) {
		keyword("null");
		pp_tok(";");
	    }
	    else
		pp_vlist (p -> nd.recordx.field_list);
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("record");
	    pp_end();
	    break;
	case nop_rec_var: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("case");
	    pp_space();
	    pp_expr (p -> nd.casex.expr);
	    pp_space();
	    keyword("is");
	    {
		nodep q = p -> nd.casex.when_list;
		for (; q; q = q -> ndf.next) {
		    pp_cut(2);
		    pp (q);
		}
	    }
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("case");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_rec_when: 
	    pp_begin(p, indent, 0, f, f, t);
	    keyword("when");
	    pp_space();
	    pp_list (p -> nd.whenx.choice_list, " |");
	    pp_space();
	    pp_tok("=>");
	    pp_cut(2);
	    pp_end();
	    pp_vlist (p -> nd.whenx.stmt);
	    break;
	case nop_access: 
	    pp_begin(p, 0, 0, t, f, t);
	    keyword("access");
	    pp_cut(1);
	    pp(p->nd.accessx.target_type);
	    pp_end();
	    break;
	case nop_procedure: 
	case nop_function:
	    entertoc (tokname (p -> ndf.op), p -> nd.subp.name, subplevel++);
	    if (p->nd.subp.context) {
		pp_vlist(p->nd.subp.context);
		pp_cut(2);
	    }
	    pp_begin(p, 0, 1, f, t, f);
	    pp_begin(null, indent, 0, f, f, t);
	    pp_begin(null, 0, 0, f, f, t);
	    keyword(tokname (p -> ndf.op));
	    pp_cut(1);
	    if (p -> nd.subp.is_oper)
		pp_tok("\"");
	    identifier(p -> nd.subp.name);
	    if (p -> nd.subp.is_oper)
		pp_tok("\"");
	    pp_end();
	    if (p -> nd.subp.spec) {
		pp_tok("(");
		pp_list (p -> nd.subp.spec, ";");
		pp_tok(")");
	    }
	    if (p -> nd.subp.ret_type) {
		pp_cut(1);
		keyword("return");
		pp_space();
		pp(p->nd.subp.ret_type);
	    }
	    pp_end();
	    if (p -> nd.subp.is_separate) {
		pp_space();
		keyword("is");
		pp_space();
		keyword("separate");
	    }
	    else
		if (p -> nd.subp.bodyx) {
		    pp_space();
		    keyword("is");
		    pp_cut(2);
		    if (export) pp_for_export(p -> nd.subp.decl);
			/* print forwards if exporting	*/
		    else pp_vlist (p -> nd.subp.decl);
		    pp_cut(2);
		    keyword("begin");
		    pp_cut(2);
		    pp_vlist (p -> nd.subp.bodyx);
		    if (p -> nd.subp.except) {
			pp_cut(2);
			keyword("exception");
			pp_cut(2);
			pp_vlist (p -> nd.subp.except);
		    }
		    pp_cut(2);
		    keyword("end");
		    pp_space();
		    if (p -> nd.subp.is_oper)
			pp_tok("\"");
		    identifier(p -> nd.subp.name);
		    if (p -> nd.subp.is_oper)
			pp_tok("\"");
		}
	    pp_tok(";");
	    pp_end();
	    subplevel--;
	    break;
	case nop_declare: 
	    pp_begin(p, 0, 1, f, t, f);
	    if (p -> nd.subp.name) {
		identifier(p -> nd.subp.name);
		pp_tok(":");
		pp_cut(1);
	    }
	    if (p -> nd.subp.decl) {
		keyword("declare");
		pp_cut(2);
		if (export) pp_for_export(p);
		else pp_vlist (p -> nd.subp.decl);
	    }
	    pp_cut(2);
	    keyword("begin");
	    pp_cut(2);
	    pp_vlist (p -> nd.subp.bodyx);
	    pp_cut(2);
	    if (p -> nd.subp.except) {
		keyword("exception");
		pp_cut(2);
		pp_vlist (p -> nd.subp.except);
		pp_cut(2);
	    }
	    keyword("end");
	    if (p -> nd.subp.name) {
		pp_space();
		identifier(p -> nd.subp.name);
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_package: 
	    entertoc ("package", p -> nd.packagex.name, subplevel++);
	    pp_begin(p, 0, 1, f, t, f);
	    if (p->nd.packagex.context) {
		pp_vlist(p->nd.packagex.context);
		pp_cut(2);
	    }
	    pp_begin(null, 0, 0, f, f, t);
	    keyword("package");
	    pp_cut(1);
	    identifier(p -> nd.packagex.name);
	    pp_cut(1);
	    keyword("is");
	    pp_end();
	    pp_cut(2);
	    pp_vlist (p -> nd.packagex.decl);
	    if (p -> nd.packagex.priv_decl) {
		pp_cut(1);
		keyword("private");
		pp_vlist (p -> nd.packagex.priv_decl);
	    }
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    identifier(p -> nd.packagex.name);
	    pp_tok(";");
	    pp_end();
	    subplevel--;
	    break;
	case nop_body: 
	    entertoc ("package body", p -> nd.packagex.name, subplevel++);
	    pp_begin(p, 0, 1, f, t, f);
	    if (p->nd.packagex.context) {
		pp_vlist(p->nd.packagex.context);
		pp_cut(2);
	    }
	    pp_begin(null, 0, 0, f, f, t);
	    keyword("package");
	    pp_cut(1);
	    keyword("body");
	    pp_cut(1);
	    identifier(p -> nd.packagex.name);
	    pp_cut(1);
	    keyword("is");
	    pp_end();
	    if (p -> nd.packagex.is_separate) {
		pp_cut(1);
		keyword("separate");
	    }
	    else {
		pp_cut(2);
	        pp_vlist (p -> nd.packagex.decl);
		if (p -> nd.packagex.bodyx) {
    		    pp_cut(2);
		    keyword("begin");
		    pp_cut(2);
		    pp_vlist (p -> nd.packagex.bodyx);
		    if (p -> nd.packagex.except) {
			pp_cut(2);
			keyword("exception");
			pp_cut(2);
			pp_vlist (p -> nd.packagex.except);
		    }
		}
		pp_cut(2);
		keyword("end");
		pp_space();
		identifier(p -> nd.packagex.name);
	    }
	    pp_tok(";");
	    pp_end();
	    subplevel--;
	    break;
	case nop_task: 
	case nop_task_type: 
	    if (p -> ndf.op == nop_task)
		entertoc ("task", p -> nd.taskx.name, subplevel++);
	    else
		entertoc ("task type", p -> nd.taskx.name, subplevel++);
	    pp_begin(p, 0, 1, f, t, f);
	    pp_begin(null, 0, 0, f, f, t);
	    keyword("task");
	    if (p -> ndf.op == nop_task_type) {
		pp_cut(1);
		keyword("type");
	    }
	    pp_cut(1);
	    identifier(p -> nd.taskx.name);
	    pp_cut(1);
	    keyword("is");
	    pp_end();
	    pp_cut(2);
	    pp_vlist (p -> nd.taskx.entryx);
	    pp_cut(1);
	    pp_vlist (p -> nd.taskx.rep_spec);
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    identifier(p -> nd.taskx.name);
	    pp_tok(";");
	    pp_end();
	    subplevel--;
	    break;
	case nop_task_body: 
	    entertoc ("task body", p -> nd.taskx.name, subplevel++);
	    pp_begin(p, 0, 1, f, t, f);
	    pp_begin(null, 0, 0, f, f, t);
	    keyword("task");
	    pp_cut(1);
	    keyword("body");
	    pp_cut(1);
	    identifier(p -> nd.taskx.name);
	    pp_cut(1);
	    keyword("is");
	    pp_end();
	    if (p -> nd.taskx.is_separate) {
		pp_cut(1);
		keyword("separate");
	    }
	    else {
		pp_cut(2);
		pp_vlist (p -> nd.taskx.decl);
		pp_cut(2);
		keyword("begin");
		pp_cut(2);
		pp_vlist (p -> nd.taskx.bodyx);
		if (p -> nd.taskx.except) {
		    pp_cut(2);
		    keyword("exception");
		    pp_cut(2);
		    pp_vlist (p -> nd.taskx.except);
		}
		pp_cut(2);
		keyword("end");
		pp_space();
		identifier(p -> nd.taskx.name);
	    }
	    pp_tok(";");
	    pp_end();
	    subplevel--;
	    break;
	case nop_enum: 
	    pp_begin(p, indent, 0, f, f, t);
	    pp_tok("(");
	    pp_list (p -> nd.enumtype.ids, ",");
	    pp_tok(")");
	    pp_end();
	    break;
	case nop_range: 
	    pp_begin(p, indent, 0, t, f, t);
	    if (p -> nd.rangex.type_id) {
		pp(p->nd.rangex.type_id);
	    }
	    if (p -> nd.rangex.is_range_constraint) {
		pp_space();
		keyword("range");
		pp_space();
	    }
	    if (p -> nd.rangex.not_constrained)
		pp_tok("<>");
	    else {
		pp_expr (p -> nd.rangex.left);
		pp_tok("..");
		pp_expr (p -> nd.rangex.right);
	    }
	    pp_end();
	    break;
	case nop_delta: 
	case nop_digits: 
	    pp_begin(p, indent, 0, t, f, t);
	    if (p -> nd.rangex.type_id) {
		identifier(p -> nd.rangex.type_id -> nd.ident.id);
		pp_space();
	    }
	    keyword(nodename (p));
	    pp_space();
	    pp_expr (p -> nd.rangex.accuracy_expr);
	    if (p -> nd.rangex.not_constrained) {
		pp_cut(1);
		pp_tok("<>");
	    }
	    else
		if (p -> nd.rangex.left) {
		    pp_cut(1);
		    keyword("range");
		    pp_space();
		    pp_expr (p -> nd.rangex.left);
		    pp_tok("..");
		    pp_expr (p -> nd.rangex.right);
		}
	    pp_end();
	    break;
	case nop_derived_type: 
	    pp_begin(p, indent, 0, f, f, t);
	    keyword("new");
	    pp_cut(1);
	    pp(p->nd.derived_type.typex);
	    pp_end();
	    break;
	case nop_in: 
	    pp_begin(p, 0, 0, t, f, t);
	    pp_expr (p -> nd.inop.expr);
	    pp_cut(1);
	    if (p -> nd.inop.not_in) {
		keyword("not");
		pp_cut(1);
	    }
	    keyword("in");
	    pp_cut(1);
	    pp(p->nd.inop.rangex);
	    pp_end();
	    break;
	case nop_for: 
	    pp_begin(p, 0, 0, f, t, f);
	    if (p->nd.forx.label) {
		identifier(p->nd.forx.label);
		pp_tok(":");
		pp_space();
	    }
	    pp_begin(null, indent, 0, f, f, t);
	    keyword("for");
	    pp_cut(1);
	    identifier(p -> nd.forx.cntrl_var);
	    pp_cut(1);
	    keyword("in");
	    pp_cut(1);
	    if (p -> nd.forx.reversex) {
		keyword("reverse");
		pp_cut(1);
	    }
	    pp_expr (p -> nd.forx.lims);
	    pp_cut(1);
	    keyword("loop");
	    pp_end();
	    pp_cut(2);
	    pp_vlist (p -> nd.forx.bodyx);
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("loop");
	    if (p->nd.forx.label) {
		pp_space();
		identifier(p->nd.forx.label);
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_null_stmt: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("null");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_case: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("case");
	    pp_space();
	    pp_expr (p -> nd.casex.expr);
	    pp_space();
	    keyword("is");
	    {
		nodep q = p -> nd.casex.when_list;
		for (; q; q = q -> ndf.next) {
		    pp_cut(2);
		    pp (q);
		}
	    }
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("case");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_when: 
	    pp_begin(p, indent, 0, f, f, t);
	    keyword("when");
	    pp_space();
	    pp_list (p -> nd.whenx.choice_list, " |");
	    pp_space();
	    pp_tok("=>");
	    pp_cut(2);
	    pp_end();
	    pp_vlist (p -> nd.whenx.stmt);
	    break;
	case nop_choice: 
	    pp_begin(p, 0, 0, f, t, f);
	    if (p -> nd.choice.choice)
		pp(p->nd.choice.choice);
	    else
		keyword("others");
	    pp_end();
	    break;
	case nop_exit: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("exit");
	    if (p -> nd.exitx.loop_name) {
		pp_space();
		pp(p->nd.exitx.loop_name);
	    }
	    if (p -> nd.exitx.cond) {
		pp_space();
		keyword("when");
		pp_space();
		pp_expr (p -> nd.exitx.cond);
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_goto: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("goto");
	    pp_space();
	    pp(p->nd.gotox.label);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_entry: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("entry");
	    pp_space();
	    identifier(p -> nd.entryx.id);
	    if (p -> nd.entryx.rangex) {
		pp_tok("(");
		pp(p->nd.entryx.rangex);
		pp_tok(")");
	    }
	    if (p -> nd.entryx.formal) {
		pp_tok("(");
		pp_list (p -> nd.entryx.formal, ";");
		pp_tok(")");
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_accept: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("accept");
	    pp_space();
	    pp(p->nd.acceptx.name);
	    if (p -> nd.acceptx.stmts) {
	        pp_space();
	        keyword("do");
		pp_cut(2);
		pp_vlist (p -> nd.acceptx.stmts);
		pp_cut(2);
		keyword("end");
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_delay: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("delay");
	    pp_cut(1);
	    pp_expr (p -> nd.delayx.expr);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_raise: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("raise");
	    pp_space();
	    pp(p->nd.raisex.except);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_lb: 
	    pp_begin(p, -indent, 0, f, f, f);
	    pp_tok("<<");
	    identifier(p -> nd.ident.id);
	    pp_tok(">>");
	    pp_end();
	    break;
	case nop_pragma: 
	    pp_begin(p, 0, 0, f, t, f);
	    keyword("pragma");
	    pp_space();
	    identifier(p -> nd.pragmax.id);
	    if (p -> nd.pragmax.args) {
		pp_tok("(");
		pp_list (p -> nd.pragmax.args, ",");
		pp_tok(")");
	    }
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_use: 
	case nop_with: 
	    pp_begin(p, indent, 0, f, f, t);
	    keyword(tokname (p -> ndf.op));
	    pp_space();
	    pp_list (p -> nd.usex.list, ",");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_name: 
	    pp(p->nd.name_list.name);
	    break;
	case nop_qual_expr: 
	    pp_begin(p, indent, 0, f, f, t);
	    pp(p->nd.aggregate.qualifier);
	    pp_tok("'");
	    pp_tok("(");
	    pp_expr (p -> nd.aggregate.agg);
	    pp_tok(")");
	    pp_end();
	    break;
	case nop_aggregate: 
	    pp_begin(p, indent, 0, f, f, t);
	    if (p -> nd.aggregate.qualifier) {
		pp(p->nd.aggregate.qualifier);
		pp_tok("'");
	    }
	    pp_tok("(");
	    pp_list (p -> nd.aggregate.agg, ",");
	    pp_tok(")");
	    pp_end();
	    break;
	case nop_agg_comp: 
	    pp_begin(p, indent, 0, f, f, t);
	    if (p -> nd.agg_comp.choice) {
		pp_list (p -> nd.agg_comp.choice, " |");
		pp_space();
		pp_tok("=>");
		pp_cut(1);
	    }
	    pp_expr (p -> nd.agg_comp.expr);
	    pp_end();
	    break;
	case nop_private: 
	    keyword("private");
	    break;
	case nop_limited: 
	    pp_begin(p, indent, 0, t, f, f);
	    keyword("limited");
	    pp_cut(1);
	    keyword("private");
	    pp_end();
	    break;
	case nop_code: 
	    pp_begin(p, indent, 0, f, f, f);
	    pp(p->nd.code.expr);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_select:
	    pp_begin(p,0,0,f,t,f);
	    keyword("select");
	    pp_cut(2);
	    pp(p->nd.select_wait.or_list);
	    {   nodep q;
		for (q = p->nd.select_wait.or_list->ndf.next;
		     q;
		     q = q->ndf.next) {
			 pp_cut(2);
			 keyword("or");
			 pp_space();
			 pp(q);
		     }
	     }
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("select");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_sel_when:
	    pp_begin(p,indent,0,t,f,f);
	    if (p->nd.select_when.whenx) {
		pp_begin(null,indent,0,t,f,f);
		keyword("when");
		pp_cut(1);
		pp_expr(p->nd.select_when.whenx);
		pp_cut(1);
		pp_tok("=>");
		pp_end();
		pp_cut(1);
	    }
	    pp_vlist(p->nd.select_when.alt);
	    pp_end();
	    break;
	case nop_terminate:
	    pp_begin(p,0,0,f,f,f);
	    keyword("terminate");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_cond_ent:
	    pp_begin(p,0,0,f,t,f);
	    keyword("select");
	    pp_space();
	    pp(p->nd.cond_entry.call);
	    pp_cut(2);
	    pp_vlist(p->nd.cond_entry.stmt);
	    pp_cut(2);
	    keyword("else");
	    pp_cut(1);
	    pp_vlist(p->nd.cond_entry.else_part);
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("select");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_timed_ent:
	    pp_begin(p,0,0,f,t,f);
	    keyword("select");
	    pp_space();
	    pp(p->nd.timed_entry.call);
	    pp_cut(2);
	    pp_vlist(p->nd.timed_entry.stmt);
	    pp_cut(2);
	    keyword("or");
	    pp_cut(1);
	    pp_vlist(p->nd.timed_entry.or_part);
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("select");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_len_spec:
	    pp_begin(p,indent,0,f,f,f);
	    keyword("for");
	    pp_cut(1);
	    pp(p->nd.rep_spec.name);
	    pp_cut(1);
	    keyword("use");
	    pp_cut(1);
	    pp_expr(p->nd.rep_spec.expr);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_addr_spec:
	    pp_begin(p,0,0,f,f,f);
	    keyword("for");
	    pp_cut(1);
	    pp(p->nd.rep_spec.name);
	    pp_cut(1);
	    keyword("use");
	    pp_space();
	    keyword("at");
	    pp_cut(1);
	    pp_expr(p->nd.rep_spec.expr);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_rec_spec:
	    pp_begin(p,0,0,f,f,f);
	    pp_begin(null,indent,0,f,f,f);
	    keyword("for");
	    pp_cut(1);
	    pp(p->nd.rec_rspec.name);
	    pp_cut(1);
	    keyword("use");
	    pp_cut(1);
	    pp_end();
	    pp_begin(p,indent,0,f,t,f);
	    pp_begin(p,indent,0,t,f,f);
	    keyword("record");
	    if (p->nd.rec_rspec.algn) {
	        pp_cut(1);
		keyword("at");
		pp_space();
		keyword("mod");
		pp_cut(1);
		pp_expr(p->nd.rec_rspec.algn);
	 	pp_tok(";");
	    }
	    pp_end();
	    pp_cut(2);
	    pp_vlist(p->nd.rec_rspec.loc);
	    pp_cut(2);
	    keyword("end");
	    pp_space();
	    keyword("record");
	    pp_tok(";");
	    pp_end();
	    pp_end();
	    break;
	case nop_at:
	    pp_begin(p,0,0,f,f,f);
	    pp(p->nd.location.name);
	    pp_cut(1);
	    keyword("at");
	    pp_space();
	    pp_expr(p->nd.location.expr);
	    pp_cut(1);
	    keyword("range");
	    pp_space();
	    pp_expr(p->nd.location.rangex);
	    pp_tok(";");
	    pp_end();
	    break;	    
	case nop_gen_with:
	    pp_begin(p,0,0,t,f,f);
	    keyword("with");
	    pp_cut(1);
	    pp(p->nd.gen_with.subp);
	    if (p->nd.gen_with.box || p->nd.gen_with.name != null) {
		pp_cut(1);
		keyword("is");
	        pp_cut(1);
		if (p->nd.gen_with.box) pp_tok("<>");
		else pp(p->nd.gen_with.name);
	    }
	    pp_tok(";");
	    pp_end();
	    break;		
	case nop_abort:
	    pp_begin(p,0,0,f,f,f);
	    keyword("abort");
	    pp_cut(1);
	    pp_list(p->nd.abortx.tasks);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_disc_type:
	    pp_begin(p,0,0,t,f,f);
	    pp_tok("(<>)");
	    pp_end();
	    break;
	case nop_err:
	    pp_tok("*ERR*");
	    break;
	case nop_pack_ren:
	case nop_task_ren:
	    pp_begin(p,indent,0,t,f,f);
	    if (p->ndf.op == nop_pack_ren)
		keyword("package");
	    else
		keyword("task");
	    pp_space();
	    identifier(p->nd.pack_ren.id);
	    pp_cut(1);
	    keyword("renames");
	    pp_cut(1);
	    pp_expr(p->nd.pack_ren.old_name);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_expr:
	    pp_expr(p->nd.expr.expr);
	    break;
	case nop_func_ginst:
	case nop_proc_ginst:
	    pp_begin(p, 0, 1, f, t, f);
	    if (p->nd.subp.context) {
		pp_vlist(p->nd.subp.context);
		pp_cut(2);
	    }
	    pp_begin(null, indent, 0, f, f, t);
	    pp_begin(null, 0, 0, f, f, t);
	    if (p->ndf.op == nop_func_ginst)
		keyword("function");
	    else
		keyword("procedure");
	    pp_cut(1);
	    if (p -> nd.subp.is_oper)
		pp_tok("\"");
	    identifier(p -> nd.subp.name);
	    if (p -> nd.subp.is_oper)
		pp_tok("\"");
	    pp_end();
	    if (p -> nd.subp.spec) {
		pp_tok("(");
		pp_list (p -> nd.subp.spec, ";");
		pp_tok(")");
	    }
	    if (p -> nd.subp.ret_type) {
		pp_cut(1);
		keyword("return");
		pp_space();
		pp(p->nd.subp.ret_type);
	    }
	    pp_end();
	    keyword("is");
	    pp_cut(1);
	    pp(p->nd.subp.bodyx);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_pack_ginst:
	    pp_begin(p, 0, 0, t, f, f);
	    if (p->nd.packagex.context) {
		pp_vlist(p->nd.packagex.context);
		pp_cut(2);
	    }
	    pp_begin(null, 0, 0, f, f, t);
	    keyword("package");
	    pp_cut(1);
	    identifier(p -> nd.packagex.name);
	    pp_cut(1);
	    keyword("is");
	    pp_end();
	    pp_cut(2);
	    pp(p->nd.packagex.bodyx);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_func_ren:
	case nop_proc_ren:
	    pp_begin(p, 0, 1, f, t, f);
	    pp_begin(null, indent, 0, f, f, t);
	    pp_begin(null, 0, 0, f, f, t);
	    if (p->ndf.op == nop_func_ren)
		keyword("function");
	    else
		keyword("procedure");
	    pp_cut(1);
	    if (p -> nd.subp.is_oper)
		pp_tok("\"");
	    identifier(p -> nd.subp.name);
	    if (p -> nd.subp.is_oper)
		pp_tok("\"");
	    pp_end();
	    if (p -> nd.subp.spec) {
		pp_tok("(");
		pp_list (p -> nd.subp.spec, ";");
		pp_tok(")");
	    }
	    if (p -> nd.subp.ret_type) {
		pp_cut(1);
		keyword("return");
		pp_space();
		pp(p->nd.subp.ret_type);
	    }
	    pp_end();
	    keyword("renames");
	    pp_cut(1);
	    pp_expr(p->nd.subp.bodyx);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_generic:
	    pp_begin(p,0,0,f,f,f);
	    if (p->nd.genericx.context) {
		pp_vlist(p->nd.genericx.context);
		pp_cut(2);
	    }
	    keyword("generic");
	    pp_cut(2);
      	    pp_vlist(p->nd.genericx.param);
	    pp_cut(2);
	    pp(p->nd.genericx.unit);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_instant:
	    pp_begin(p,indent,0,t,f,f);
	    keyword("new");
	    pp_cut(1);
	    pp_expr(p->nd.instant.name);
	    pp_end();
	    break;
	case nop_obj_ren:
	    pp_begin(p, indent, 0, t, f, t);
	    pp_list (p -> nd.obj_rename.id_list, ",");
	    pp_tok(":");
	    pp_cut(1);
	    pp(p->nd.obj_rename.typex);
	    pp_cut(1);
	    keyword("renames");
	    pp_space();
	    pp_expr(p->nd.obj_rename.old_name);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_except_ren:
	    pp_begin(p, indent, 0, t, f, t);
	    pp_list (p -> nd.obj_rename.id_list, ",");
	    pp_tok(":");
	    pp_cut(1);
	    keyword("exception");
	    pp_cut(1);
	    keyword("renames");
	    pp_space();
	    pp_expr(p->nd.obj_rename.old_name);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_oper_string:
	    pp_tok("\"");
	    pp(p->nd.ident.id);
	    pp_tok("\"");
	    break;

  /* Castor */

	case nop_expr_subst:
	case nop_name_subst:
	case nop_type_subst:
	case nop_subtype_subst:
	case nop_ident_subst:
	    pp_begin(p,indent,0,f,f,t);
	    pp_tok("$");
	    identifier(p->nd.ident.id);
	    pp_end();
	    break;
	case nop_decl_subst:
	case nop_stmt_subst:
	    pp_begin(p,indent,0,f,f,t);
	    pp_tok("$");
	    identifier(p->nd.ident.id);
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_macro:			/* Castor */ 
	    entertoc ("macro", p -> nd.macrox.designator, subplevel++);
	    pp_begin(p, 0, 1, f, t, f);
	    if (p->nd.macrox.context) {
		pp_vlist(p->nd.macrox.context);
		pp_cut(2);
	    }
	    pp_begin(null, indent, 0, f, f, t);
	    pp_begin(null, 0, 0, f, f, t);
	    keyword("macro");
	    pp_cut(1);
	    pp(p->nd.macrox.designator);
	    pp_end();
	    if (p -> nd.macrox.ret_type) {
		pp_cut(1);
		keyword("return");
		pp_space();
		pp(p->nd.macrox.ret_type);
	    }
	    pp_end();
	    if (p -> nd.macrox.is_separate) {
		pp_space();
		keyword("is");
		pp_space();
		keyword("separate");
	    }
	    else
		if (p -> nd.macrox.bodyx) {
		    pp_space();
		    keyword("is");
		    pp_cut(2);
		    pp_vlist (p -> nd.macrox.decl);
		    pp_cut(2);
		    keyword("begin");
		    pp_cut(2);
		    pp_vlist (p -> nd.macrox.bodyx);
		    if (p -> nd.macrox.except) {
			pp_cut(2);
			keyword("exception");
			pp_cut(2);
			pp_vlist (p -> nd.macrox.except);
		    }
		    pp_cut(2);
		    keyword("end");
		}
	    pp_tok(";");
	    pp_end();
	    subplevel--;
	    break;
	case nop_macro_desig:
	    pp_begin(p,0,0,t,f,t);
	    pp_tok("{");
	    pp_list(p->nd.call_form.list,null);
	    pp_tok("}");
	    pp_end();
	    break;
	case nop_callf_ident:			/* Castor */
	    pp_begin(p,0,0,f,f,t);
	    identifier(p -> nd.id_decl.id);
	    pp_end();
	    break;
	case nop_macro_param_list:
	    pp_begin(p,0,0,f,f,t);
	    pp_tok("(");
	    pp_list(p->nd.macro_list.list,";");
	    pp_tok(")");
	    pp_end();
	    break;
	case nop_macro_param:
	    pp_begin(p, indent, 0, t, f, t);
	    pp_list (p -> nd.param_decl.id_list, ",");
	    if (p -> nd.param_decl.subtyp) {
		pp_tok(":");
	    	pp_cut(1);
	    	pp(p->nd.param_decl.subtyp);
	    	if (p -> nd.param_decl.initexp) {
		    pp_cut(1);
		    pp_tok(":=");
		    pp_space();
		    pp_expr (p -> nd.param_decl.initexp);
	        }
	    }
	    pp_end();
	    break;
	case nop_macro_arr_param:
	    pp_begin(p, indent, 0, t, f, t);
	    pp_list (p -> nd.param_decl.id_list, ",");
	    pp_tok(":");
	    pp_cut(1);
	    keyword("array");
	    pp_end();
	    break;
	case nop_type_callf:
	case nop_name_callf:
	case nop_expr_callf:
	case nop_top_callf:
	    pp_begin(p,0,0,f,f,t);
	    pp_tok("{");
	    pp_space();
	    pp_list(p->nd.call_form.list,null);
	    pp_space();
	    pp_tok("}");
	    pp_end();
	    break;
	case nop_decl_callf:
	case nop_stmt_callf:
	    pp_begin(p,0,0,f,f,t);
	    pp_tok("{");
	    pp_space();
	    pp_list(p->nd.call_form.list,null);
	    pp_space();
	    pp_tok("}");
	    pp_tok(";");
	    pp_end();
	    break;
	case nop_callf_arg_list:
	    pp_begin(p,0,0,f,f,t);
	    pp_tok("(");
	    pp_list(p->nd.macro_list.list,",");
	    pp_tok(")");
	    pp_end();
	    break;
	case nop_callf_stmt_arg:
	    pp_begin(p,indent,0,f,f,f);
	    keyword("begin");
	    pp_cut(2);
	    pp_vlist(p->nd.macro_list.list);
	    pp_cut(2);	
	    keyword("end");
	    pp_end();
	    break;
	default: 
	    pp_put("[Invalid node #%d]", p->ndf.op);
	    break;
    }
    /* restore color */
    current_color = parent_color;
    pp_set_color(current_color);
    pp_depth--;
}
