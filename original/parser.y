/* $Header:   /ub/darpa/ada/RCS/parser.v  Revision 1.5  83/11/30  14:24:56  ada  Exp$ */

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

/* $Log:	/ub/darpa/ada/RCS/parser.v $
 * Revision 1.5  83/11/30  14:24:56  ada
 * same as 1.4.5.1
 * 
 * Revision 1.4.5.1  83/11/29  16:08:41  snider
 * Fixed a problem relating to prettyprinting "range".
 * 
 * Revision 1.4  83/06/14  16:33:40  ada
 * Merged 1.3.4.1 and 1.3.5.1.
 * 
 * Revision 1.3.5.1  83/05/26  14:05:27  snider
 * Fill in a new field in range nodes: "is_range_constraint", to provide
 * information to the prettyprinter about what kind of range node it is.
 * 
 * Revision 1.3.4.1  83/06/14  14:08:24  tadman
 * Changed the parsing of compilation units and context specifications.
 * Rather than having a comp_unit_node which points to them, the
 * subp_node, package_node, macro_node, generic_node, or subunit_node
 * has a pointer to the context specification.
 * 
 * Revision 1.3  83/03/17  14:24:22  ada
 * same as 1.2.4.1
 * 
 * Revision 1.2.4.1  83/03/17  11:26:56  tadman
 * Changed the binding of unary operators in a simple_expression.  It
 * used to bind to the entire adding_operator_list. It now bind to just
 * the first term.  (Example: old -4*5+2 = -22, new -4*5+2 = -18).
 * Reordered the productions to correspond roughly to the Ada82
 * reference manual (but didn't update the syntax to Ada82 yet).
 * 
 * Revision 1.2  83/01/12  09:39:53  ada
 * Added coyright notice.
 * 
 * Revision 1.1  82/12/12  21:35:18  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

%start thing_to_parse

%token <ident> '&'
%token <ident> '('
%token <ident> ')'
%token <ident> '*'
%token <ident> '+'
%token <ident> ','
%token <ident> '-'
%token <ident> '.'
%token <ident> '/'
%token <ident> ':'
%token <ident> ';'
%token <ident> '<'
%token <ident> '='
%token <ident> '>'
%token <ident> '\''
%token <ident> '|'
%token <ident> '$'			/* Castor */
%token <ident> '{'			/* Castor */
%token <ident> '}'			/* Castor */

%token <ident> ABORT
%token <ident> ACCEPT
%token <ident> ACCESS
%token <ident> ALL
%token <ident> AND
%token <ident> ARRAY
%token <ident> ASSIGN_OP
%token <ident> AT
%token <ident> XBEGIN
%token <ident> BODY
%token <ident> BOX
%token <ident> CASE
%token <ident> CONSTANT
%token <ident> DBL_DOLLAR		/* Castor */
%token <ident> DECLARE
%token <ident> DELAY
%token <ident> DELTA
%token <ident> DIGITS
%token <ident> DO
%token <ident> ELSE
%token <ident> ELSIF
%token <ident> END
%token <ident> ENTRY
%token <ident> EXCEPTION
%token <ident> EXIT
%token <ident> EXPONENTIAL
%token <ident> FOR
%token <ident> FUNCTION
%token <ident> GENERIC
%token <ident> GEQ
%token <ident> GOTO
%token <ident> IF
%token <ident> IN
%token <ident> IS
%token <ident> LEFT_LABEL_BRACKET
%token <ident> LEQ
%token <ident> LIMITED
%token <ident> LOOP
%token <ident> MACRO			/* Castor */
%token <ident> MOD
%token <ident> NEQ
%token <ident> NEW
%token <ident> NOT
%token <ident> XNULL
%token <ident> OF
%token <ident> OR
%token <ident> OTHERS
%token <ident> OUT
%token <ident> PACKAGE
%token <ident> PRAGMA
%token <ident> PRIVATE
%token <ident> PROCEDURE
%token <ident> RAISE
%token <ident> RANGE
%token <ident> RECORD
%token <ident> REM
%token <ident> RENAMES
%token <ident> RETURN
%token <ident> REVERSE
%token <ident> RIGHT_ARROW
%token <ident> RIGHT_LABEL_BRACKET
%token <ident> SELECT
%token <ident> SEPARATE
%token <ident> SUBRANGE_SYM
%token <ident> SUBTYPE
%token <ident> TASK
%token <ident> TERMINATE
%token <ident> THEN
%token <ident> TYPE
%token <ident> USE
%token <ident> WHEN
%token <ident> WHILE
%token <ident> WITH
%token <ident> XOR

%token <character> Character
%token <ident> Identifier
%token <integer> Integer
%token <ident> Operator_String
%token <real> Real
%token <ident> String

%type <nodepointer> abort_statement
%type <nodepointer> accept_statement
%type <nodepointer> access_type_definition
%type <nodepointer> accuracy_constraint
%type <token> adding_operator
%type <nodepointer> address_specification
%type <nodepointer> aggregate
%type <nodepointer> alignment_clause
%type <nodepointer> alignment_clause_optional
%type <nodepointer> allocator
%type <nodepointer> argument_item
%type <nodepointer> argument_list
%type <nodepointer> argument_part
%type <nodepointer> array_type_definition
%type <nodepointer> assignment_statement
%type <nodepointer> attribute
%type <nodepointer> basic_loop
%type <nodepointer> block
%type <nodepointer> body
%type <nodepointer> body_stub
%type <nodepointer> boolean_expression
%type <nodepointer> call_form_stmt_arg_list	/* Castor */
%type <nodepointer> calling_form		/* Castor */
%type <nodepointer> calling_form_argument	/* Castor */
%type <nodepointer> calling_form_argument_list	/* Castor */
%type <nodepointer> calling_form_item		/* Castor */
%type <nodepointer> calling_form_item_list	/* Castor */
%type <nodepointer> case_statement
%type <nodepointer> castor_declarative_item	/* Castor */
%type <nodepointer> castor_statement		/* Castor */
%type <nodepointer> character
%type <nodepointer> choice
%type <nodepointer> choice_stroke_list
%type <nodepointer> code_statement
%type <nodepointer> compilation
%type <nodepointer> compilation_unit
%type <nodepointer> component_association
%type <nodepointer> component_association_comma_list
%type <nodepointer> component_declaration
%type <nodepointer> component_declaration_null_list
%type <nodepointer> component_list
%type <nodepointer> component_name
%type <nodepointer> component_subtype_indication
%type <nodepointer> compound_statement
%type <nodepointer> condition
%type <nodepointer> conditional_entry_call
%type <nodepointer> context_specification
%type <nodepointer> decl_calling_form		/* Castor */
%type <nodepointer> decl_substitution_point	/* Castor */
%type <nodepointer> declaration
%type <nodepointer> declarative_item
%type <nodepointer> declarative_item_null_list
%type <nodepointer> declarative_part
%type <nodepointer> delay_statement
%type <nodepointer> derived_type_definition
%type <ident> designator			/* won't work */
%type <ident> designator_optional
%type <nodepointer> discrete_range
%type <nodepointer> discrete_range_list
%type <nodepointer> discriminant_declaration
%type <nodepointer> discriminant_declaration_semicolon_list
%type <nodepointer> discriminant_part
%type <nodepointer> discriminant_part_optional
%type <nodepointer> else_optional
%type <nodepointer> else_part_optional
%type <nodepointer> elsif_list
%type <nodepointer> entry_declaration
%type <nodepointer> entry_declaration_null_list
%type <nodepointer> enumeration_literal
%type <nodepointer> enumeration_literal_comma_list
%type <nodepointer> enumeration_type_definition
%type <nodepointer> exception_choice
%type <nodepointer> exception_choice_stroke_list
%type <nodepointer> exception_declaration
%type <nodepointer> exception_handler
%type <nodepointer> exception_handler_list
%type <nodepointer> exception_name
%type <nodepointer> exceptions_optional
%type <nodepointer> exit_statement
%type <nodepointer> expression
%type <nodepointer> factor
%type <nodepointer> factor_multiplying_operator_list
%type <nodepointer> fixed_point_constraint
%type <nodepointer> floating_point_constraint
%type <nodepointer> formal_part
%type <nodepointer> generic_formal_parameter
%type <nodepointer> generic_formal_parameter_list
%type <nodepointer> generic_instantiation
%type <nodepointer> generic_package_declaration
%type <nodepointer> generic_package_instantiation
%type <nodepointer1> generic_part
%type <nodepointer> generic_subprogram_declaration
%type <nodepointer> generic_subprogram_instantiation
%type <nodepointer> generic_type_definition
%type <nodepointer> goto_statement
%type <nodepointer> identifier_list
%type <nodepointer> identifier
%type <nodepointer> ident_list_item
%type <nodepointer> ident_substitution_point
%type <nodepointer> if_statement
%type <nodepointer> incomplete_type_declaration
%type <nodepointer> index
%type <nodepointer> index_comma_list
%type <nodepointer> index_constraint
%type <nodepointer> initialization_optional
%type <nodepointer> integer
%type <nodepointer> integer_type_definition
%type <nodepointer> interactive_input
%type <nodepointer> iteration_clause
%type <nodepointer> label
%type <nodepointer> label_list
%type <nodepointer> label_name
%type <nodepointer> length_or_enumeration_specification
%type <nodepointer> literal
%type <nodepointer> literal_expression
%type <nodepointer> location
%type <nodepointer> location_clause_null_list
%type <nodepointer> loop_name
%type <ident> loop_parameter
%type <nodepointer> loop_statement
%type <nodepointer> macro_array_parameter	/* Castor */
%type <nodepointer> macro_body			/* Castor */
%type <nodepointer> macro_designator		/* Castor */
%type <nodepointer> macro_designator_optional	/* Castor */
%type <nodepointer> macro_desig_item		/* Castor */
%type <nodepointer> macro_desig_item_list	/* Castor */
%type <nodepointer> macro_decl_semi_list	/* Castor */
%type <nodepointer> macro_param_decl		/* Castor */
%type <nodepointer> macro_param_list		/* Castor */
%type <nodepointer> macro_specification		/* Castor */
%type <param_mode> mode
%type <token> multiplying_operator
%type <nodepointer> name
%type <nodepointer> name1
%type <nodepointer> name_calling_form		/* Castor */
%type <nodepointer> name_substitution_point	/* Castor */
%type <nodepointer> null_statement
%type <nodepointer> number_declaration
%type <nodepointer> numeric_literal
%type <nodepointer> object_declaration
%type <nodepointer> operator_symbol
%type <nodepointer> or_part
%type <nodepointer> or_part_null_list
%type <nodepointer> package_body
%type <nodepointer2> package_body_part_optional
%type <nodepointer> package_declaration
%type <nodepointer> package_name
%type <nodepointer> package_name_comma_list
%type <nodepointer> package_specification
%type <nodepointer> parameter_declaration
%type <nodepointer> parameter_declaration_semicolon_list
%type <nodepointer> pragma
%type <nodepointer> primary
%type <nodepointer> private_part_optional
%type <nodepointer> private_type_definition
%type <nodepointer> procedure_or_entry_call
%type <nodepointer> program_component
%type <nodepointer> qualified_expression
%type <nodepointer> raise_statement
%type <nodepointer> range
%type <nodepointer> range_constraint
%type <nodepointer> real
%type <nodepointer> real_type_definition
%type <nodepointer> record_type_definition
%type <nodepointer> record_type_representation
%type <nodepointer> relation
%type <nodepointer> relation_and_list
%type <nodepointer> relation_and_then_list
%type <nodepointer> relation_or_else_list
%type <nodepointer> relation_or_list
%type <nodepointer> relation_xor_list
%type <token> relational_operator
%type <nodepointer> renaming_declaration
%type <nodepointer> representation_specification
%type <nodepointer> representation_specification_null_list
%type <nodepointer> return_statement
%type <nodepointer> select_alternative
%type <nodepointer> select_statement
%type <nodepointer> selected_component
%type <nodepointer> selective_wait
%type <nodepointer> sequence_of_statements
%type <nodepointer> sequence_of_statements_optional
%type <nodepointer> simple_expression
%type <nodepointer> simple_expression_relational_list
%type <nodepointer> simple_statement
%type <nodepointer> statement
%type <nodepointer> stmt_calling_form		/* Castor */
%type <nodepointer> stmt_substitution_point	/* Castor */
%type <nodepointer> static_simple_expression
%type <nodepointer> string
%type <nodepointer> subprogram_body
%type <nodepointer> subprogram_declaration
%type <nodepointer> subprogram_specification
%type <nodepointer> substitution_point		/* Castor */
%type <nodepointer> subtype_declaration
%type <nodepointer> subtype_indication
%type <nodepointer> subtype_substitution_point	/* Castor */
%type <nodepointer> subunit
%type <nodepointer> task_body
%type <nodepointer> task_declaration
%type <nodepointer> task_name
%type <nodepointer> task_name_comma_list
%type <nodepointer> task_specification
%type <nodepointer2> task_specification_part
%type <nodepointer> term
%type <nodepointer> term_adding_operator_list
%type <nodepointer> thing_to_parse
%type <nodepointer> timed_entry_call
%type <nodepointer> top_level_expression
%type <nodepointer> top_level_item
%type <nodepointer> top_level_list
%type <nodepointer> top_level_statement
%type <nodepointer> type_declaration
%type <nodepointer> type_definition
%type <nodepointer> type_calling_form		/* Castor */
%type <nodepointer> type_substitution_point	/* Castor */
%type <token> unary_operator
%type <nodepointer> unit_name
%type <nodepointer> unit_name_comma_list
%type <nodepointer> unlabeled_block
%type <nodepointer> unlabeled_compound_statement
%type <nodepointer> unlabeled_loop_statement
%type <nodepointer> use_clause
%type <nodepointer> variant_element_null_list
%type <nodepointer> variant_part
%type <nodepointer> when_null_list
%type <nodepointer> when_part_optional
%type <nodepointer> with_clause
%type <nodepointer> with_use_clause_null_list

%{
# include <stdio.h>
# include "macros.h"
# include "nodes.h"
# include "int.h"
# include "scan.h"
# include "oblist.h"
# include "parser.h"

    static nodep parsetree;
    extern int yylineno;
    extern boolean in_call_form;

# define yyerror(msg) 	{ vlookahead(yystate);  perror("%s",msg); }

%}

%%
			/* for a description of this see the parser
			   function below */
thing_to_parse	: '+' interactive_input
			{ parsetree = $2;  }
		| '&' expression
			{ parsetree = $2;  }
		| '%' name
			{ parsetree = $2;  }
		| '*' declarative_part
			{ parsetree = $2;  }
		| '-' sequence_of_statements
			{ parsetree = $2;  }
		| '=' type_definition
			{ parsetree = $2;  }
		| ':' subtype_indication
			{ parsetree = $2;  }
		| ';' compilation
			{ parsetree = $2;  }
		;
interactive_input :   /* accept just end-of-file */
			{ $$ = null; }
		| top_level_list
			{ $$ = $1; }
		| top_level_expression
			{ $$ = $1; }
		;
top_level_expression : expression
			{ $$ = $1;
			  if ($$->ndf.op == nop_expr_callf)
			      $$->ndf.op = nop_top_callf;
			  else {
			      $$ = newnode(expr_node, nop_expr);
			      $$->nd.expr.expr = $1;
			  }  }
		;
top_level_list : top_level_item
		| top_level_list top_level_item
			{ $$ = append($1,$2); }
		;
top_level_item	: top_level_statement
			{ $$ = $1;
			  if ($$->ndf.op == nop_stmt_callf)
			      $$->ndf.op = nop_top_callf; }
		| declarative_item
		| program_component
		;

/**** 2.1 ****/

/* handled by scanner */

/**** 2.3 ****/

identifier	: Identifier
			{ $$ = newnode(ident_node, nop_ident);
			  $$->nd.ident.id = $1.val;
			  cm($$,$1.attach,-1); }
		;

/**** 2.4 ****/

/**** 2.4.1 ****/

integer		: Integer
			{ $$ = newnode(integer_node, nop_int);
			  $$->nd.integer.val = $1.val;
			  $$->nd.integer.prval = $1.prval;
			  cm($$,$1.attach,-1); }
		;
real		: Real
			{ $$ = newnode(real_node, nop_real);
			  $$->nd.real.val = $1.val;
			  $$->nd.real.prval = $1.prval;
			  cm($$,$1.attach,-1); }
		;

/**** 2.4.2 ****/

/**** 2.5 ****/

character	: Character
			{ $$ = newnode(integer_node, nop_char);
			  $$->nd.integer.val = $1.val;
			  cm($$,$1.attach,-1); }
		;
/**** 2.6 ****/

string		: String
			{ $$ = newnode(ident_node, nop_string);
			  $$->nd.ident.id = $1.val;
			  cm($$,$1.attach,-1); }
		;

/**** 2.8 ****/

pragma		: PRAGMA Identifier ';'
			{ $$ = newnode(pragma_node, nop_pragma);
			  $$->nd.pragmax.id = $2.val;
			  $$->nd.pragmax.args = null; 
			  cm($$,$1.attach,$2.attach,$3.attach,-1); }
		| PRAGMA Identifier argument_list ';'
			{ $$ = newnode(pragma_node, nop_pragma);
			  $$->nd.pragmax.id = $2.val;
			  $$->nd.pragmax.args = $3; 
			  cm($$,$1.attach,$2.attach,$4.attach,-1); }
		;
/**** 3.1 ****/

declaration	: object_declaration
		| type_declaration
		| subprogram_declaration
		| task_declaration
		| renaming_declaration
		| number_declaration
		| subtype_declaration
		| package_declaration
		| exception_declaration
		;
/**** 3.2 ****/

object_declaration : identifier_list ':' subtype_indication
			initialization_optional ';'
			{ $$ = newnode(obj_decl_node, nop_obj_decl);
			  $$->nd.obj_decl.id_list = $1;
			  $$->nd.obj_decl.is_constant = false;
			  $$->nd.obj_decl.subtypex = $3;
			  $$->nd.obj_decl.initexp = $4;
			  cm($$,$2.attach,$5.attach,-1); }
		| identifier_list ':' array_type_definition
			initialization_optional ';'
			{ $$ = newnode(obj_decl_node, nop_obj_decl);
			  $$->nd.obj_decl.id_list = $1;
			  $$->nd.obj_decl.is_constant = false;
			  $$->nd.obj_decl.subtypex = $3;
			  $$->nd.obj_decl.initexp = $4;
			  cm($$,$2.attach,$5.attach,-1); }
		| identifier_list ':' CONSTANT subtype_indication
			initialization_optional ';'
			{ $$ = newnode(obj_decl_node, nop_obj_decl);
			  $$->nd.obj_decl.id_list = $1;
			  $$->nd.obj_decl.is_constant = true;
			  $$->nd.obj_decl.subtypex = $4;
			  $$->nd.obj_decl.initexp = $5;
			  cm($$,$2.attach,$3.attach,
				$6.attach,-1); }
		| identifier_list ':' CONSTANT array_type_definition
			initialization_optional ';'
			{ $$ = newnode(obj_decl_node, nop_obj_decl);
			  $$->nd.obj_decl.id_list = $1;
			  $$->nd.obj_decl.is_constant = true;
			  $$->nd.obj_decl.subtypex = $4;
			  $$->nd.obj_decl.initexp = $5;
			  cm($$,$2.attach,$3.attach,
				$6.attach,-1); }
		;
initialization_optional :
			{ $$ = null; }
		| ASSIGN_OP expression
			{ $$ = $2;
			  cm($$,$1.attach,-1); }
		;
number_declaration : identifier_list ':' CONSTANT ASSIGN_OP
			literal_expression ';'
			{ $$ = newnode(obj_decl_node, nop_obj_decl);
			  $$->nd.obj_decl.id_list = $1;
			  $$->nd.obj_decl.is_constant = true;
			  $$->nd.obj_decl.initexp = $5;
			  cm($$,$2.attach,$3.attach,
				$4.attach,$6.attach,-1); }
		;
identifier_list : ident_list_item
		| identifier_list ',' ident_list_item
			  { $$ = cmappend($1,$3,$2.attach); }
		;
ident_list_item : Identifier
			{ $$ = newnode(id_decl_node, nop_id_decl);
			  $$->nd.id_decl.id = $1.val;
			  cm($$,$1.attach,-1); }
		| ident_substitution_point		/* Castor */
		;
/**** 3.3.1 ****/

type_declaration : TYPE Identifier discriminant_part_optional IS
			type_definition ';'
			{ $$ = newnode(type_decl_node, nop_type);
			  $$->nd.type_decl.id = $2.val;
			  $$->nd.type_decl.disc = $3;
			  $$-> nd.type_decl.typex = $5;
			  cm($$,$1.attach,$2.attach,
				$4.attach,$6.attach,-1); }
		| incomplete_type_declaration
		;
discriminant_part_optional :
			{ $$ = null; }
		| discriminant_part
		;
type_definition : enumeration_type_definition
		| real_type_definition
		| record_type_definition
		| derived_type_definition
		| integer_type_definition
		| array_type_definition
		| access_type_definition
		| private_type_definition
		| type_calling_form		/* Castor */
		| type_substitution_point	/* Castor */
		;
/**** 3.3.2 ****/

subtype_declaration : SUBTYPE Identifier IS subtype_indication ';'
			{ $$ = newnode(type_decl_node, nop_subtype);
			  $$->nd.type_decl.id = $2.val;
			  $$->nd.type_decl.typex = $4;
			  cm($$,$1.attach,$2.attach,
				$3.attach,$5.attach,-1); }
		;
subtype_indication : name
		| name range_constraint
			{ $$ = $2;
			  $$->nd.rangex.type_id = $1; }
		| name accuracy_constraint
			{ $$ = $2;
			  $$->nd.rangex.type_id = $1; }
		| subtype_substitution_point		/* Castor */
		;
/**** 3.4 ****/

derived_type_definition : NEW subtype_indication
			{ $$ = newnode(derived_type_node, nop_derived_type); 
			  $$->nd.derived_type.typex = $2;
			  cm($$,$1.attach,-1); }
		;
/**** 3.5 ****/

range_constraint : RANGE range
			{ $$ = $2; 
			  $$->nd.rangex.is_range_constraint = true;
			  cm($$,$1.attach,-1); }
		;
range		: simple_expression SUBRANGE_SYM simple_expression
			{ $$ = newnode(range_node, nop_range);
			  $$->nd.rangex.left = $1;
			  $$->nd.rangex.right = $3; 
			  cm($$,$2.attach,-1); }
		;
/**** 3.5.1 ****/

enumeration_type_definition : '(' enumeration_literal_comma_list ')'
			{ $$ = newnode(enumtype_node, nop_enum);
			  $$->nd.enumtype.ids = $2; 
			  cm($$,$1.attach,$3.attach,-1); }
		;
enumeration_literal_comma_list : enumeration_literal
		| enumeration_literal_comma_list ',' enumeration_literal
			{ $$ = cmappend($1,$3,$2.attach);  }
		;
enumeration_literal : Identifier
			{ $$ = newnode(id_decl_node, nop_id_decl);
			  $$->nd.id_decl.id = $1.val; 
			  cm($$,$1.attach,-1); }
		| Character
			{ $$ = newnode(char_enum_node, nop_char_enum);
			  $$->nd.char_enum.ch = $1.val;
			  cm($$,$1.attach,-1); }
		;
/**** 3.5.4 ****/

integer_type_definition : range_constraint
		;
/**** 3.5.6 ****/

real_type_definition : accuracy_constraint
		;
/**** 3.5.7 ****/

accuracy_constraint : floating_point_constraint
		| fixed_point_constraint
		;
floating_point_constraint : DIGITS static_simple_expression
			{ $$ = newnode(range_node, nop_digits);
			  $$->nd.rangex.accuracy_expr = $2;
			  cm($$,$1.attach,-1); }
		| DIGITS static_simple_expression range_constraint
			{ $$ = $3;
			  $$->ndf.op = nop_digits;
			  $$->nd.rangex.accuracy_expr = $2;
			  cm($$,$1.attach,-1); }
		;
/**** 3.5.9 ****/

fixed_point_constraint : DELTA static_simple_expression
			{ $$ = newnode(range_node, nop_delta);
			  $$->nd.rangex.accuracy_expr = $2;
			  cm($$,$1.attach,-1); }
		| DELTA static_simple_expression range_constraint
			{ $$ = $3;
			  $$->ndf.op = nop_delta;
			  $$->nd.rangex.accuracy_expr = $2;
			  cm($$,$1.attach,-1); }
		;
/**** 3.6 ****/

array_type_definition : ARRAY '(' index_comma_list ')' OF
			component_subtype_indication
			{ $$ = newnode(array_node, nop_array);
			  $$->nd.arrayx.index_list = $3;
			  $$->nd.arrayx.ele_type = $6;	 
			  cm($$,$1.attach,$2.attach,$4.attach,
				$5.attach,-1); }
		| ARRAY index_constraint OF component_subtype_indication
			{ $$ = newnode(array_node, nop_array);
			  $$->nd.arrayx.index_list = $2;
			  $$->nd.arrayx.ele_type = $4;
			  cm($$,$1.attach,$3.attach,-1); }
		;
index_comma_list : index
			{ $$ = newnode(index_node, nop_index);
			  $$->nd.index.typex = $1; }
		| index_comma_list ',' index
			{ nodep p;
			  p = newnode(index_node, nop_index);
			  p->nd.index.typex = $3;
			  $$ = cmappend($1,p,$2.attach);  }
		;
index		: name RANGE BOX
			{ $$ = newnode(range_node, nop_range);
			  $$->nd.rangex.type_id = $1;
			  $$->nd.rangex.not_constrained = true; 
			  $$->nd.rangex.is_range_constraint = true;
			  cm($$,$2.attach,$3.attach,-1); }
		;
index_constraint : '(' discrete_range_list ')'
			{ $$ = $2; 
			  cm($$,$1.attach,$3.attach,-1); }
		;
discrete_range_list : discrete_range
			{ $$ = newnode(index_node, nop_index);
			  $$->nd.index.typex = $1; }
		| discrete_range_list ',' discrete_range
			{ nodep p;
			  p = newnode(index_node, nop_index);
			  p->nd.index.typex = $3;
			  $$ = cmappend($1,p,$2.attach);  }
		;
discrete_range	: name
		| name range_constraint
			{ $$ = $2;
			  $$->nd.rangex.type_id = $1;
			  $$->nd.rangex.not_constrained = false; }
		| range
		;
/**** 3.7 ****/

record_type_definition : RECORD component_list END RECORD
			{ $$ = newnode(record_node, nop_record);
			  $$->nd.recordx.field_list = $2; 
			  cm($$,$1.attach,$3.attach,
				$4.attach,-1); }
		;
component_list	: component_declaration_null_list
		| component_declaration_null_list variant_part
			{ $$ = append($1,$2); }
		| XNULL ';'
			{ $$ = null; } 	/* what about attachments ?? */
		;
component_declaration_null_list :
			{ $$ = null; }
		| component_declaration_null_list component_declaration
			{ $$ = append($1,$2); }
		;
component_declaration  : identifier_list ':' subtype_indication
			initialization_optional ';'
			{ $$ = newnode(obj_decl_node, nop_obj_decl);
			  $$->nd.obj_decl.id_list = $1;
			  $$->nd.obj_decl.is_constant = false;
			  $$->nd.obj_decl.subtypex = $3;
			  $$->nd.obj_decl.initexp = $4; 
			  cm($$,$2.attach,$5.attach,-1); }
		| identifier_list ':' array_type_definition
			initialization_optional ';'
			{ $$ = newnode(obj_decl_node, nop_obj_decl);
			  $$->nd.obj_decl.id_list = $1;
			  $$->nd.obj_decl.is_constant = false;
			  $$->nd.obj_decl.subtypex = $3;
			  $$->nd.obj_decl.initexp = $4; 
			  cm($$,$2.attach,$5.attach,-1); }
		;
/**** 3.7.1 ****/

discriminant_part : '(' discriminant_declaration_semicolon_list ')'
			{ $$ = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
discriminant_declaration_semicolon_list : discriminant_declaration
		| discriminant_declaration_semicolon_list ';'
			discriminant_declaration
			{ $$ = append($1,$3,$2.attach);  }
		;
discriminant_declaration : identifier_list ':' subtype_indication
			initialization_optional
			{ $$ = newnode(obj_decl_node, nop_discriminant);
			  $$->nd.obj_decl.id_list = $1;
			  $$->nd.obj_decl.is_constant = false;
			  $$->nd.obj_decl.subtypex = $3;
			  $$->nd.obj_decl.initexp = $4; 
			  cm($$,$2.attach,-1); }
		;
/**** 3.7.3 ****/

variant_part	: CASE name IS variant_element_null_list END CASE ';'
			{ $$ = newnode(case_node, nop_rec_var);
			  $$->nd.casex.expr = $2;
			  $$->nd.casex.when_list = $4;
			  cm($$,$1.attach,$3.attach,$5.attach,
				$6.attach,$7.attach, -1); }
		;
variant_element_null_list :
			{ $$ = null; }
		| variant_element_null_list WHEN choice_stroke_list 
			RIGHT_ARROW component_list
			{ nodep p;
			  p = newnode(when_node, nop_rec_when);
			  p->nd.whenx.choice_list = $3;
			  p->nd.whenx.stmt = $5;
			  $$ = append($1,p);
			  cm(p,$2.attach,$4.attach,-1); }
		;
choice_stroke_list : choice
			{ $$ = newnode(choice_node, nop_choice);
			  $$->nd.choice.choice = $1; }
		| choice_stroke_list '|' choice
			{ nodep p;
			  p = newnode(choice_node, nop_choice);
			  p->nd.choice.choice = $3; 
			  $$ = append($1,p,$2.attach);  }
		;
choice		: simple_expression
		| name range_constraint
			{ $$ = $2;
			  $$->nd.rangex.type_id = $1;
			  $$->nd.rangex.not_constrained = false; }
		| range
		| OTHERS
			{ $$ = null; }		/* attachments? */
		;
/**** 3.8 ****/

access_type_definition : ACCESS subtype_indication
			{ $$ = newnode(access_node, nop_access);
			  $$->nd.accessx.target_type = $2; 
			  cm($$,$1.attach,-1); }
		;
/**** 3.8.1 ****/

incomplete_type_declaration  : TYPE Identifier ';'
			{ $$ = newnode(type_decl_node, nop_type);
			  $$->nd.type_decl.id = $2.val;
			  cm($$,$1.attach,$2.attach,
				$3.attach,-1); }
		| TYPE Identifier discriminant_part ';'
			{ $$ = newnode(type_decl_node, nop_type);
			  $$->nd.type_decl.id = $2.val;
			  $$->nd.type_decl.disc = $3;
			  cm($$,$1.attach,$2.attach,
				$4.attach,-1); }
		;
/**** 3.9 ****/

declarative_part :
			{ $$ = null; }
		| declarative_part declarative_item
			{ $$ = append($1,$2); }
		| declarative_part representation_specification
			{ $$ = append($1,$2); }
		| declarative_part program_component
			{ $$ = append($1,$2); }
		| declarative_part pragma
			{ $$ = append($1,$2); }
		| declarative_part castor_declarative_item
			{ $$ = append($1,$2); }
		;
declarative_item : declaration
		| use_clause
		;
program_component : body
		| body_stub
		;
body		: subprogram_body
		| package_body
		| task_body
		| macro_body				/* Castor */
		;
/**** 4.1 ****/

name		: name1
			{ $$ = $1;
			  if ($$->ndf.op == nop_oper_string)
				$$->ndf.op = nop_string;
			}
		;
name1		: identifier
		| name1 '(' ')'	  /***	not legal ADA ***/
			{ $$ = newnode(call_node, nop_call);
			  $$->nd.call.var = $1;
			  $$->nd.call.args = null;
			  cm($$,$2.attach,$3.attach,-1); }
		| name1 argument_list
			{ $$ = newnode(call_node, nop_call);
			  $$->nd.call.var = $1;
			  $$->nd.call.args = $2; }
		| selected_component
		| attribute
		| operator_symbol
		| name_calling_form		/* Castor */
		| name_substitution_point	/* Castor */
		;
/**** 4.1.1 ****/

/**** 4.1.2 ****/

/**** 4.1.3 ****/

selected_component : name1 '.' Identifier
			{ $$ = newnode(select_node, nop_dot);
			  $$->nd.selectx.var = $1;
			  $$->nd.selectx.field = $3.val;
			  $$->nd.selectx.is_oper = false;
			  cm($$,$2.attach,$3.attach,-1); }
		| name1 '.' ALL
			{ $$ = newnode(select_node, nop_dot);
			  $$->nd.selectx.var = $1;
			  cm($$,$2.attach,$3.attach,-1); }
		| name1 '.' Operator_String
			{ $$ = newnode(select_node, nop_dot);
			  $$->nd.selectx.var = $1;
			  $$->nd.selectx.field = $3.val;
			  $$->nd.selectx.is_oper = true;
			  cm($$,$2.attach,$3.attach,-1); }
		;
/**** 4.1.4 ****/

attribute	: name1 '\'' Identifier
			{ $$ = newnode(select_node, nop_quote);
			  $$->nd.selectx.var = $1;
			  $$->nd.selectx.field = $3.val;
			  cm($$,$2.attach,$3.attach,-1); }
		| name1 '\'' DELTA
			{ $$ = newnode(select_node, nop_quote);
			  $$->nd.selectx.var = $1;
			  $$->nd.selectx.field = intern("delta");
			  cm($$,$2.attach,$3.attach,-1); }
		| name1 '\'' DIGITS
			{ $$ = newnode(select_node, nop_quote);
			  $$->nd.selectx.var = $1;
			  $$->nd.selectx.field = intern("digits");
			  cm($$,$2.attach,$3.attach,-1); }
		| name1 '\'' RANGE
			{ $$ = newnode(select_node, nop_quote);
			  $$->nd.selectx.var = $1;
			  $$->nd.selectx.field = intern("range");
			  cm($$,$2.attach,$3.attach,-1); }
		;
/**** 4.3 ****/

aggregate	: '(' component_association_comma_list ')'
			{ $$ = newnode(aggregate_node, nop_aggregate); 
			  $$->nd.aggregate.agg = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
component_association_comma_list : component_association
		| component_association_comma_list ',' component_association
			{ $$ = cmappend($1,$3,$2.attach);  }
		;
component_association : expression
			{ $$ = newnode(agg_comp_node, nop_agg_comp);
			  $$->nd.agg_comp.expr = $1; }
		| choice_stroke_list RIGHT_ARROW expression
			{ $$ = newnode(agg_comp_node, nop_agg_comp);
			  $$->nd.agg_comp.choice = $1;
			  $$->nd.agg_comp.expr = $3;
			  cm($$,$2.attach,-1); }
		;
/**** 4.4 ****/

expression	: relation
		| relation_and_list
		| relation_or_list
		| relation_xor_list
		| relation_and_then_list
		| relation_or_else_list
		;
relation_and_list : relation AND relation
			{ $$ = newnode(binop_node, nop_and);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3; 
			  cm($$,$2.attach,-1); }
		| relation_and_list AND relation
			{ $$ = newnode(binop_node, nop_and);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3;
			  cm($$,$2.attach,-1); }
		;
relation_or_list : relation OR relation
			{ $$ = newnode(binop_node, nop_or);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3;
			  cm($$,$2.attach,-1); }
		| relation_or_list OR relation
			{ $$ = newnode(binop_node, nop_or);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3;
			  cm($$,$2.attach,-1); }
		;
relation_xor_list : relation XOR relation
			{ $$ = newnode(binop_node, nop_xor);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3; 
			  cm($$,$2.attach,-1); }
		| relation_or_list XOR relation
			{ $$ = newnode(binop_node, nop_xor);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3;
			  cm($$,$2.attach,-1); }
		;
relation_and_then_list : relation AND THEN relation
			{ $$ = newnode(binop_node, nop_andthen);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $4;
			  cm($$,$2.attach,$3.attach,-1); }
		| relation_and_then_list AND THEN relation
			{ $$ = newnode(binop_node, nop_andthen);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $4;
			  cm($$,$2.attach,$3.attach,-1); }
		;
relation_or_else_list : relation OR ELSE relation
			{ $$ = newnode(binop_node, nop_orelse);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $4;
			  cm($$,$2.attach,$3.attach,-1); }
		| relation_or_else_list OR ELSE relation
			{ $$ = newnode(binop_node, nop_orelse);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $4;
			  cm($$,$2.attach,$3.attach,-1); }
		;
relation	: simple_expression_relational_list
		| simple_expression IN subtype_indication
			{ $$ = newnode(inop_node, nop_in);
			  $$->nd.inop.expr = $1;
			  $$->nd.inop.not_in = false;
			  $$->nd.inop.rangex = $3;
			  cm($$,$2.attach,-1); }
		| simple_expression IN range
			{ $$ = newnode(inop_node, nop_in);
			  $$->nd.inop.expr = $1;
			  $$->nd.inop.not_in = false;
			  $$->nd.inop.rangex = $3;
			  cm($$,$2.attach,-1); }
		| simple_expression NOT IN subtype_indication
			{ $$ = newnode(inop_node, nop_in);
			  $$->nd.inop.expr = $1;
			  $$->nd.inop.not_in = true;
			  $$->nd.inop.rangex = $4;
			  cm($$,$2.attach,$3.attach,-1); }
		| simple_expression NOT IN range
			{ $$ = newnode(inop_node, nop_in);
			  $$->nd.inop.expr = $1;
			  $$->nd.inop.not_in = true;
			  $$->nd.inop.rangex = $4;
			  cm($$,$2.attach,$3.attach,-1); }
		;
simple_expression_relational_list : simple_expression
		| simple_expression_relational_list relational_operator
			simple_expression
			{ $$ = newnode(binop_node, $2.token);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3;
			  cm($$,$2.attach,-1); }
		;
simple_expression : term_adding_operator_list
		;
term_adding_operator_list : term
		| unary_operator term
			{ $$ = newnode(uop_node, $1.token);
			  $$->nd.uop.child = $2;
			  cm($$,$1.attach,-1); }
		| term_adding_operator_list adding_operator term
			{ $$ = newnode(binop_node, $2.token);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3;
			  cm($$,$2.attach,-1); }
		;
term		: factor_multiplying_operator_list
		;
factor_multiplying_operator_list : factor
		| factor_multiplying_operator_list multiplying_operator factor
			{ $$ = newnode(binop_node, $2.token);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3;
			  cm($$,$2.attach,-1); }
		;
factor		: primary
		| primary EXPONENTIAL primary
			{ $$ = newnode(binop_node, nop_expon);
			  $$->nd.binop.left = $1;
			  $$->nd.binop.right = $3;
			  cm($$,$2.attach,-1); }
		;
primary		: literal
		| '(' expression ')'
			{ $$ = newnode(uop_node, nop_lp);
			  $$->nd.uop.child = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		| aggregate
		| name
			{ $$ = $1;
			  if ($$->ndf.op == nop_name_callf)
				$$->ndf.op = nop_expr_callf;
			  else if ($$->ndf.op == nop_name_subst)
				$$->ndf.op = nop_expr_subst; }
		| allocator
		| qualified_expression
		;
/**** 4.5 ****/

relational_operator : '='
			{ $$.token = nop_equal; $$.attach = $1.attach; }
		| NEQ
			{ $$.token = nop_nequal; $$.attach = $1.attach; }
		| '<'
			{ $$.token = nop_lt; $$.attach = $1.attach; }
		| LEQ
			{ $$.token = nop_le; $$.attach = $1.attach; }
		| '>'
			{ $$.token = nop_gt; $$.attach = $1.attach; }
		| GEQ
			{ $$.token = nop_ge; $$.attach = $1.attach; }
		;
adding_operator : '+'
			{ $$.token = nop_plus; $$.attach = $1.attach; }
		| '-'
			{ $$.token = nop_minus; $$.attach = $1.attach; }
		| '&'
			{ $$.token = nop_amper; $$.attach = $1.attach; }
		;
unary_operator	: '+'
			{ $$.token = nop_pos; $$.attach = $1.attach; }
		| '-'
			{ $$.token = nop_neg; $$.attach = $1.attach; }
		| NOT
			{ $$.token = nop_not; $$.attach = $1.attach; }
		;
multiplying_operator : '*'
			{ $$.token = nop_star; $$.attach = $1.attach; }
		| '/'
			{ $$.token = nop_div; $$.attach = $1.attach; }
		| MOD
			{ $$.token = nop_mod; $$.attach = $1.attach; }
		| REM
			{ $$.token = nop_rem; $$.attach = $1.attach; }
		;
/**** 4.6 ****/

/**** 4.7 ****/

qualified_expression : name1 '\'' '(' expression ')'
			{ $$ = newnode(aggregate_node, nop_qual_expr);
			  $$->nd.aggregate.qualifier = $1;
			  $$->nd.aggregate.agg = $4;
			  cm($$,$2.attach,$3.attach,$5.attach,-1); }
		| name1 '\'' aggregate
			{ $$ = $3;
			  $$->nd.aggregate.qualifier = $1;
			  cm($$,$2.attach,-1); }
		;
/**** 4.8 ****/

allocator	: NEW name
			{ $$ = newnode(new_node, nop_new);
			  $$->nd.newx.type_id = $2;
			  cm($$,$1.attach,-1); }
		;
/**** 5.1 ****/

sequence_of_statements : statement
		| sequence_of_statements statement
			{ $$ = append($1,$2); }
		;

statement	: simple_statement
		| compound_statement
		| pragma
		| label_list simple_statement
			{ $$ = append($1,$2); }
		| label_list compound_statement
			{ $$ = append($1,$2); }
		;
top_level_statement : simple_statement
		| unlabeled_compound_statement
		| pragma
		;
label_list	: label
		| label_list label
			{ $$ = append($1,$2); }
		;
    /*  Castor statement here comes here first because of a reduce-reduce
	conflict.  The syntax of a procedure call is simply "name;".  A
	calling form in a statement list should generate a statement
	calling form, not a name calling form.  A similar problem occurs
	with name and statement substitution points.  Putting 
	castor_statement first causes yacc to resolve these conflicts
	correctly. */

simple_statement : castor_statement		/* Castor */
		| null_statement
		| assignment_statement
		| return_statement
		| procedure_or_entry_call
		| delay_statement
		| raise_statement
		| exit_statement
		| goto_statement
		| abort_statement
		| code_statement
		;
compound_statement : if_statement
		| loop_statement
		| accept_statement
		| case_statement
		| block
		| select_statement
		;
unlabeled_compound_statement : if_statement
		| unlabeled_loop_statement
		| accept_statement
		| case_statement
		| unlabeled_block
		| select_statement
		;
label		: LEFT_LABEL_BRACKET Identifier RIGHT_LABEL_BRACKET
			{ $$ = newnode(ident_node, nop_lb); 
			  $$->nd.ident.id = $2.val;
			  cm($$,$1.attach,$2.attach,$3.attach,-1); }
		;
null_statement : XNULL ';'
			{ $$ = newnode(simple_node, nop_null_stmt);
			  cm($$,$1.attach,$2.attach,-1); }
		;
/**** 5.2 ****/

assignment_statement : name ASSIGN_OP expression ';'
			{ $$ = newnode(asgn_stmt_node, nop_asgn);
			  $$->nd.asgn_stmt.lhs = $1;
			  $$->nd.asgn_stmt.rhs = $3;
			  cm($$,$2.attach,$4.attach,-1); }
		;
/**** 5.3 ****/

if_statement	: IF condition THEN sequence_of_statements elsif_list
			else_optional END IF ';'
			{ $$ = newnode(if_node, nop_if);
			  $$->nd.elsifx.expr = $2;
			  $$->nd.elsifx.then_part = $4;
			  $$->nd.ifx.elsif_list = append($5,$6); 
			  cm($$,$1.attach,$3.attach,$7.attach,
				$8.attach,$9.attach,-1); }
		;
elsif_list	:
			{ $$ = null; }
		| elsif_list ELSIF condition THEN sequence_of_statements
			{ nodep p;
			  p = newnode(elsif_node, nop_elsif);
			  p->nd.elsifx.expr = $3;
			  p->nd.elsifx.then_part = $5;
			  $$ = append($1,p); 
			  cm(p,$2.attach,$4.attach,-1); }
		;
else_optional	:
			{ $$ = null; }
		| ELSE sequence_of_statements
			{ $$ = newnode(elsif_node, nop_elsif);
			  $$->nd.elsifx.then_part = $2;
			  cm($$,$1.attach,-1); }
		;
condition	: boolean_expression
		;
/**** 5.4 ****/

case_statement	: CASE expression IS when_null_list END CASE ';'
			{ $$ = newnode(case_node, nop_case);
			  $$->nd.casex.expr = $2;
			  $$->nd.casex.when_list = $4;
			  cm($$,$1.attach,$3.attach,$5.attach,
				$5.attach,$6.attach,$7.attach,
				-1); }
		;
when_null_list	:
			{ $$ = null; }
		| when_null_list WHEN choice_stroke_list
			RIGHT_ARROW sequence_of_statements
			{ nodep p;
			  p = newnode(when_node, nop_when);
			  p->nd.whenx.choice_list = $3;
			  p->nd.whenx.stmt = $5;
			  $$ = append($1,p);
			  cm(p,$2.attach,$4.attach,-1); }
		;
/**** 5.5 ****/

loop_statement	: unlabeled_loop_statement
		| Identifier ':' basic_loop Identifier ';'
			{ idcheck($1.val,$4.val,"LOOP");
			  $$ = newnode(while_node, nop_loop);
			  $$->nd.whilex.label = $1.val;
			  $$->nd.whilex.bodyx = $3;
			  cm($$,$1.attach,$2.attach,$4.attach,$5.attach,-1); }
		| Identifier ':' iteration_clause 
		  	basic_loop Identifier ';'
			{ idcheck($1.val,$5.val,"LOOP");
			  $$ = $3;
			  if ($$->ndf.op == nop_while) {
				$$->nd.whilex.bodyx = $4;
				$$->nd.whilex.label = $1.val;
			  }
			  else {
				$$->nd.forx.bodyx = $4; 
				$$->nd.forx.label = $1.val;
			  }
			  cm($$,$1.attach,$2.attach,$5.attach,$6.attach,-1); }
		;
unlabeled_loop_statement : basic_loop ';'
			{ $$ = newnode(while_node, nop_loop);
			  $$->nd.whilex.bodyx = $1; 
			  cm($$,$2.attach,-1); }
		| iteration_clause basic_loop ';'
			{ $$ = $1;
			  if ($$->ndf.op == nop_while)
				$$->nd.whilex.bodyx = $2;
			  else
				$$->nd.forx.bodyx = $2; 
			  cm($$,$3.attach,-1); }
		;
basic_loop	: LOOP sequence_of_statements END LOOP
			{ $$ = $2; 
			  cm($$,$1.attach,$3.attach,
				$4.attach,-1); }
		;
iteration_clause : FOR loop_parameter IN discrete_range
			{ $$ = newnode(for_node, nop_for);
			  $$->nd.forx.cntrl_var = $2.val;
			  $$->nd.forx.reversex = false;
			  $$->nd.forx.lims = $4;
			  cm($$,$1.attach,$3.attach,-1); }
		| FOR loop_parameter IN REVERSE discrete_range
			{ $$ = newnode(for_node, nop_for);
			  $$->nd.forx.cntrl_var = $2.val;
			  $$->nd.forx.reversex = true;
			  $$->nd.forx.lims = $5;
			  cm($$,$1.attach,$3.attach,
				$4.attach,-1); }
		| WHILE condition
			{ $$ = newnode(while_node, nop_while);
			  $$->nd.whilex.expr = $2;
			  cm($$,$1.attach,-1); }
		;
loop_parameter	: Identifier
		;
/**** 5.6 ****/

block		: unlabeled_block
		| Identifier ':' XBEGIN sequence_of_statements
			exceptions_optional END Identifier ';'
			{ idcheck($1.val,$7.val,"BLOCK");
			  $$ = newnode(subp_node, nop_declare);
			  $$->nd.subp.name = $1.val;
			  $$->nd.subp.is_oper = false;
			  $$->nd.subp.bodyx = $4;
			  $$->nd.subp.except = $5;
			  cm($$,$1.attach,$2.attach,$3.attach,$6.attach,
				$7.attach,$8.attach,-1); }
		| Identifier ':' DECLARE declarative_part
			XBEGIN sequence_of_statements exceptions_optional
			END Identifier ';'
			{ idcheck($1.val, $9.val, "BLOCK");
  			  $$ = newnode(subp_node, nop_declare);
			  $$->nd.subp.name = $1.val;
			  $$->nd.subp.is_oper = false;
			  $$->nd.subp.decl = $4;
			  $$->nd.subp.bodyx = $6;
			  $$->nd.subp.except = $7;
			  cm($$,$1.attach,$2.attach,$3.attach,$5.attach,
				$8.attach,$9.attach,$10.attach,-1); }
		;
unlabeled_block	: XBEGIN sequence_of_statements exceptions_optional END ';'
			{ $$ = newnode(subp_node, nop_declare);
			  $$->nd.subp.is_oper = false;
			  $$->nd.subp.bodyx = $2;
			  $$->nd.subp.except = $3;
			  cm($$,$1.attach,$4.attach,$5.attach,
				-1); }
		| DECLARE declarative_part XBEGIN sequence_of_statements
			exceptions_optional END ';'
			{ $$ = newnode(subp_node, nop_declare);
			  $$->nd.subp.is_oper = false;
			  $$->nd.subp.decl = $2;
			  $$->nd.subp.bodyx = $4;
			  $$->nd.subp.except = $5; 
			  cm($$,$1.attach,$3.attach,
				$6.attach,$7.attach,-1); }
		;
/**** 5.7 ****/

exit_statement	: EXIT ';'
			{ $$ = newnode(exit_node, nop_exit);
			  cm($$,$1.attach,$2.attach,-1); }
		| EXIT loop_name ';'
			{ $$ = newnode(exit_node, nop_exit);
			  $$->nd.exitx.loop_name = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		| EXIT WHEN condition ';'
			{ $$ = newnode(exit_node, nop_exit);
			  $$->nd.exitx.cond = $3;
			  cm($$,$1.attach,$2.attach,$4.attach,
				-1); }
		| EXIT loop_name WHEN condition ';'
			{ $$ = newnode(exit_node, nop_exit);
			  $$->nd.exitx.loop_name = $2;
			  $$->nd.exitx.cond = $4;
			  cm($$,$1.attach,$3.attach,$5.attach,
				-1); }
		;
/**** 5.8 ****/

return_statement : RETURN ';'
			{ $$ = newnode(return_node, nop_return);
			  $$->nd.returnx.val = null; 
			  cm($$,$1.attach,$2.attach,-1); }
		| RETURN expression ';'
			{ $$ = newnode(return_node, nop_return);
			  $$->nd.returnx.val = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
/**** 5.9 ****/

goto_statement : GOTO label_name ';'
			{ $$ = newnode(goto_node, nop_goto);
			  $$->nd.gotox.label = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
/**** 6.1 ****/

subprogram_declaration : subprogram_specification ';'
			{ $$ = $1;
			  cm($$,$2.attach,-1); }
		| macro_specification ';'		/* Castor */
			{ $$ = $1;
			  cm($$,$2.attach,-1); }
		| generic_subprogram_declaration
		| generic_subprogram_instantiation
		;
subprogram_specification : PROCEDURE Identifier
			{ $$ = newnode(subp_node, nop_procedure);
			  $$->nd.subp.name = $2.val; 
			  $$->nd.subp.is_oper = false;
			  $$->nd.subp.is_separate = false;
			  cm($$,$1.attach,$2.attach,-1); }
		| PROCEDURE Identifier formal_part
			{ $$ = newnode(subp_node, nop_procedure);
			  $$->nd.subp.name = $2.val;
			  $$->nd.subp.is_oper = false;
			  $$->nd.subp.is_separate = false;
			  $$->nd.subp.spec = $3;
			  cm($$,$1.attach,$2.attach,-1); }
		| FUNCTION designator RETURN subtype_indication
			{ $$ = newnode(subp_node, nop_function);
			  $$->nd.subp.name = $2.val;
			  $$->nd.subp.is_oper = $2.flag;
			  $$->nd.subp.is_separate = false;
			  $$->nd.subp.spec = null;
			  $$->nd.subp.ret_type = $4; 
			  cm($$,$1.attach,$3.attach,-1);  }
		| FUNCTION designator formal_part RETURN subtype_indication
			{ $$ = newnode(subp_node, nop_function);
			  $$->nd.subp.name = $2.val;
			  $$->nd.subp.is_oper = $2.flag;
			  $$->nd.subp.is_separate = false;
			  $$->nd.subp.spec = $3;
			  $$->nd.subp.ret_type = $5;
			  cm($$,$1.attach,$4.attach,-1);  }
		;
designator	: Identifier
			{ $$.val = $1.val;
			  $$.flag = false;
			  $$.attach = $1.attach; }
		| Operator_String
			{ $$.val = $1.val;
			  $$.flag = true;
			  $$.attach = $1.attach; }
		;
operator_symbol : Operator_String
			{ $$ = newnode(ident_node, nop_oper_string);
			  $$->nd.ident.id = $1.val;
			  cm($$,$1.attach,-1); }
		;
formal_part	: '(' parameter_declaration_semicolon_list ')'
			{ $$ = $2; 
			  cm($$,$1.attach,$3.attach,-1); }
		;
parameter_declaration_semicolon_list : parameter_declaration
		| parameter_declaration_semicolon_list ';'
			parameter_declaration
			{ $$ = cmappend($1,$3,$2.attach); }
		;
parameter_declaration : identifier_list ':' mode subtype_indication
			{ $$ = newnode(param_decl_node, nop_param);
			  $$->nd.param_decl.id_list = $1;
			  $$->nd.param_decl.is_in = $3.in;
			  $$->nd.param_decl.is_out = $3.out;
			  $$->nd.param_decl.subtyp = $4;
			  cm($$,$2.attach,-1); }
		| identifier_list ':' mode subtype_indication
			ASSIGN_OP expression
			{ $$ = newnode(param_decl_node, nop_param);
			  $$->nd.param_decl.id_list = $1;
			  $$->nd.param_decl.is_in = $3.in;
			  $$->nd.param_decl.is_out = $3.out;
			  $$->nd.param_decl.subtyp = $4;
			  $$->nd.param_decl.initexp = $6;
			  cm($$,$2.attach,$5.attach,-1); }
		;
mode		:
			{ $$.in = true;
			  $$.out = false;
			  $$.attach = null; }
		| IN
			{ $$.in = true;
			  $$.out = false;
			  $$.attach = $1.attach; }
		| OUT
			{ $$.in = false;
			  $$.out = true;
			  $$.attach = $1.attach; }
		| IN OUT
			{ $$.in = true;
			  $$.out = true;
			  $$.attach = append($1.attach,$2.attach); }
		;
/**** 6.3 ****/

subprogram_body : subprogram_specification IS declarative_part
			XBEGIN sequence_of_statements exceptions_optional
			END designator_optional ';'
			{ $$ = $1;
			  $$->nd.subp.decl = $3;
			  $$->nd.subp.bodyx = $5; 
			  $$->nd.subp.except = $6; 
			  cm($$,$2.attach,$4.attach,$7.attach,
			     $8.attach,$9.attach,-1); }
		;
exceptions_optional :
			{ $$ = null; }
		| EXCEPTION exception_handler_list
			{ $$ = $2; 
			  cm($$,$1.attach,-1); }
		;
exception_handler_list : exception_handler
		| exception_handler_list exception_handler
			{ $$ = append($1,$2); }
		;
designator_optional :
			{ $$.val = null;
			  $$.attach = null; }
		| designator
		;
/**** 6.4 ****/

procedure_or_entry_call : name ';'
			{ if ($1->ndf.op == nop_call) {
				$$ = $1;
				$$->ndf.op = nop_proc_call;
			  }
				/* BIG KLUDGE until I understand
				   how to make yacc do this right */
			  else 	if ($1->ndf.op == nop_name_callf) {
				$$ = $1;
				$$->ndf.op = nop_stmt_callf;
			  }
			  else 	if ($1->ndf.op == nop_name_subst) {
				$$ = $1;
				$$->ndf.op = nop_stmt_subst;
			  }
			  else {
				$$ = newnode(call_node, nop_proc_call);
				$$->nd.call.var = $1;
				$$->nd.call.args = null;
			  }
			  cm($$,$2.attach,-1); }
		;
/**** 7.1 ****/

package_declaration : package_specification ';'
			{ $$ = $1;
			  cm($$,$2.attach,-1); }
		| generic_package_declaration
		| generic_package_instantiation
		;
package_specification : PACKAGE Identifier IS declarative_item_null_list
			private_part_optional END
			{ $$ = newnode(package_node, nop_package);
			  $$->nd.packagex.name = $2.val;
			  $$->nd.packagex.decl = $4; 
			  $$->nd.packagex.priv_decl = $5; 
			  $$->nd.packagex.is_separate = false;
			  cm($$,$1.attach,$2.attach,$3.attach,
				$6.attach,-1); }
		| PACKAGE Identifier IS declarative_item_null_list
			private_part_optional END Identifier
			{ $$ = newnode(package_node, nop_package);
			  $$->nd.packagex.name = $2.val;
			  $$->nd.packagex.decl = $4; 
			  $$->nd.packagex.priv_decl = $5; 
			  $$->nd.packagex.is_separate = false;
			  idcheck($2.val,$7.val,"PACKAGE");
			  cm($$,$1.attach,$2.attach,$3.attach,
				$6.attach,$7.attach,-1); }
		;
package_body	: PACKAGE BODY Identifier IS declarative_part
			package_body_part_optional END ';'
			{ $$ = newnode(package_node, nop_body);
			  $$->ndf.op = nop_body;
			  $$->nd.packagex.name = $3.val;
			  $$->nd.packagex.decl = $5;
			  $$->nd.packagex.bodyx = $6.p1;
			  $$->nd.packagex.except = $6.p2;
			  $$->nd.packagex.is_separate = false;
			  cm($$,$1.attach,$2.attach,$3.attach,
				$4.attach,$7.attach,$8.attach,-1); }
		| PACKAGE BODY Identifier IS declarative_part
			package_body_part_optional END Identifier ';'
			{ $$ = newnode(package_node, nop_body);
			  $$->ndf.op = nop_body;
			  $$->nd.packagex.name = $3.val;
			  $$->nd.packagex.decl = $5;
			  $$->nd.packagex.bodyx = $6.p1;
			  $$->nd.packagex.except = $6.p2;
			  $$->nd.packagex.is_separate = false;
			  idcheck($3.val,$8.val,"PACKAGE BODY");
			  cm($$,$1.attach,$2.attach,$3.attach,
				$4.attach,$7.attach,$8.attach,
				$9.attach,-1); }
		;
declarative_item_null_list :
			{ $$ = null; }
		| declarative_item_null_list declarative_item
			{ $$ = append($1,$2); }
		;
private_part_optional :
			{ $$ = null; }
		| PRIVATE declarative_item_null_list declarative_item
			{ $$ = append($2,$3);
			  cm($3,$1.attach,-1); }
		;
representation_specification_null_list :
			{ $$ = null; }
		| representation_specification_null_list
			representation_specification
			{ $$ = append($1,$2); }
		| representation_specification_null_list pragma
			{ $$ = append($1,$2); }
		;
package_body_part_optional :
			{ $$.p1 = null; }
		| XBEGIN sequence_of_statements exceptions_optional
			{ $$.p1 = $2;
			  $$.p2 = $3;
			  $$.attach = $1.attach; }
		;
/**** 7.4 ****/

private_type_definition : PRIVATE
			{ $$ = newnode(simple_node, nop_private);
			  cm($$,$1.attach,-1); }
		| LIMITED PRIVATE
			{ $$ = newnode(simple_node, nop_limited);
			  cm($$,$1.attach,$2.attach,-1); }
		;

/**** 8.4 ****/

use_clause	: USE package_name_comma_list ';'
			{ $$ = newnode(use_node, nop_use); 
			  $$->nd.usex.list = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
package_name_comma_list : package_name
			{ $$ = newnode(name_list_node, nop_name);
			  $$->nd.name_list.name = $1; }
		| package_name_comma_list ',' package_name
			{ nodep p;
			  p = newnode(name_list_node, nop_name);
			  p->nd.name_list.name = $3;
			  $$ = cmappend($1,p,$2.attach);  }
		;
/**** 8.5 ****/

renaming_declaration : identifier_list ':' name RENAMES name ';'
			{ $$ = newnode(obj_rename_node, nop_obj_ren);
			  $$->nd.obj_rename.id_list = $1;
			  $$->nd.obj_rename.typex = $3;
			  $$->nd.obj_rename.old_name = $5;
			  cm($$,$2.attach,$4.attach,$6.attach,-1); }
		| identifier_list ':' EXCEPTION RENAMES name ';'
			{ $$ = newnode(obj_rename_node, nop_except_ren);
			  $$->nd.obj_rename.id_list = $1;
			  $$->nd.obj_rename.old_name = $5;
			  cm($$,$2.attach,$3.attach,$4.attach,$6.attach,-1); }
		| PACKAGE Identifier RENAMES name ';'
			{ $$ = newnode(pack_ren_node, nop_pack_ren);
			  $$->nd.pack_ren.id = $2.val;
			  $$->nd.pack_ren.old_name = $4;
			  cm($$,$1.attach,$2.attach,$3.attach,$5.attach,-1); }
		| TASK Identifier RENAMES name ';'
			{ $$ = newnode(pack_ren_node, nop_task_ren);
			  $$->nd.pack_ren.id = $2.val;
			  $$->nd.pack_ren.old_name = $4;
			  cm($$,$1.attach,$2.attach,$3.attach,$5.attach,-1); }
		| subprogram_specification RENAMES name ';'
			{ $$ = $1;
			  if ($$->ndf.op = nop_function) 
				$$->ndf.op = nop_func_ren;
			  else
				$$->ndf.op = nop_proc_ren;
			  $$->nd.subp.bodyx = $3; 
			  cm($$,$2.attach, $4.attach,-1); }
		;
/**** 9.1 ****/

task_declaration : task_specification
		;
task_specification : TASK Identifier ';'
			{ $$ = newnode(task_node, nop_task);
			  $$->nd.taskx.name = $2.val;
			  cm($$,$1.attach,$2.attach,-1); }
		| TASK TYPE Identifier ';'
			{ $$ = newnode(task_node, nop_task_type);
			  $$->nd.taskx.name = $3.val;
			  cm($$,$1.attach,$2.attach,$3.attach,$4.attach,-1); }
		| TASK Identifier task_specification_part ';'
			{ $$ = newnode(task_node, nop_task);
			  $$->nd.taskx.name = $2.val; 
			  $$->nd.taskx.entryx = $3.p1;
			  $$->nd.taskx.rep_spec = $3.p1;
			  cm($$,$1.attach,$2.attach,$4.attach,-1); }
		| TASK TYPE Identifier task_specification_part ';'
			{ $$ = newnode(task_node, nop_task);
			  $$->nd.taskx.name = $3.val; 
			  $$->nd.taskx.entryx = $4.p1;
			  $$->nd.taskx.rep_spec = $4.p1;
			  cm($$,$1.attach,$2.attach,$3.attach,$5.attach,-1); }
		;
task_specification_part : IS entry_declaration_null_list
			representation_specification_null_list END
			{ $$.p1 = $2;  $$.p2 = $3;
			  $$.attach = append($1.attach,$4.attach); }
		| IS entry_declaration_null_list
			representation_specification_null_list END Identifier
			{ $$.p1 = $2;  $$.p2 = $3;
			  $$.attach = append($4.attach,$5.attach);
			  $$.attach = append($$.attach,$1.attach); }
		;
entry_declaration_null_list :
			{ $$ = null; }
		| entry_declaration_null_list entry_declaration
			{ $$ = append($1,$2); }
		;
task_body	: TASK BODY Identifier IS declarative_part
			XBEGIN sequence_of_statements exceptions_optional 
			END ';'
			{ $$ = newnode(task_node, nop_task_body);
			  $$->nd.taskx.name = $3.val;
			  $$->nd.taskx.decl = $5;
			  $$->nd.taskx.bodyx = $7;
			  $$->nd.taskx.except = $8;
			  cm($$,$1.attach,$2.attach,$3.attach,$4.attach,
				$6.attach,$9.attach,$10.attach,-1); }
		| TASK BODY Identifier IS declarative_part
			XBEGIN sequence_of_statements exceptions_optional END
			Identifier ';'
			{ $$ = newnode(task_node, nop_task_body);
			  $$->nd.taskx.name = $3.val;
			  $$->nd.taskx.decl = $5;
			  $$->nd.taskx.bodyx = $7;
			  $$->nd.taskx.except = $8;
			  cm($$,$1.attach,$2.attach,$3.attach,$4.attach,
				$6.attach,$9.attach,$10.attach,-1); }
		;
/**** 9.5 ****/

entry_declaration : ENTRY Identifier ';'
			{ $$ = newnode(entry_node, nop_entry);
			  $$->nd.entryx.id = $2.val;
			  cm($$,$1.attach,$2.attach,$3.attach,-1); }
		| ENTRY Identifier '(' discrete_range ')' ';'
			{ $$ = newnode(entry_node, nop_entry);
			  $$->nd.entryx.id = $2.val;
			  $$->nd.entryx.rangex = $4;
			  cm($$,$1.attach,$2.attach,$3.attach,$5.attach,
				$6.attach,-1); }
		| ENTRY Identifier formal_part ';'
			{ $$ = newnode(entry_node, nop_entry);
			  $$->nd.entryx.id = $2.val;
			  $$->nd.entryx.formal = $3;
			  cm($$,$1.attach,$2.attach,$4.attach,-1); }
		| ENTRY Identifier '(' discrete_range ')' formal_part ';'
			{ $$ = newnode(entry_node, nop_entry);
			  $$->nd.entryx.id = $2.val;
			  $$->nd.entryx.rangex = $4;
			  $$->nd.entryx.formal = $6;
			  cm($$,$1.attach,$2.attach,$3.attach,$5.attach,
				$7.attach,-1); }
		;
accept_statement : ACCEPT name ';'
			{ $$ = newnode(accept_node, nop_accept);
			  $$->nd.acceptx.name = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		| ACCEPT name DO sequence_of_statements END ';'
			{ $$ = newnode(accept_node, nop_accept);
			  $$->nd.acceptx.name = $2;
			  $$->nd.acceptx.stmts = $4;
			  cm($$,$1.attach,$3.attach,$5.attach,$6.attach,-1); }
		| ACCEPT name DO sequence_of_statements END Identifier ';'
			{ $$ = newnode(accept_node, nop_accept);
			  $$->nd.acceptx.name = $2;
			  $$->nd.acceptx.stmts = $4;
			  cm($$,$1.attach,$3.attach,$5.attach,$6.attach,
				$7.attach,-1); }
		;
/**** 9.6 ****/

delay_statement : DELAY simple_expression ';'
			{ $$ = newnode(delay_node, nop_delay);
			  $$->nd.delayx.expr = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
/**** 9.7 ****/

select_statement : selective_wait
		| conditional_entry_call
		| timed_entry_call
		;
selective_wait	: SELECT when_part_optional select_alternative
			or_part_null_list else_part_optional END SELECT ';'
			{ nodep p;
			  $$ = newnode(select_wait_node, nop_select); 
			  p = newnode(select_when_node, nop_sel_when);
			  p->nd.select_when.whenx = $2;
			  p->nd.select_when.alt = $3;
			  p = append(p,$4);
			  p = append(p,$5);
			  $$->nd.select_wait.or_list = p;
			  cm($$,$1.attach,$6.attach,$7.attach,$8.attach,-1); }
		;
when_part_optional :
			{ $$ = null; }
		| WHEN condition RIGHT_ARROW
			{ $$ = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
or_part_null_list :
			{ $$ = null; }
		| or_part_null_list or_part
			{ $$ = append($1,$2); }
		;
or_part		: OR select_alternative
			{ $$ = newnode(select_when_node, nop_sel_when); 
			  $$->nd.select_when.whenx = $2;
			  cm($$,$1.attach,-1); }
		| OR WHEN condition RIGHT_ARROW select_alternative
			{ $$ = newnode(select_when_node, nop_sel_when); 
			  $$->nd.select_when.whenx = $3;
			  $$->nd.select_when.alt = $5;
			  cm($$,$1.attach,$2.attach,$4.attach,-1); }
		;
else_part_optional :
			{ $$ = null; }
		| ELSE sequence_of_statements
			{ $$ = $2;
			  cm($$,$1.attach,-1); }
		;
select_alternative : accept_statement sequence_of_statements_optional
			{ $$ = append($1,$2); }
		| delay_statement sequence_of_statements_optional
			{ $$ = append($1,$2); }
		| TERMINATE ';'
			{ $$ = newnode(simple_node, nop_terminate);
			  cm($$,$1.attach,$2.attach,-1); }
		;
sequence_of_statements_optional :
			{ $$ = null; }
		| sequence_of_statements
		;
/**** 9.7.1 ****/

conditional_entry_call : SELECT procedure_or_entry_call
			ELSE sequence_of_statements END SELECT ';'
			{ $$ = newnode(cond_entry_node, nop_cond_ent); 
			  $$->nd.cond_entry.call = $2;
			  $$->nd.cond_entry.else_part = $4;
			  cm($$,$1.attach,$3.attach,$5.attach,$6.attach,
				$7.attach,-1); }
		| SELECT procedure_or_entry_call sequence_of_statements
			ELSE sequence_of_statements END SELECT ';'
			{ $$ = newnode(cond_entry_node, nop_cond_ent); 
			  $$->nd.cond_entry.call = $2;
			  $$->nd.cond_entry.stmt = $3;
			  $$->nd.cond_entry.else_part = $5;
			  cm($$,$1.attach,$4.attach,$6.attach,$7.attach,
				$8.attach,-1); }
		;
/**** 9.7.3 ****/

timed_entry_call : SELECT procedure_or_entry_call
			sequence_of_statements_optional OR
			delay_statement sequence_of_statements_optional
			END SELECT ';'
			{ $$ = newnode(timed_entry_node, nop_timed_ent);
			  $$->nd.timed_entry.call = $2;
			  $$->nd.timed_entry.stmt = $3;
			  $$->nd.timed_entry.or_part = append($5,$6);
			  cm($$,$1.attach,$4.attach,$7.attach,$8.attach,
				$9.attach,-1); }
		;
/**** 9.10 ****/

abort_statement : ABORT task_name_comma_list ';'
			{ $$ = newnode(abort_node, nop_abort);
			  $$->nd.abortx.tasks = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
task_name_comma_list : task_name
		| task_name_comma_list ',' task_name
			{ $$ = cmappend($1,$3,$2.attach);  }
		;
/**** 10.1 ****/

compilation	: compilation_unit
		| compilation compilation_unit
			{ $$ = append($1,$2); }
		;
compilation_unit : context_specification subprogram_declaration
			{ $$ = $2;
			  switch ($$->ndf.op) {
				case nop_generic:
				    $$->nd.genericx.context = $1;
				    break;
				case nop_macro:
				    $$->nd.macrox.context = $1;
				    break;
				default:
				    $$->nd.subp.context = $1;
				    break;
			  } }
		| context_specification subprogram_body
			{ $$ = $2;
			  $$->nd.subp.context = $1; }
		| context_specification package_declaration
			{ $$ = $2;
			  $$->nd.packagex.context = $1; }
		| context_specification package_body
			{ $$ = $2;
			  $$->nd.packagex.context = $1; }
		| context_specification subunit
			{ $$ = $2;
			  $$->nd.subunitx.context = $1; }
		;
/**** 10.1.1 ****/

context_specification : with_use_clause_null_list
		;
with_use_clause_null_list :
			{ $$ = null; }
		| with_use_clause_null_list with_clause
			{ $$ = append($1,$2); }
		| with_use_clause_null_list with_clause use_clause
			{ $2 = append($2,$3);
			  $$ = append($1,$2); }
		| with_use_clause_null_list pragma
			{ $$ = append($1,$2); }
		;
with_clause	: WITH unit_name_comma_list ';'
			{ $$ = newnode(use_node, nop_with); 
			  $$->nd.usex.list = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
unit_name_comma_list : unit_name
			{ $$ = newnode(name_list_node, nop_name);
			  $$->nd.name_list.name = $1; }
		| unit_name_comma_list ',' unit_name
			{ nodep p;
			  p = newnode(name_list_node, nop_name);
			  p->nd.name_list.name = $3;
			  $$ = cmappend($1,p,$2.attach);  }
		;
/**** 10.2 ****/

body_stub	: subprogram_specification IS SEPARATE ';'
			{ $$ = $1;
			  $$->nd.subp.is_separate = true;
			  cm($$,$2.attach,$3.attach,$4.attach,-1); }
		| PACKAGE BODY Identifier IS SEPARATE ';'
			{ $$ = newnode(package_node, nop_body);
			  $$->nd.packagex.name = $3.val; 
			  $$->nd.packagex.is_separate = true; 
			  cm($$,$1.attach,$2.attach,$3.attach,$4.attach,
				$5.attach,$6.attach,-1); }
		| TASK BODY Identifier IS SEPARATE ';'
			{ $$ = newnode(task_node, nop_task_body);
			  $$->nd.taskx.name = $3.val; 
			  $$->nd.taskx.is_separate = true; 
			  cm($$,$1.attach,$2.attach,$3.attach,$4.attach,
				$5.attach,$6.attach,-1); }
		;
subunit		: SEPARATE '(' unit_name ')' body
			{ $$ = newnode(subunit_node, nop_subunit); 
			  $$->nd.subunitx.name = $3;
			  $$->nd.subunitx.bodyx = $5;
			  cm($$,$1.attach,$2.attach,$4.attach,-1); }
		;
/**** 11.1 ****/

exception_declaration : identifier_list ':' EXCEPTION ';'
			{ $$ = newnode(except_decl_node, nop_except_decl);
			  $$->nd.except_decl.id_list = $1;
			  cm($$,$2.attach,$3.attach,$4.attach,-1); }
		;
/**** 11.2 ****/

exception_handler : WHEN exception_choice_stroke_list RIGHT_ARROW
			sequence_of_statements
			{ $$ = newnode(when_node, nop_when);
			  $$->nd.whenx.choice_list = $2;
			  $$->nd.whenx.stmt = $4;
			  cm($$,$1.attach,$3.attach,-1); }
		;
exception_choice_stroke_list : exception_choice
			{ $$ = newnode(choice_node, nop_choice);
			  $$->nd.choice.choice = $1; }
		| exception_choice_stroke_list '|' exception_choice
			{ nodep p;
			  p = newnode(choice_node, nop_choice);
			  p->nd.choice.choice = $3; 
			  $$ = cmappend($1,p,$2.attach);  }
		;
exception_choice : exception_name
		| OTHERS
			{ $$ = null; }		/* attachments? */
		;
/**** 11.3 ****/

raise_statement : RAISE ';'
			{ $$ = newnode(raise_node, nop_raise);
			  cm($$,$1.attach,$2.attach,-1); }
		| RAISE exception_name ';'
			{ $$ = newnode(raise_node, nop_raise);
			  $$->nd.raisex.except = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
/**** 12.1 ****/

generic_subprogram_declaration : generic_part subprogram_specification ';'
			{ $$ = newnode(generic_node, nop_generic);
			  $$->nd.genericx.param = $1.p;
			  $$->nd.genericx.unit = $2; 
			  cm($$,$1.attach,$3.attach,-1); }
		;
generic_package_declaration : generic_part package_specification ';'
			{ $$ = newnode(generic_node, nop_generic);
			  $$->nd.genericx.param = $1.p;
			  $$->nd.genericx.unit = $2;
			  cm($$,$1.attach,$3.attach,-1); }
		;
generic_part	: GENERIC
			{ $$.p = null;
			  $$.attach = $1.attach; }
		| GENERIC generic_formal_parameter_list
			{ $$.p = $2;
			  $$.attach = $1.attach; }
		;
generic_formal_parameter_list : generic_formal_parameter
		| generic_formal_parameter_list generic_formal_parameter
			{ $$ = append($1,$2); }
		;
generic_formal_parameter : parameter_declaration ';'
		| TYPE Identifier IS generic_type_definition ';'
			{ $$ = newnode(type_decl_node, nop_type);
			  $$->nd.type_decl.id = $2.val;
			  $$-> nd.type_decl.typex = $4;
			  cm($$,$1.attach,$2.attach,
				$3.attach,$5.attach,-1); }
		| TYPE Identifier discriminant_part IS
			generic_type_definition ';'
			{ $$ = newnode(type_decl_node, nop_type);
			  $$->nd.type_decl.id = $2.val;
			  $$->nd.type_decl.disc = $3;
			  $$-> nd.type_decl.typex = $5;
			  cm($$,$1.attach,$2.attach,
				$4.attach,$6.attach,-1); }
		| WITH subprogram_specification ';'
			{ $$ = newnode(gen_with_node, nop_gen_with); 
			  $$->nd.gen_with.subp = $2; 
			  $$->nd.gen_with.box = false;
			  cm($$,$1.attach,$3.attach,-1); }
		| WITH subprogram_specification IS name ';'
			{ $$ = newnode(gen_with_node, nop_gen_with); 
			  $$->nd.gen_with.subp = $2; 
			  $$->nd.gen_with.name = $4;
			  $$->nd.gen_with.box = false;
			  cm($$,$1.attach,$3.attach,$5.attach,-1); }
		| WITH subprogram_specification IS BOX ';'
			{ $$ = newnode(gen_with_node, nop_gen_with); 
			  $$->nd.gen_with.subp = $2; 
			  $$->nd.gen_with.box = true;
			  cm($$,$1.attach,$3.attach,$4.attach,$5.attach,-1); }
		;
generic_type_definition : '(' BOX ')'
			{ $$ = newnode(simple_node, nop_disc_type);
			  cm($$,$1.attach,$2.attach,$3.attach,-1); }
		| RANGE BOX
			{ $$ = newnode(range_node, nop_range);
			  $$->nd.rangex.not_constrained = true;
			  cm($$,$1.attach,$2.attach,-1); }
		| DELTA BOX
			{ $$ = newnode(range_node, nop_delta);
			  $$->nd.rangex.not_constrained = true;
			  cm($$,$1.attach,$2.attach,-1); }
		| DIGITS BOX
			{ $$ = newnode(range_node, nop_digits);
			  $$->nd.rangex.not_constrained = true;
			  cm($$,$1.attach,$2.attach,-1); }
		| array_type_definition
		| access_type_definition
		| private_type_definition
		;
/**** 12.3 ****/

generic_subprogram_instantiation : subprogram_specification IS
			generic_instantiation ';'
			{ $$ = $1;
			  if ($$->ndf.op == nop_function) 
				$$->ndf.op = nop_func_ginst;
			  else
				$$->ndf.op = nop_proc_ginst;
			  $$->nd.subp.bodyx = $3;
			  cm($$,$2.attach,$4.attach,-1); }
		| FUNCTION designator IS generic_instantiation ';'
			{ $$ = newnode(subp_node, nop_func_ginst);
			  $$->nd.subp.name = $2.val;
			  $$->nd.subp.is_oper = $2.flag;
			  $$->nd.subp.is_separate = false;
			  $$->nd.subp.bodyx = $4;
			  cm($$,$1.attach,$3.attach,$5.attach,-1); }
		;
generic_package_instantiation : PACKAGE Identifier IS
				generic_instantiation ';'
			{ $$ = newnode(package_node, nop_pack_ginst);
			  $$->nd.packagex.name = $2.val;
			  $$->nd.packagex.is_separate = false;
			  $$->nd.packagex.bodyx = $4;
			  cm($$,$1.attach,$2.attach,$3.attach,
				$5.attach,-1); }
		;
generic_instantiation : NEW designator
			{ $$ = newnode(instant_node, nop_instant);
			  $$->nd.instant.name = $1.val;
			  $$->nd.instant.is_oper = $1.flag;
			  cm($$,$1.attach,-1); }
		| NEW designator argument_list
			{ $$ = newnode(instant_node, nop_instant);
			  $$->nd.instant.name = $1.val;
			  $$->nd.instant.is_oper = $1.flag;
			  $$->nd.instant.arg = $3;
			  cm($$,$1.attach,-1); }
		;
/**** 13.1 ****/

representation_specification : length_or_enumeration_specification
		| record_type_representation
		| address_specification
		;
/**** 13.2 ****/

length_or_enumeration_specification : FOR name USE expression ';'
			{ $$ = newnode(rep_spec_node, nop_len_spec); 
			  $$->nd.rep_spec.name = $2;
			  $$->nd.rep_spec.expr = $4;
			  cm($$,$1.attach,$3.attach,$5.attach,-1); }
		;
/**** 13.4 ****/

record_type_representation : FOR name USE RECORD alignment_clause_optional
			location_clause_null_list END RECORD ';'
			{ $$ = newnode(rec_rspec_node, nop_rec_spec); 
			  $$->nd.rec_rspec.name = $2;
			  $$->nd.rec_rspec.algn = $5;
			  $$->nd.rec_rspec.loc = $6;
			  cm($$,$1.attach,$3.attach,$4.attach,$7.attach,
				$8.attach,$9.attach,-1); }
		;
alignment_clause_optional :
			{ $$ = null; }
		| alignment_clause ';'
			{ $$ = $1;  cm($$,$2.attach,-1); }
		;
location_clause_null_list :
			{ $$ = null; }
		| location_clause_null_list component_name location ';'
			{ $3->nd.location.name = $2;
			  $$ = append($1,$3);
			  cm($$,$4.attach,-1); }
		;
location	: AT static_simple_expression RANGE range
			{ $$ = newnode(location_node, nop_at);
			  $$->nd.location.expr = $2;
			  $$->nd.location.rangex = $4;
			  cm($$,$1.attach,$3.attach,-1); }
		;
alignment_clause : AT MOD static_simple_expression
			{ $$ = $3; cm($$,$1.attach,$2.attach,-1); }
		;
/**** 13.5 ****/

address_specification : FOR name USE AT static_simple_expression ';'
			{ $$ = newnode(rep_spec_node, nop_addr_spec); 
			  $$->nd.rep_spec.name = $2;
			  $$->nd.rep_spec.expr = $5;
			  cm($$,$1.attach,$3.attach,$4.attach,$6.attach,-1); }
		;
/**** 13.8 ****/

code_statement	: qualified_expression ';'
			{ $$ = newnode(code_node, nop_code);
			  $$->nd.code.expr = $1;
			  cm($$,$2.attach,-1); }
		;
/**** argument lists ****/

argument_list	: '(' argument_part ')'
			{ $$ = $2; cm($$,$1.attach,$3.attach,-1); }
		;
argument_part	: argument_item
			{ $$ = $1; }
		| argument_part ',' argument_item
			{ $$ = cmappend($1,$3,$2.attach);  }
		;
argument_item	: expression
			{ $$ = newnode(arg_node, nop_arg);
			  $$->nd.arg.exp = $1; }
		| name range_constraint
			{ $$ = $2;
			  $$->nd.rangex.type_id = $1;
			  $$->nd.rangex.not_constrained = false; }
		| range
		| Identifier RIGHT_ARROW expression
			{ $$ = newnode(arg_node, nop_arg);
			  $$->nd.arg.name = $1.val;
			  $$->nd.arg.exp = $3;
			  cm($$,$1.attach,$2.attach,-1); }
		;

/**** unssigned stuff ****/

			/* The following kludge makes a name which is */
			/* a string which looks like an operator symbol */
			/* be a string.  Actually, in this case the */
			/* parser produces the wrong parse.  The string */
			/* is a literal which reduces to a primary.     */
			/* Thus, "+"() calls the function "+" with two */
			/* default arguments, but "+" is a string literal */
literal		: numeric_literal
		| string
		| character
		| XNULL
			{ $$ = newnode(simple_node, nop_null); }
		;
package_name	: name
		;
unit_name	: name
		;
loop_name	: name
		;
label_name	: name
		;
task_name	: name
		;
exception_name	: name
		;
component_name	: name
		;
literal_expression : expression
		;
boolean_expression : expression
		;
static_simple_expression : simple_expression
		;
component_subtype_indication : subtype_indication
		;
numeric_literal : real
		| integer
			/* based_real and based_integer are handled	    */
			/* by the scanner.  To the parser they are the same */
			/* as real and integer respectively.		    */
		;

/**** CASTOR ****/

macro_specification : MACRO macro_designator RETURN subtype_indication
			{ $$ = newnode(macro_node, nop_macro);
			  $$->nd.macrox.designator = $2;
			  $$->nd.macrox.is_oper = false;
			  $$->nd.macrox.is_separate = false;
			  $$->nd.macrox.ret_type = $4; 
			  cm($$,$1.attach,$3.attach,-1);  }
		;
macro_body : 	macro_specification IS declarative_part
			XBEGIN sequence_of_statements exceptions_optional
			END macro_designator_optional ';'
			{ $$ = $1;
			  $$->nd.macrox.decl = $3;
			  $$->nd.macrox.bodyx = $5; 
			  $$->nd.macrox.except = $6; 
			  cm($$,$2.attach,$4.attach,$7.attach,$9.attach,-1); }
		;
macro_designator_optional :   /* nothing */
			{ $$ = null; }
		| macro_designator
		;
macro_designator : '{' 
			{in_call_form = true;} 
		  macro_desig_item_list '}'
			{ $$ = newnode(call_form_node, nop_macro_desig);
			  $$->nd.call_form.list = $3;
			  cm($$,$1.attach,$4.attach,-1); 
			  in_call_form = false; }
		;
macro_desig_item_list : macro_desig_item_list macro_desig_item
			{ $$ = append($1,$2); }
		| macro_desig_item
		;
macro_desig_item : Identifier
			{ $$ = newnode(id_decl_node, nop_callf_ident);
			  $$->nd.id_decl.id = $1.val;
			  cm($$,$1.attach,-1); }
		| '('
			{in_call_form = false; } 
		  macro_param_list ')'
			{ $$ = $3;
			  cm($$,$1.attach,$4.attach,-1);
			  in_call_form = true; }
		;
macro_param_list : macro_decl_semi_list 
			{ $$ = newnode(macro_list_node, nop_macro_param_list);
			  $$->nd.macro_list.list = $1; }
		|  macro_array_parameter
			{ $$ = newnode(macro_list_node, nop_macro_param_list);
			  $$->nd.macro_list.list = $1; }
		;
macro_decl_semi_list : macro_decl_semi_list ';' macro_param_decl
			{ $$ = cmappend($1,$3,$2.attach); }
		| macro_param_decl
		;
macro_param_decl : identifier_list
			{ $$ = newnode(param_decl_node, nop_macro_param);
			  $$->nd.param_decl.id_list = $1; }
		| identifier_list ':' subtype_indication
			{ $$ = newnode(param_decl_node, nop_macro_param);
			  $$->nd.param_decl.id_list = $1;
			  $$->nd.param_decl.subtyp = $3;
			  cm($$,$2.attach,-1); }
		| identifier_list ':' subtype_indication
			ASSIGN_OP expression
			{ $$ = newnode(param_decl_node, nop_macro_param);
			  $$->nd.param_decl.id_list = $1;
			  $$->nd.param_decl.subtyp = $3;
			  $$->nd.param_decl.initexp = $5;
			  cm($$,$2.attach,$4.attach,-1); }
		;
macro_array_parameter : identifier_list ':' ARRAY
				/* list should be only one long */
			{ $$ = newnode(param_decl_node, nop_macro_arr_param);
			  if ($1->ndf.next) 
			     	perror("Only one identifier allowed on array declaration");
 			  $$->nd.param_decl.id_list = $1;
			  cm($$,$2.attach,$3.attach,-1); }
		;
castor_declarative_item: decl_calling_form
		| decl_substitution_point
		;
castor_statement : stmt_calling_form
		| stmt_substitution_point
		;
decl_calling_form : calling_form ';'
			{ $$ = $1; 
			  $$->ndf.op = nop_decl_callf; 
			  cm($$,$2.attach,-1); }
		;
name_calling_form : calling_form
			{ $$ = $1; $$->ndf.op = nop_name_callf; }
		;
stmt_calling_form : calling_form ';'
			{ $$ = $1; 
			  $$->ndf.op = nop_stmt_callf;
			  cm($$,$2.attach,-1);  }
		;
type_calling_form : calling_form
			{ $$ = $1; $$->ndf.op = nop_type_callf; }
		;
		/* setting in_call_form causes the scanner to only recognize
			"begin" and "end" as keywords" */
calling_form	: '{' 
			{in_call_form = true;} 
		  calling_form_item_list '}'
			{ $$ = newnode(call_form_node, nop_decl_callf);
			  $$->nd.call_form.list = $3;
			  cm($$,$1.attach,$4.attach,-1); 
			  in_call_form = false;}
		;
calling_form_item_list : calling_form_item_list calling_form_item
			{ $$ = append($1,$2); }
		| calling_form_item
		;
calling_form_item : Identifier
			{ $$ = newnode(id_decl_node, nop_callf_ident);
			  $$->nd.id_decl.id = $1.val;
			  cm($$,$1.attach,-1); }
		| '(' ')'
 			{ $$ = newnode(macro_list_node, nop_callf_arg_list);
			  cm($$,$1.attach,$2.attach,-1); }
		| '(' 
			{ in_call_form = false; }
		  calling_form_argument_list ')'
  			{ $$ = newnode(macro_list_node, nop_callf_arg_list);
			  $$->nd.macro_list.list = $3;
			  cm($$,$1.attach,$4.attach,-1); 
			  in_call_form = true; }
		| XBEGIN 
			{ in_call_form = false; } 
		  call_form_stmt_arg_list END
			{ $$ = newnode(macro_list_node, nop_callf_stmt_arg);
			  $$->nd.macro_list.list = $3;
			  cm($$,$1.attach,$4.attach,-1);
			  in_call_form = true;  }
		;
calling_form_argument_list : calling_form_argument_list ',' 
			calling_form_argument
			{ $$ = cmappend($1,$3,$2.attach); }
		| calling_form_argument
		;
calling_form_argument : expression
			{ $$ = newnode(arg_node, nop_arg);
			  $$->nd.arg.exp = $1; }
		| name range_constraint
			{ $$ = $2;
			  $$->nd.rangex.type_id = $1;
			  $$->nd.rangex.not_constrained = false; }
		| range
		;
call_form_stmt_arg_list : sequence_of_statements
		;
decl_substitution_point : substitution_point ';'
			{ $$ = $1; 
			  $$->ndf.op = nop_decl_subst; 
			  cm($$,$2.attach,-1); }
		;
name_substitution_point : substitution_point
			{ $$ = $1; $$->ndf.op = nop_name_subst; }
		;
stmt_substitution_point : substitution_point ';'
			{ $$ = $1; 
			  $$->ndf.op = nop_stmt_subst;
			  cm($$,$2.attach,-1);  }
		;
type_substitution_point : substitution_point
			{ $$ = $1; $$->ndf.op = nop_type_subst; }
		;
subtype_substitution_point : substitution_point
			{ $$ = $1; $$->ndf.op = nop_subtype_subst; }
		;
ident_substitution_point : substitution_point
			{ $$ = $1; $$->ndf.op = nop_ident_subst; }
		;
substitution_point : '$' Identifier
			{ $$ = newnode(ident_node, nop_decl_subst);
			  $$->nd.ident.id = $2.val;
			  cm($$,$1.attach,$2.attach,-1); }
		;
%%


static nodep append(list,item)
    nodep list, item;
{ 
    register nodep n;

    if (list == null) return item;
    for (n=list; n->ndf.next != null; n = n->ndf.next);
    n->ndf.next = item;
    return list;
}	

static idcheck(ib,ie,ty)
    char *ib, *ie, *ty;
{
    if (strcmp(ib,ie) != 0)
	perror("identifier mismatch %s %s / END %s",ty,ib,ie);
}

# include <varargs.h>

static cm(va_alist)
    va_dcl
{
    va_list pvar;
    nodep p, m;

    va_start(pvar);
    m = (nodep)va_arg(pvar,nodep);	/* first argument is syntax node */
		/******* check this cast to make sure it's right ******/
    while ((int)(p = (nodep)va_arg(pvar,nodep)) != -1) { 
	if (p) {
	    /* printf("attaching \"-- %s\" to %s node\n",p->nd.comment.text,nodename(m)); */
	    m->ndf.ag = append(m->ndf.ag,p);
	}
    } 
    va_end(pvar);
}

 /* cmappend appends an element to a list and attaches a list of 
    attachments to the old last element of the list.  It is used to avoid
    tracing the list pointers twice					*/

 /* It's main application is in lists separated by punctuation.  For example,
    in an enumeration type:						*/
 /*      type x is (a,b,c,d, -- a comment attached to d			*/
 /* the comment should be attached to the node for d.			*/ 
    
static nodep cmappend(list,item,att)
    nodep list, item, att;
{ 
    register nodep n;

    if (list == null) return item;
    for (n=list; n->ndf.next != null; n = n->ndf.next);
    cm(n,att,-1);
    n->ndf.next = item;
    return list;
}	

# ifdef YYDEBUG
    int yydebug;
# endif YYDEBUG

    /*	parser() is the top level call to the parser.  Yacc generates a parser
	which always uses the same start symbol.  Since we want to parse
	a statement list, or a declaration list, or several other possible
	constructs, we force a dummy single character token into the
	scanner lookahead buffer before we start.  This token is the first
	on to be shifted by the parser and it causes the parser to begin
	parsing the actual input in the correct state. */

nodep parser(construct)
    enum parser_construct construct;
{
		/* if yaccdebug is set in the environment and nonzero set  */
		/* yydebug (parser trace) */
		/* note that parser.c must be compiled with YYDEBUG defined */
# ifdef YYDEBUG
	yydebug = getval("yaccdebug");
# endif YYDEBUG
	scan_debug();		/* similiarly for scanner (see scanner.l) */
	scan_reset();

    switch (construct) {
	case interactive_input:		w_unput('+');  break;
	case expression:		w_unput('&');  break;
	case name:			w_unput('%');  break;
	case declarative_part:		w_unput('*');  break;
	case sequence_of_statements:	w_unput('-');  break;
	case type_definition:		w_unput('=');  break;
	case subtype_indication:	w_unput(':');  break;
	case compilation:		w_unput(';');  break;
    }

    yyparse();
    return parsetree;
}

