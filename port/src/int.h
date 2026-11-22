/* $Header:   /usr/ics/darpa/ada/RCS/int.h.v  Revision 1.4  83/06/14  16:36:39  ada  Exp$ */

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


/* $Log:	/usr/ics/darpa/ada/RCS/int.h.v $
Revision 1.4  83/06/14  16:36:39  ada
Merged 1.2.40.1 and 1.2.5.1.  Version 1.3 is from a failed experiment.
Ignore it.


Revision 1.2.5.1  83/05/26  13:59:42  snider
Fixed a problem with the reserved word "range" not being prettyprinted
appropriately.

Revision 1.2.40.1  83/06/14  13:57:58  tadman
Deleted comp_unit_node.  Added context field to subp_node, 
macro_node, package_node, and subunit_node.

Revision 1.2  83/01/12  09:04:04  ada
Added copyright notice.

Revision 1.1  82/12/12  21:34:58  tadman
Initial revision (Arcturus release 1.3E)
 */

/* This files defines the nodes of the internal form parse tree */
/* Each node has a fixed and a variant part.  If p is a pointer */
/* to a node then a field in the fixed part (ag for example)	*/
/* is referenced by: 						*/
/*			p->ndf.ag				*/
/*								*/
/* The type of the node (p->ndf.op) determines the format of	*/
/* variant part.  To reference a field in the variant part	*/
/* the format is (for node type whilex, field bodyx):		*/
/*								*/
/*			p->nd.whilex.bodyx			*/
/*								*/
/* The file int.ada describes the internal form in ada.  In 	*/
/* order to keep that file consistant with this one, the 	*/
/* node and field names in this file should not be C or ada	*/
/* reserved words.  In some cases, "x" has been appended	*/
/* to an identifier to avoid this.				*/

typedef struct node *nodep;

typedef struct symtabform *symtabp;

int	curr_generation;	/* for symbol pointer binding trick */

struct node {
struct F {
	enum tokentypes op;
	nodep ag;
	nodep next;
} ndf;

union {				/* union for internal form variant parts */

struct comment_node {
        char *text;
} comment;

struct counter_node {
	int count;
} counter;

struct abort_node {
        nodep tasks; 
} abortx;

struct accept_node {
        nodep name;
        nodep stmts;
} acceptx;

struct access_node {
        nodep target_type;
} accessx;

struct agg_comp_node {
	nodep choice;
	nodep expr;
} agg_comp;

struct aggregate_node {
        nodep qualifier;
	nodep agg;
} aggregate;

struct arg_node {
        nodep exp;
        char *name;
} arg;

struct array_node {
        nodep index_list;
        nodep ele_type;
} arrayx;

struct asgn_stmt_node {
        nodep lhs;
        nodep rhs;
} asgn_stmt;

struct binop_node {
        nodep left;
        nodep right;
} binop;

struct call_node {
        nodep var;
        nodep args;
} call;

struct call_form_node {
	nodep list;
	int items;
	nodep refined_to;	 /* warning:  This pointer is a pointer back
				   to the internal form.  It should not
				   be traversed by an enumerating function
				   because the ag list and the refined_to
				   together are circular (for Gene) */
} call_form;

struct case_node {
        nodep expr;
        nodep when_list;
} casex;

struct char_enum_node {
        char ch;
} char_enum;

struct choice_node {
        nodep choice;
} choice;

struct code_node {
	nodep expr;
} code;

struct cond_entry_node {
	nodep call;
	nodep stmt;
	nodep else_part;
} cond_entry;

struct delay_node {
        nodep expr;
} delayx;

struct derived_type_node {
        nodep typex;
} derived_type;

struct elsif_node {
        nodep expr;
        nodep then_part;
} elsifx;

struct entry_node {
        char *id;
        nodep rangex;
        nodep formal;
} entryx;

struct enumtype_node {
        nodep ids;
} enumtype;

struct except_decl_node {
        nodep id_list;
} except_decl;
 
struct exit_node {
        nodep loop_name;
        nodep cond;
} exitx;

struct expr_node {          /* for typing expressions at top level */
        nodep expr;
} expr;
 
struct for_node {
        char *cntrl_var;
        boolean reversex;
        nodep lims;
        nodep bodyx;
	char *label;
	symtabp table;
} forx;

struct gen_with_node {
	nodep subp;
	nodep name;
	boolean box;
} gen_with;

struct generic_node {
	nodep context;
	nodep param;
	nodep unit;
} genericx;

struct goto_node {
        nodep label;
		/* the following are for the interpreter */
	nodep dest;
	nodep throw_dest;
	int gen_number;
} gotox;

struct id_decl_node {
        char *id;
} id_decl;

struct ident_node {
        char *id;
	int *curr_binding;	/* hack to get around symp */
	int gen_number;
} ident;
 
struct if_node {
        nodep expr;
        nodep then_part;
        nodep elsif_list;
} ifx;

struct index_node { 	/* this is used as an argnode, the index_type field */
                        /* should be the same as the arg_exp field */
        nodep typex;
} index;

struct inop_node {
        boolean not_in;
        nodep expr;
        nodep rangex;
} inop;

struct instant_node {
      	char *name;
	boolean is_oper;
	nodep arg;
} instant;

struct integer_node {
        int val;
        char *prval;
} integer;
                 
struct location_node {
	nodep name;
	nodep expr;
	nodep rangex;
} location;

struct macro_node {
        nodep designator;
	boolean is_separate;
	boolean is_oper;
        nodep ret_type;
	nodep context;
        nodep decl;
        nodep bodyx;
        nodep except;
	symtabp table;
} macrox;

struct macro_list_node {
	nodep list;
} macro_list;

struct name_list_node {
        nodep name;
} name_list;

struct new_node {
        nodep type_id;
} newx;

struct obj_decl_node {
        nodep id_list;
        nodep subtypex;
        nodep initexp;
        boolean is_constant;
} obj_decl;

struct obj_rename_node {
	nodep id_list;
	nodep typex;
	nodep old_name;
} obj_rename;

struct pack_ren_node {
	char *id;
	nodep old_name;
} pack_ren;

struct package_node {
        char *name;
	nodep context;
        nodep decl;
        nodep priv_decl;
        nodep bodyx;
        nodep except;
	boolean is_separate;
	symtabp table;
} packagex;

struct param_decl_node {
        nodep id_list;
        nodep subtyp;
        nodep initexp;
        boolean is_in;
        boolean is_out;
} param_decl;

struct pdl_node {
    char *pdl;		/* temp until things get changed */
    nodep cform;
    nodep decls;  	/* maybe list for these later */
    int num_decls;
    nodep stmts;
    int num_stmts;
} pdl;

struct pragma_node {
        char *id;
        nodep args;
} pragmax;

struct raise_node {
        nodep except;
} raisex;

struct range_node {
        nodep type_id;
        nodep right;
        nodep left;
	nodep accuracy_expr;
        boolean not_constrained;
	boolean is_range_constraint;	/* info. for prettyprinter about   */
					/* whether to print "range" or not */
} rangex;

struct real_node {
        float val;
        char *prval;
} real;
                 
struct rec_rspec_node {
	nodep name;
	nodep algn;
	nodep loc;
} rec_rspec;

struct record_node {
        nodep field_list;
} recordx;

	/* also used for accuracy constraints and derived types */
struct rep_spec_node {
	nodep name;
	nodep expr;
} rep_spec;

struct return_node {
        nodep val;
} returnx;

struct select_node {
	boolean is_oper;
        nodep var;
        char *field;
} selectx;

struct select_wait_node {
	nodep or_list;	
} select_wait;

struct select_when_node {
	nodep whenx;
	nodep alt;
} select_when;

struct simple_node {
    char waste;
} simple;

struct subp_node {
        char *name;
	boolean is_separate;
	boolean is_oper;
	nodep context;
        nodep spec;
        nodep ret_type;
        nodep decl;
        nodep bodyx;
        nodep except;
	symtabp table;
} subp;

struct subunit_node {
	nodep name;
	nodep context;
	nodep bodyx;
} subunitx;

struct task_node {
        char *name;
	nodep entryx;
        nodep rep_spec;
        nodep decl;
        nodep bodyx;
        nodep except;
	boolean is_separate;
	symtabp table;
} taskx;

struct timed_entry_node {
	nodep call;
	nodep stmt;
	nodep or_part;
} timed_entry;
    
struct type_decl_node {
        char *id;
        nodep disc;
        nodep typex;
} type_decl;

struct uop_node {
        nodep child;
} uop;

struct use_node {
	nodep list;
} usex;

struct when_node {
        nodep choice_list;
        nodep stmt;
} whenx;

struct while_node {
        nodep expr;
        nodep bodyx;
	char *label;
} whilex;


} nd;

};


#define newnode(structtag, op) \
	newnode1(sizeof(struct F)+sizeof(struct structtag), op)

nodep newnode1();
