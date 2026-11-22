static char rcsid[] =
"$Header:   /usr/ics/darpa/ada/RCS/maptree.v  Revision 1.3  83/06/14  16:31:04  ada  Exp$";

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


/* $Log:	/usr/ics/darpa/ada/RCS/maptree.v $
 * Revision 1.3  83/06/14  16:31:04  ada
 * Same as 1.2.4.1.
 * 
 * Revision 1.2.4.1  83/06/14  14:01:51  tadman
 * Deleted case for nop_comp_unit.  Added calls to traverse context links.
 * 
 * Revision 1.2  83/01/12  09:17:38  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:33:46  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>              
#include "macros.h"
#include "nodes.h"
#include "int.h"

static (*pre_func)(), (*post_func)(),
                (*pre_list_func)(), (*post_list_func)(),
                (*Map)();


 /****************************************************************
maptree traverses the internal form applying functions at each node and
before and after each list of nodes.  The functions are:

pre_fn(n,p) is applied to each node before its children are visited.
n is a pointer to the node.  p is a pointer to the parents pointer to
this node.  It is used for copying or modifying the tree.  If *p is
changed by pre_fn, maptree will visit the children of the node pointed
to by the modified pointer.

post_fn(n,p) is applied to each node after its children are visited.
n and p are as for pre_fn.  Changing *p does not affect maptree's traversal.

pre_lst_fn(n,p) is applied to each list before its nodes are visited.
A list is a sequence of nodes linked together by the ndf.next field. 
n is a pointer to the first node in the list.  p is a pointer to the
parent's pointer to this node.  For the first node, this is the 
pointer to the list. For all other nodes, it is the address of the 
previous ndf.next field. If *p is changed by pre_lst_fn, maptree will
visit the children of the node pointed to by the modified pointer.

post_lst_fn(n,p) is applied to each list after its nodes are visited. 
n and p are as for pre_list_fn (yes, n points to the first node of the
list, not the last).  Changing *p does not affect maptree's traversal.

maptree returns a pointer to the root of the tree.  This is normally the
same as the original root supplied in ptr.  However, if on of the
above functions used its p argument to modified the pointer to the root,
the modified pointer is returned.

The function smaptree() is the same as maptree, with the addition of a
new second argument.  The second argument is a pointer to a function
which is called in place of map (the recursive part of maptree).  This
function can specially process selected nodes while calling map for
nodes which are to be processed normally.

maptree() and smaptree() are reentrant so they must save all globals and
restore them.  

maptree() and smaptree() do not traverse the ndf.next field of the root
of the tree.  maptreelist() and smaptreelist() are the same as maptree()
and smaptree() respectively except that they do traverse the ndf.next field.
 ****************************************************************/

map();  /* forward declaration */

nodep smaptree(ptr, Map_fn, pre_fn, post_fn, pre_list_fn, post_list_fn)
    nodep ptr;	
    int (*Map_fn)();
    int (*pre_fn)(), (*post_fn)(), (*pre_list_fn)(), (*post_list_fn)();
{
    int (*Map_fn_sav)();
    int (*pre_fn_sav)(), (*post_fn_sav)(), 
	(*pre_list_fn_sav)(), (*post_list_fn_sav)();

    Map_fn_sav = Map;
    pre_fn_sav = pre_func;
    post_fn_sav = post_func;
    pre_list_fn_sav = pre_list_func;
    post_list_fn_sav = post_list_func;

    pre_func = pre_fn;
    post_func = post_fn;
    pre_list_func = pre_list_fn;
    post_list_func = post_list_fn;
    Map = Map_fn;    

    (*Map)(&ptr);

    Map = Map_fn_sav;
    pre_func = pre_fn_sav;
    post_func = post_fn_sav;
    pre_list_func = pre_list_fn_sav;
    post_list_func = post_list_fn_sav;

    return ptr;
}

nodep maptree(ptr, pre_fn, post_fn, pre_list_fn, post_list_fn)
    register nodep ptr;
    int (*pre_fn)(), (*post_fn)(), (*pre_list_fn)(), (*post_list_fn)();
{
    return smaptree(ptr, map, pre_fn, post_fn, pre_list_fn, post_list_fn);
}

nodep smaptreelist(ptr, Map_fn, pre_fn, post_fn, pre_list_fn, post_list_fn)
    register nodep ptr;
    int (*Map_fn)();
    int (*pre_fn)(), (*post_fn)(), (*pre_list_fn)(), (*post_list_fn)();
{
    nodep *q;

    ptr = smaptree(ptr, Map_fn, pre_fn, post_fn, pre_list_fn, post_list_fn);
    if (ptr) {
	q = &(ptr->ndf.next);
	while (*q) {
	    *q = smaptree(*q, Map_fn, pre_fn, post_fn, 
			 pre_list_fn, post_list_fn);
	    q = &((*q)->ndf.next);
	}
    }
    return ptr;
}

nodep maptreelist(ptr, pre_fn, post_fn, pre_list_fn, post_list_fn)
    register nodep ptr;
    int (*pre_fn)(), (*post_fn)(), (*pre_list_fn)(), (*post_list_fn)();
{
    return smaptreelist(ptr, map, pre_fn, post_fn, pre_list_fn, post_list_fn);
}

nill() {}               /* use this for unneeded functional arguments */

maplist(ptr)
    nodep *ptr;
{
    register nodep *p = ptr;

    if (*p == null) return;

    (*pre_list_func)(*p,p);
    while (*p) {
        (*Map)(p);
        p = &((*p)->ndf.next);
    }
    (*post_list_func)(*ptr,ptr);
}

map(ptr)
    register nodep *ptr;
{
    if (*ptr == null) return;

    (*pre_func)(*ptr,ptr);	/* apply the function to current node */
 
    switch ((*ptr)->ndf.op) {
        case nop_abort:                         /* abort_node */
                maplist(&((*ptr)->nd.abortx.tasks));
                break;       
        case nop_accept:                        /* accept_node */
                (*Map)(&((*ptr)->nd.acceptx.name));
                maplist(&((*ptr)->nd.acceptx.stmts));
                break;
        case nop_access:                        /* access_node */
                (*Map)(&((*ptr)->nd.accessx.target_type));
                break;
        case nop_agg_comp:                      /* agg_comp_node */
                (*Map)(&((*ptr)->nd.agg_comp.choice));
                (*Map)(&((*ptr)->nd.agg_comp.expr));
                break;
        case nop_aggregate:                     
        case nop_qual_expr:                     /* aggregate_node */
                (*Map)(&((*ptr)->nd.aggregate.qualifier));
                (*Map)(&((*ptr)->nd.aggregate.agg));
                break;
        case nop_arg:                           /* arg_node */
                (*Map)(&((*ptr)->nd.arg.exp));
                break;
        case nop_array:                         /* array_node */
                maplist(&((*ptr)->nd.arrayx.index_list));
                (*Map)(&((*ptr)->nd.arrayx.ele_type));
                break;
        case nop_asgn:                          /* asgn_stmt_node */
                (*Map)(&((*ptr)->nd.asgn_stmt.lhs));
                (*Map)(&((*ptr)->nd.asgn_stmt.rhs));
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
                (*Map)(&((*ptr)->nd.binop.left));
                (*Map)(&((*ptr)->nd.binop.right));
                break;
        case nop_call:                          
        case nop_proc_call:                     /* call_node */
                (*Map)(&((*ptr)->nd.call.var));
                maplist(&((*ptr)->nd.call.args));
                break;
        case nop_case:
        case nop_rec_var:                       /* case_node */
                (*Map)(&((*ptr)->nd.casex.expr));
                maplist(&((*ptr)->nd.casex.when_list));
                break;
        case nop_char_enum:                     /* char_enum_node */
                break;
        case nop_choice:                        /* choice_node */
                (*Map)(&((*ptr)->nd.choice.choice));
                break;
        case nop_code:                          /* code_stmt_node */
                (*Map)(&((*ptr)->nd.code.expr));
                break;
        case nop_cond_ent:                      /* cond_entry_node */
                (*Map)(&((*ptr)->nd.cond_entry.call));
                maplist(&((*ptr)->nd.cond_entry.stmt));
                maplist(&((*ptr)->nd.cond_entry.else_part));
                break;
        case nop_delay:                         /* delay_node */
                (*Map)(&((*ptr)->nd.delayx.expr));
                break;
        case nop_derived_type:                  /* derived_type_node */
                (*Map)(&((*ptr)->nd.derived_type.typex));
                break;
        case nop_elsif:                         /* elsif_node */
                (*Map)(&((*ptr)->nd.elsifx.expr));
                maplist(&((*ptr)->nd.elsifx.then_part));
                break;
        case nop_entry:                         /* entry_node */
                (*Map)(&((*ptr)->nd.entryx.rangex));
                (*Map)(&((*ptr)->nd.entryx.formal));
                break;
        case nop_enum:                          /* enum_node */
                maplist(&((*ptr)->nd.enumtype.ids));
                break;
        case nop_except_decl:                   /* except_decl_node */
                maplist(&((*ptr)->nd.except_decl.id_list));
                break;
        case nop_exit:                          /* exit_node */
                (*Map)(&((*ptr)->nd.exitx.cond));
                break;
        case nop_expr:                          /* expr_node */
                (*Map)(&((*ptr)->nd.expr.expr));
                break;
        case nop_for:                           /* for_node */
                (*Map)(&((*ptr)->nd.forx.lims));
                maplist(&((*ptr)->nd.forx.bodyx));
                break;
        case nop_gen_with:                      /* gen_with_node */
                (*Map)(&((*ptr)->nd.gen_with.subp));
                (*Map)(&((*ptr)->nd.gen_with.name));
                (*Map)(&((*ptr)->nd.gen_with.box));   /* is this right? (fpt) */
                break;
        case nop_generic:                       /* generic_node */
                maplist(&((*ptr)->nd.genericx.context));
                maplist(&((*ptr)->nd.genericx.param));
                (*Map)(&((*ptr)->nd.genericx.unit));
                break;
        case nop_goto:                          /* goto_node */
                (*Map)(&((*ptr)->nd.gotox.label));
                break;
        case nop_id_decl:                       /* id_decl_node */
                break;
        case nop_ident: 
        case nop_lb:
        case nop_oper_string:
        case nop_string:                        /* ident_node */
                break;
        case nop_if:                            /* if_stmt_node */
                (*Map)(&((*ptr)->nd.elsifx.expr));
                maplist(&((*ptr)->nd.elsifx.then_part));
                maplist(&((*ptr)->nd.ifx.elsif_list));
                break;
        case nop_in:                            /* in_node */
                (*Map)(&((*ptr)->nd.inop.expr));
                (*Map)(&((*ptr)->nd.inop.rangex));
                break;
        case nop_index:                         /* index_node */
                (*Map)(&((*ptr)->nd.index.typex));
                break;
        case nop_instant:                       /* instant_node */
                (*Map)(&((*ptr)->nd.instant.name));
                maplist(&((*ptr)->nd.instant.arg));
                break;
        case nop_char:                          
        case nop_int:                           /* int_node */
                break;         
        case nop_at:                            /* location_node */
                (*Map)(&((*ptr)->nd.location.name));
                (*Map)(&((*ptr)->nd.location.expr));
                (*Map)(&((*ptr)->nd.location.rangex));
                break;
        case nop_name:                          /* name_list_node */
                (*Map)(&((*ptr)->nd.name_list.name));
                break;
        case nop_new:                           /* new_node */
                break;
        case nop_null:                          /* null_node */
                break;
        case nop_null_stmt:                     /* null_stmt_node */
                break;
        case nop_discriminant:                  
        case nop_obj_decl:                      /* obj_decl_node */
                maplist(&((*ptr)->nd.obj_decl.id_list));  
                (*Map)(&((*ptr)->nd.obj_decl.subtypex));
                (*Map)(&((*ptr)->nd.obj_decl.initexp));
                break;
        case nop_obj_ren:                       /* obj_rename_node */
                (*Map)(&((*ptr)->nd.obj_rename.id_list));
                (*Map)(&((*ptr)->nd.obj_rename.typex));
                (*Map)(&((*ptr)->nd.obj_rename.old_name));
                break;
        case nop_pack_ren:                      
        case nop_task_ren:                      /* pack_ren_node */
                (*Map)(&((*ptr)->nd.pack_ren.id));
                (*Map)(&((*ptr)->nd.pack_ren.old_name));
                break;
        case nop_body:
        case nop_pack_ginst:
        case nop_package:                       /* package_node */
                maplist(&((*ptr)->nd.packagex.context));
                maplist(&((*ptr)->nd.packagex.decl));
                maplist(&((*ptr)->nd.packagex.priv_decl));
                maplist(&((*ptr)->nd.packagex.bodyx));
                maplist(&((*ptr)->nd.packagex.except));
                break;
        case nop_param:                         /* param_decl_node */
                maplist(&((*ptr)->nd.obj_decl.id_list));
                (*Map)(&((*ptr)->nd.obj_decl.subtypex));
                (*Map)(&((*ptr)->nd.obj_decl.initexp));
                break;
        case nop_pragma:                        /* pragma_node */
                maplist(&((*ptr)->nd.pragmax.args));
                break;
        case nop_raise:                         /* raise_node */
                maplist(&((*ptr)->nd.raisex.except));
                break;
        case nop_delta:                         
        case nop_digits:
        case nop_range:                         /* range_node */
                (*Map)(&((*ptr)->nd.rangex.right));
                (*Map)(&((*ptr)->nd.rangex.left));
                (*Map)(&((*ptr)->nd.rangex.accuracy_expr));
                break;
        case nop_real:                          /* real_node */
                break;
        case nop_rec_spec:                      /* rec_rspec_node */
                (*Map)(&((*ptr)->nd.rec_rspec.name));
                (*Map)(&((*ptr)->nd.rec_rspec.algn));
                (*Map)(&((*ptr)->nd.rec_rspec.loc));
                break;
        case nop_record:                        /* record_node */
                maplist(&((*ptr)->nd.recordx.field_list));
                break;
        case nop_addr_spec:                     
        case nop_len_spec:                      /* rep_spec_node */
                (*Map)(&((*ptr)->nd.rep_spec.name));
                (*Map)(&((*ptr)->nd.rep_spec.expr));
                break;
        case nop_return:                        /* break_stmt_node */
                (*Map)(&((*ptr)->nd.returnx.val));
                break;
        case nop_dot:             
        case nop_quote:                         /* select_node */
                (*Map)(&((*ptr)->nd.selectx.var));
                break;
        case nop_sel_when:                      /* select_when_node */
                (*Map)(&((*ptr)->nd.select_when.whenx));
                (*Map)(&((*ptr)->nd.select_when.alt));
                break;
        case nop_select:                        /* select_wait_node */
                maplist(&((*ptr)->nd.select_wait.or_list));
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
                maplist(&((*ptr)->nd.subp.context));
                maplist(&((*ptr)->nd.subp.spec));
                (*Map)(&((*ptr)->nd.subp.ret_type));
                maplist(&((*ptr)->nd.subp.decl));
                maplist(&((*ptr)->nd.subp.bodyx));
                maplist(&((*ptr)->nd.subp.except));
                break;   
        case nop_subunit:                       /* subunit_node */
                (*Map)(&((*ptr)->nd.subunitx.context));
                (*Map)(&((*ptr)->nd.subunitx.name));
                (*Map)(&((*ptr)->nd.subunitx.bodyx));
                break;
        case nop_task:          
        case nop_task_body:                     
        case nop_task_type:                     /* task_node */
                (*Map)(&((*ptr)->nd.taskx.entryx));
                maplist(&((*ptr)->nd.taskx.rep_spec));
                maplist(&((*ptr)->nd.taskx.decl));
                maplist(&((*ptr)->nd.taskx.bodyx));
                maplist(&((*ptr)->nd.taskx.except));
                break;
        case nop_timed_ent:                     /* time_entry_node */
                (*Map)(&((*ptr)->nd.timed_entry.call));
                maplist(&((*ptr)->nd.timed_entry.stmt));
                maplist(&((*ptr)->nd.timed_entry.or_part));
                break;
        case nop_subtype:                      
        case nop_type:                         /* type_decl_node */
                maplist(&((*ptr)->nd.type_decl.disc));
                maplist(&((*ptr)->nd.type_decl.typex));
                break;
        case nop_lp:                            
        case nop_neg:                        
        case nop_not:                           
        case nop_pos:                           /* uop_node (unary_node) */
                (*Map)(&((*ptr)->nd.uop.child));          
                break;
        case nop_use:
        case nop_with:                          /* use_node */
                maplist(&((*ptr)->nd.usex.list));
                break;
        case nop_rec_when:                      
        case nop_when:                          /* when_node */
                maplist(&((*ptr)->nd.whenx.choice_list));
                maplist(&((*ptr)->nd.whenx.stmt));
                break;
        case nop_loop:                          
        case nop_while:                         /* while_node */
                (*Map)(&((*ptr)->nd.whilex.expr));
                maplist(&((*ptr)->nd.whilex.bodyx));
                break;

	/* Castor */

	case nop_macro:
		(*Map)(&((*ptr)->nd.macrox.context));
		(*Map)(&((*ptr)->nd.macrox.designator));
                (*Map)(&((*ptr)->nd.macrox.ret_type));
                maplist(&((*ptr)->nd.macrox.decl));
                maplist(&((*ptr)->nd.macrox.bodyx));
                maplist(&((*ptr)->nd.macrox.except));
		break;
	case nop_macro_desig:
	case nop_decl_callf:
	case nop_expr_callf:
	case nop_name_callf:
	case nop_stmt_callf:
	case nop_top_callf:
	case nop_type_callf:
		maplist(&((*ptr)->nd.call_form.list));
		break;
	case nop_macro_param:
	case nop_macro_arr_param:
		maplist(&((*ptr)->nd.param_decl.id_list));
		(*Map)(&((*ptr)->nd.param_decl.subtyp));
		(*Map)(&((*ptr)->nd.param_decl.initexp));
		break;
	case nop_macro_param_list:
	case nop_callf_arg_list:
	case nop_callf_stmt_arg:
		maplist(&((*ptr)->nd.macro_list.list));
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
        default: error("default reached for %s in map", nodename((*ptr)));
                break;
    }
    (*post_func)(*ptr,ptr);	/* apply function to node */
}
