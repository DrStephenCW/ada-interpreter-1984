/* $Header:   /ub/darpa/ada/RCS/nodes.h.v  Revision 1.4  84/01/12  16:16:49  ada  Exp$ */

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


/* $Log:	/ub/darpa/ada/RCS/nodes.h.v $
Revision 1.4  84/01/12  16:16:49  ada
got rid of xxdebugflag - it's not used anymore

Revision 1.3  83/06/14  16:31:41  ada
Same as 1.2.4.1.

Revision 1.2.4.1  83/06/14  14:03:27  tadman
Delete nop_comp_unit.

Revision 1.2  83/01/12  09:04:16  ada
Added copyright notice.

Revision 1.1  82/12/12  21:36:14  tadman
Initial revision (Arcturus release 1.3E)
 */

/************************************************************************
*   This file contains a list which is the discriminant			*
*   of the nodes in the internal program form.  It is also used to 	*
*   generate ntabs.c, which contains tables parallel to this		*
*   enumeration.  Each element has an attached comment of the form:	*
*									*
*	/* $ ~string1~  ~string2~  node * /				*
*         ^                              ^  omit these spaces		*
*									*
*   ~string1~	Name of node in ada					*
*   ~string2~   Name of node in C.  If omitted, same as ~string1~	*
*   node	Name of C union tag for corresponding node (see int.h)	*
************************************************************************/

enum tokentypes {

nop_comment,		/*$ ~comment~ 		comment_node */
nop_count,		/*$ ~count~ 		counter_node */

nop_abort,		/*$ ~abort~ 		abort_node */
nop_accept,		/*$ ~accept~ 		accept_node */
nop_access,		/*$ ~access~ 		access_node */
nop_addr_spec,		/*$ ~addr_spec~ 	rep_spec_node */
nop_agg_comp,		/*$ ~agg_comp~ 		agg_comp_node */
nop_aggregate,		/*$ ~aggregate~ 	aggregate_node */
nop_amper,		/*$ ~&~ 		binop_node */
nop_and,		/*$ ~and~  ~&~		binop_node */
nop_andthen,		/*$ ~and then~  ~&&~	binop_node */
nop_arg,		/*$ ~arg~ 		arg_node */
nop_array,		/*$ ~array~ 		array_node */
nop_asgn,		/*$ ~:=~  ~=~		asgn_stmt_node */
nop_at,			/*$ ~at~ 		location_node */
nop_body,		/*$ ~body~ 		package_node */
nop_call,		/*$ ~call~ 		call_node */
nop_callf_arg_list,	/*$ ~callf_arg_list~  	macro_list_node */
nop_callf_ident,	/*$ ~callf_ident~ 	id_decl_node */
nop_callf_stmt_arg,	/*$ ~callf_stmt_arg~ 	macro_list_node */
nop_case,		/*$ ~case~		case_node */
nop_char,		/*$ ~char~		integer_node */
nop_char_enum,		/*$ ~char_enum~		char_enum_node */
nop_choice,		/*$ ~choice~		choice_node */
nop_code,		/*$ ~code~		code_node */
nop_cond_ent,		/*$ ~cond_ent~		cond_entry_node */
nop_declare,		/*$ ~declare~		subp_node */
nop_decl_callf,		/*$ ~decl_callf~ 	call_form_node */
nop_decl_subst,		/*$ ~decl_subst~ 	ident_node */
nop_delay,		/*$ ~delay~		delay_node */
nop_delta,		/*$ ~delta~		range_node */
nop_derived_type,	/*$ ~derived_type~ 	derived_type_node */
nop_digits,		/*$ ~digits~		range_node */
nop_disc_type,		/*$ ~disc_type~		simple_node */
nop_discriminant,	/*$ ~discriminant~ 	obj_decl_node */
nop_div,		/*$ ~/~			binop_node */
nop_dot,		/*$ ~.~			select_node */
nop_elsif,		/*$ ~elsif~		elsif_node */
nop_entry,		/*$ ~entry~		entry_node */
nop_enum,		/*$ ~enum~		enumtype_node */
nop_equal,		/*$ ~=~	 ~==~ 		binop_node */
nop_err,		/*$ ~*err*~		simple_node */
nop_except_decl,	/*$ ~except_decl~ 	except_decl_node */
nop_except_ren,		/*$ ~except_ren~ 	obj_rename_node */
nop_exit,		/*$ ~exit~		exit_node */
nop_expon,		/*$ ~**~  ~expon~	binop_node */
nop_expr,		/*$ ~expr~		expr_node */
nop_expr_callf,		/*$ ~expr_callf~	call_form_node */
nop_expr_subst,		/*$ ~expr_subst~ 	ident_node */
nop_for,		/*$ ~for~		for_node */
nop_func_ginst,		/*$ ~func_ginst~ 	subp_node */
nop_func_ren,		/*$ ~func_ren~		subp_node */
nop_function,		/*$ ~function~		subp_node */
nop_ge,			/*$ ~>=~		binop_node */
nop_gen_with,		/*$ ~gen_with~		gen_with_node */
nop_generic,		/*$ ~generic~		generic_node */
nop_goto,		/*$ ~goto~		goto_node */
nop_gt,			/*$ ~>~			binop_node */
nop_id_decl,		/*$ ~id_decl~		id_decl_node */
nop_ident,		/*$ ~ident~		ident_node */
nop_ident_subst,	/*$ ~ident_subst~	ident_node */
nop_if,			/*$ ~if~		if_node */
nop_in,			/*$ ~in~		inop_node */
nop_index,		/*$ ~index~		index_node */
nop_instant,		/*$ ~instant~		instant_node */
nop_int,		/*$ ~int~		integer_node */
nop_lb,			/*$ ~lb~		ident_node */
nop_le,			/*$ ~<=~		binop_node */
nop_len_spec,		/*$ ~len_spec~		rep_spec_node */
nop_limited,		/*$ ~limited~		simple_node */
nop_loop,		/*$ ~loop~		while_node */
nop_lp,			/*$ ~(~			uop_node */
nop_lt,			/*$ ~<~			binop_node */
nop_macro,		/*$ ~macro~		macro_node */
nop_macro_desig,	/*$ ~macro_desig~ 	call_form_node */
nop_macro_arr_param,	/*$ ~macro_arr_param~	param_decl_node */
nop_macro_param,	/*$ ~macro_param~ 	param_decl_node */
nop_macro_param_list,	/*$ ~macro_param_list~ 	macro_list_node */
nop_minus,		/*$ ~-~			binop_node */
nop_mod,		/*$ ~mod~		binop_node */
nop_name,		/*$ ~name~		name_list_node */
nop_name_callf,		/*$ ~name_callf~	call_form_node */
nop_name_subst,		/*$ ~name_subst~	ident_node */
nop_neg,		/*$ ~-~			uop_node */
nop_nequal,		/*$ ~/=~  ~!=~		binop_node */
nop_new,		/*$ ~new~		new_node */
nop_not,		/*$ ~not~  ~!~		uop_node */
nop_null,		/*$ ~null~		simple_node */
nop_null_stmt,		/*$ ~null_stmt~		simple_node */
nop_obj_decl,		/*$ ~obj_decl~		obj_decl_node */
nop_obj_ren,		/*$ ~obj_ren~		obj_rename_node */
nop_oper_string,	/*$ ~oper_string~ 	ident_node */
nop_or,			/*$ ~or~  ~|~		binop_node */
nop_orelse,		/*$ ~or else~	~||~	binop_node */
nop_pack_ginst,		/*$ ~pack_ginst~	package_node */
nop_pack_ren,		/*$ ~pack_ren~		pack_ren_node */
nop_package,		/*$ ~package~		package_node */
nop_param,		/*$ ~param~		param_decl_node */
nop_plus,		/*$ ~+~			binop_node */
nop_pos,		/*$ ~+~			uop_node */
nop_pragma,		/*$ ~pragma~		pragma_node */
nop_private,		/*$ ~private~		simple_node */
nop_proc_call,		/*$ ~proc_call~		call_node */
nop_proc_ginst,		/*$ ~proc_ginst~	subp_node */
nop_proc_ren,		/*$ ~proc_ren~		subp_node */
nop_procedure,		/*$ ~procedure~		subp_node */
nop_qual_expr,		/*$ ~qual_expr~		aggregate_node */
nop_quote,		/*$ ~\"~  ~quote~	select_node */
nop_raise,		/*$ ~raise~		raise_node */
nop_range,		/*$ ~range~		range_node */
nop_real,		/*$ ~real~		real_node */
nop_rec_spec,		/*$ ~rec_spec~		rec_rspec_node */
nop_rec_var,		/*$ ~rec_var~		case_node */
nop_rec_when,		/*$ ~rec_when~		when_node */
nop_record,		/*$ ~record~		record_node */
nop_rem,		/*$ ~rem~		binop_node */
nop_return,		/*$ ~return~		return_node */
nop_sel_when,		/*$ ~sel_when~		select_when_node */
nop_select,		/*$ ~select~		select_wait_node */
nop_star,		/*$ ~*~			binop_node */
nop_stmt_callf,		/*$ ~stmt_callf~	call_form_node */
nop_stmt_subst,		/*$ ~stmt_subst~	ident_node */
nop_string,		/*$ ~string~		ident_node */
nop_subtype,		/*$ ~subtype~		type_decl_node */
nop_subtype_subst,	/*$ ~subtype_subst~	ident_node */
nop_subunit,		/*$ ~subunit~		subunit_node */
nop_task,		/*$ ~task~		task_node */
nop_task_body,		/*$ ~task_body~		task_node */
nop_task_ren,		/*$ ~task_ren~		pack_ren_node */
nop_task_type,		/*$ ~task_type~		task_node */
nop_terminate,		/*$ ~terminate~		simple_node */
nop_timed_ent,		/*$ ~timed_ent~		timed_entry_node */
nop_top_callf,		/*$ ~top_callf~		call_form_node */
nop_type,		/*$ ~type~		type_decl_node */
nop_type_callf,		/*$ ~type_callf~	call_form_node */
nop_type_subst,		/*$ ~type_subst~	ident_node */
nop_use,		/*$ ~use~		use_node */
nop_when,		/*$ ~when~		when_node */
nop_while,		/*$ ~while~		while_node */
nop_with,		/*$ ~with~		use_node */
nop_xor,		/*$ ~xor~  ~^~		binop_node */

};

char *toktab[];
#define tokname(tok) (toktab[(int)tok])
#define nodename(p) (tokname(p->ndf.op))

typedef enum tokentypes tokenlist[];

int     *intdebugp;
#define intdebug (*intdebugp)
