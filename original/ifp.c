static char rcsid[] = 
"$Header:   /usr/ics/darpa/ada/RCS/ifp.v  Revision 1.4  83/06/14  16:27:39  ada  Exp$";

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


/* $Log:	/usr/ics/darpa/ada/RCS/ifp.v $
 * Revision 1.4  83/06/14  16:27:39  ada
 * Same as 1.3.4.1.
 * 
 * Revision 1.3.4.1  83/06/14  13:47:46  tadman
 * Delete case for nop_comp_unit.
 * 
 * Revision 1.3  83/01/12  09:15:33  ada
 * Added copyright notice.
 * 
 * Revision 1.2  83/01/04  14:36:02  ada
 * Checked in for sca.  Added field information to output.
 * 
 * Revision 1.1  82/12/12  21:27:02  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "count.h"
#include <stdio.h>
 
# define t true
# define f false

extern FILE  *ppfile;
extern pp_tok(), pp_cut();

ifp_begin(), ifp_end(), ifp_lbegin(), ifp_lend(); /* forward declarations */

ifp(p, file, nrf, vbf, cmf, atf, wid, atc, enf)
    nodep p;
    FILE *file;
    boolean nrf, vbf, cmf, atf, enf;
    int wid, atc;
{
	ppfile = file;
        if ((atf || cmf) && (wid < atc+10))
            error("no room for attachments (width < attach_column + 10)");
        pp_init(wid, 0, false, false, vbf);
  	ifp_list(p);
	pp_cleanup(enf);
}

ifp_list(p)
    nodep p;
{
    if (p == null) fputf("pointer is null\n");
    else {
        pp_begin(null,-4,0,f,f,f);		/* kludge */
        while (true) {
	    maptree(p, ifp_begin, ifp_end, ifp_lbegin, ifp_lend);
	    p = p->ndf.next;
	    if (p == null) break;
	    pp_cut(2);
	}
        pp_end();
    }
}

ifp_lbegin(p)
    nodep p;
{
    pp_begin(null,4,0,f,f,f);
    pp_tok("{");
    pp_cut(1);
}

ifp_lend(p)
    nodep p;
{
    pp_cut(1);
    pp_tok("}");
    pp_end();
}

static int Int_fld(i,fl)
    int i;
    char *fl;
{
    char s[30];

    pp_cut(1);
    sprintf(s, "%s:%d", fl, i);
    pp_tok(s);
}

static Real_fld(r,fl)
    float r;
    char *fl;
{
    char s[30];

    pp_cut(1);
    sprintf(s, "%s:%g", fl, r);
    pp_tok(s);
}

static Str_fld(s, fl)
    char *s, *fl;
{
    char v[200];

    pp_cut(1);
    sprintf(v, "%s:\"%s\"", fl, s);
    pp_tok(v);
}

static Char_fld(c, fl)
    char c, *fl;
{
    char s[30];

    pp_cut(1);
    sprintf(s, "%s:'%c'", fl, c);
    pp_tok(s);
}

static Bool_fld(b, fl)
    boolean b;
    char *fl;
{
    char s[30];

    pp_cut(1);
    sprintf(s,"%s:%s", fl, (b ? "true" : "false"));
    pp_tok(s);
}

# define int_fld(nop,fl)    Int_fld(p->nd.nop.fl, "fl")
# define real_fld(nop, fl)  Real_fld(p->nd.nop.fl, "fl")
# define str_fld(nop,fl)    Str_fld(p->nd.nop.fl, "fl")
# define char_fld(nop,fl)   Char_fld(p->nd.nop.fl, "fl") 
# define bool_fld(nop,fl)   Bool_fld(p->nd.nop.fl, "fl")

ifp_begin(p)   nodep p;    { begin_node(p,"["); }
ifp_end(p)     nodep p;    { end_node(p,"]"); }

begin_node(p,sym)
    nodep p;
    char *sym;
{
    char addr[20];

    if (p == null) return;
    pp_cut(1);
    pp_begin(p,0,0,f,f,f);
    pp_tok(sym);
    pp_tok(nodename(p));
    /* sprintf(addr, "(8#%o#)", (int)p); */
    /* pp_tok(addr);                     */
    pp_cut(0);
    pp_begin(p,4,0,f,f,t);
    switch (p -> ndf.op) {
        case nop_count:                         /* counter_node */
                int_fld(counter,count);
                break;
        case nop_comment:                       /* comment_node */
                str_fld(comment,text);
                break;
        case nop_abort:                         /* abort_node */
                break;       
        case nop_accept:                        /* accept_node */
                break;
        case nop_access:                        /* access_node */
                break;
        case nop_agg_comp:                      /* agg_comp_node */
                break;
        case nop_aggregate:                     
        case nop_qual_expr:                     /* aggregate_node */
                break;
        case nop_arg:                           /* arg_node */
                str_fld(arg,name);
                break;
        case nop_array:                         /* array_node */
                break;
        case nop_asgn:                          /* asgn_stmt_node */
                break;
        case nop_amper:
        case nop_and:
        case nop_andthen:
        case nop_div:
        case nop_equal:
        case nop_expon:                             
        case nop_ge:
        case nop_gt:
        case nop_le:
        case nop_lt:
        case nop_minus:
        case nop_mod:
        case nop_nequal:
        case nop_or:
        case nop_orelse:
        case nop_plus:
        case nop_rem:
        case nop_star:
        case nop_xor:                           /* binop_node */
                break;
        case nop_call:                          
        case nop_proc_call:                     /* call_node */
                break;
        case nop_case:
        case nop_rec_var:                       /* case_node */
                break;
        case nop_char_enum:                     /* char_enum_node */
                char_fld(char_enum,ch);
                break;
        case nop_choice:                        /* choice_node */
                break;
        case nop_code:                          /* code_stmt_node */
                break;
        case nop_cond_ent:                      /* cond_entry_node */
                break;
        case nop_delay:                         /* delay_node */
                break;
        case nop_derived_type:                  /* derived_type_node */
                break;
        case nop_elsif:                         /* elsif_node */
                break;
        case nop_entry:                         /* entry_node */
                break;
        case nop_enum:                          /* enum_node */
                break;
        case nop_except_decl:                   /* except_decl_node */
                break;
        case nop_exit:                          /* exit_node */
                break;
        case nop_expr:                          /* expr_node */
                break;
        case nop_for:                           /* for_node */
                str_fld(forx,cntrl_var);
                bool_fld(forx,reversex);
		str_fld(forx,label);
                break;
        case nop_gen_with:                      /* gen_with_node */
                bool_fld(gen_with,box);
                break;
        case nop_generic:                       /* generic_node */
                break;
        case nop_goto:                          /* goto_node */
                int_fld(gotox,gen_number);
                break;
        case nop_id_decl:                       /* id_decl_node */
	 	str_fld(id_decl,id);
                break;
        case nop_ident: 
        case nop_lb:
        case nop_oper_string:
        case nop_string:                        /* ident_node */
	        str_fld(ident,id);
                break;
        case nop_if:                            /* if_stmt_node */
                break;
        case nop_in:                            /* in_node */
                break;
        case nop_index:                         /* index_node */
                break;
        case nop_instant:                       /* instant_node */
                break;
        case nop_char:                          
        case nop_int:                           /* int_node */
                str_fld(integer,prval);
		int_fld(integer,val);
                break;         
        case nop_at:                            /* location_node */
                break;
        case nop_name:                          /* name_list_node */
                break;
        case nop_new:                           /* new_node */
                break;
        case nop_null:                          /* null_node */
                break;
        case nop_null_stmt:                     /* null_stmt_node */
                break;
        case nop_discriminant:                  
        case nop_obj_decl:                      /* obj_decl_node */
                bool_fld(obj_decl,is_constant);
                break;
        case nop_obj_ren:                       /* obj_rename_node */
                break;
        case nop_pack_ren:                      
        case nop_task_ren:                      /* pack_ren_node */
                str_fld(pack_ren,id);
                break;
        case nop_body:
        case nop_pack_ginst:
        case nop_package:                       /* package_node */
		bool_fld(packagex,is_separate);
                break;
        case nop_param:                         /* param_decl_node */
                break;
        case nop_pragma:                        /* pragma_node */
                str_fld(pragmax,id);
	        break;
        case nop_raise:                         /* raise_node */
                break;
        case nop_delta:                         
        case nop_digits:
        case nop_range:                         /* range_node */
                bool_fld(rangex,not_constrained);
                break;
        case nop_real:                          /* real_node */
                str_fld(real,prval);
		real_fld(real,val);
                break;
        case nop_rec_spec:                      /* rec_rspec_node */
                break;
        case nop_record:                        /* record_node */
                break;
        case nop_addr_spec:                     
        case nop_len_spec:                      /* rep_spec_node */
                break;
        case nop_return:                        /* break_stmt_node */
                break;
        case nop_dot:             
        case nop_quote:                         /* select_node */
                break;
        case nop_sel_when:                      /* select_when_node */
                break;
        case nop_select:                        /* select_wait_node */
                break;
        case nop_disc_type:                     
        case nop_err:                           
        case nop_limited:
        case nop_private:
        case nop_terminate:                     /* simple_node */
                break;
        case nop_declare:                       
        case nop_function:
        case nop_func_ginst:
        case nop_func_ren:        
        case nop_procedure:                     
        case nop_proc_ginst:
        case nop_proc_ren:                      /* subp_node */
                str_fld(subp,name);
		bool_fld(subp,is_separate);
		bool_fld(subp,is_oper);
                break;   
        case nop_subunit:                       /* subunit_node */
                break;
        case nop_task:          
        case nop_task_body:                     
        case nop_task_type:                     /* task_node */
                str_fld(taskx,name);
		bool_fld(taskx,is_separate);
                break;
        case nop_timed_ent:                     /* time_entry_node */
                break;
        case nop_subtype:                      
        case nop_type:                         /* type_decl_node */
                str_fld(type_decl,id);
                break;
        case nop_lp:                            
        case nop_neg:                        
        case nop_not:                           
        case nop_pos:                           /* uop_node (unary_node) */
                break;
        case nop_use:
        case nop_with:                          /* use_node */
                break;
        case nop_rec_when:                      
        case nop_when:                          /* when_node */
                break;
        case nop_loop:                          
        case nop_while:                         /* while_node */
                str_fld(whilex,label);
                break;

		/* Castor */

	case nop_macro:
		break;
	case nop_macro_desig:
	case nop_decl_callf:
	case nop_expr_callf:
	case nop_name_callf:
	case nop_stmt_callf:
	case nop_top_callf:
	case nop_type_callf:
		break;
	case nop_macro_param:
		break;
	case nop_macro_arr_param:
		break;
	case nop_macro_param_list:
	case nop_callf_arg_list:
	case nop_callf_stmt_arg:
		break;
	case nop_callf_ident:
		break;
	case nop_decl_subst:
	case nop_expr_subst:
	case nop_name_subst:
	case nop_stmt_subst:
	case nop_type_subst:
	case nop_subtype_subst:
	case nop_ident_subst:
		break;
	default: 
	    {
		char *s[30];

		sprintf(s, "[Invalid node #%d]", p->ndf.op);	
		pp_tok(s);
	    }
	    break;
    }
}

end_node(p,term)
    nodep p;
    char *term;
{
    if (p == null) return;
    if (p->ndf.ag) {
	p = p->ndf.ag;
	while (p) {
	    begin_node(p,"<");
	    end_node(p,">");
	    p = p->ndf.next;
	}
    }
    pp_end();
    pp_cut(1);
    pp_tok(term);
    pp_end();
}

