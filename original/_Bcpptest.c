static char rcsid[] = "$Header:   /usr/ics/darpa/ada/RCS/cpp.v  Revision 1.1  82/12/12  21:21:44  tadman  Rel1-3E$";

/* $Log:	/usr/ics/darpa/ada/RCS/cpp.v $
 * Revision 1.1  82/12/12  21:21:44  tadman
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
#include "oblist.h"

extern	typep typeonly();
fixp eval();
extern typedval dummy;
extern symtabp parameter_table;	/* holds parameters */
static int cdebug;              /* holds interpreter flag for each */
                                /* invocation of cpp */ 
static boolean inspec;		/* if true, then compiling spec, else body 
				   of package */
static FILE    *outfile;
static int     in;     /* indentation level */

char *gensym()          /* generate a unique symbol name with my */
                        /* initials in it; result is upper case so */
                        /* that it can't conflict with any Ada names */
{
        static int symcount = 0;
        static char symbol[15]; /* 12 digits should be enough */
        sprintf(symbol,"SHW%d",symcount++);
        return strsav(symbol);
} 

comperr(s,a,b,c,d,e)
char *s;
{       static char errstr[256];
        fclose(outfile);
        sprintf(errstr,s,a,b,c,d,e);
        sysputf("Compiler error: %s", errstr);
        reinit();
}

static inby(n) /* increase indentation level by n */
{       in += n;
}

static outby(n) /* decrease indentation level by n */
{       in -= n;
}

static indent(i) {
        while(i--)
                fputf(outfile," ");
}

char    *gentab[];      /* table of tokens => strings */

char *gennode(p)
	nodep p;
{	if (p) return (gentab[(int)p->ndf.op]);
                        /* lookup string representation of token in table */
	else return "*NULL (0) NODE*";
}

static pp_new_line()    /* move to new line on output file */
{                       /* was created to handle comments; who cares */
        fputf(outfile,"\n");   
}

compiler(f,ada_fname,wantmain)  /* translate Ada to C */
fixp f;                        /* name of procedure; may be selected... */
arrayp ada_fname;               /* pointer to unconstrained string for */
boolean wantmain;               /* file name; true if should be compiled */
{                               /* as main procedure */
        char fname[256];

        str_ada_to_c(fname, ada_fname); /* convert filename to C format */

        if (*fname eq 0)	/* check if present */
                error("Must give a filename to the compiler");

        sysputf("Writing file %s",fname);
        outfile = fopen(fname,"w");
	if (strcmp("/dev/tty",fname) eq 0) setbuf(stdout,NULL);

	in _ 0;

        fputf(outfile,"\n#include \"adamacs\"\n");
                                /* macro definitions make Ada go down easy */
        cdebug _ getval("cdebug");
                                /* if "cdebug" is defined in the environment */
                                /* and nonzero then print debugging info */

        switch(f->fxf.class) {	/* only compile procedures or functions or
				   packages */
                case func_id:
                case proc_id:
                        {nodep procp = f->fx.func.int_form;
                                /* get internal form for procedure */
                        if (procp)  {
                            if (wantmain) 
                              fputf(outfile,"\n#define %s main\n", 
                                      f->fxf.name);
			    else fputf(outfile, "\nmain () {}\n");
                            comp_subp_decl(procp);  
                                        /* call compiler proper */
                            fclose(outfile);
                        }
                        else error("subprogram has no body");
                        break;
                        }
                case type_id:
                case subtype_id:
                        error("can't compile a type or subtype");
                case pack_id:
			{
			nodep packp _ f->fx.pack.int_form;
			nodep packp_body = f->fx.pack.body_int_form;
			if (packp) comp_package_spec(packp);
			else error("No spec for package");
			if (packp_body) comp_package_body(packp_body);
			else fputf(outfile,"/* package has no body */\n");
			fclose(outfile);
                        break;
			}

                default:
                        error("can't compile that, whatever it is");
        }                                    
}


xcpp_list(p) nodep p;
{	nodep q _ p;
	while (q) {
		xcpp(q);
		q _ q -> ndf.next;
	}
}

xcpp(p)                 /* top level switching statement; I hope to */
nodep p;                /* get rid of it ASAP */
{
        if(p eq null)  {
#ifdef DEBUG_NULL
		fputf(outfile,"/* null */");
#endif
                return;
        };

        if (cdebug)         /* help track down calls to this thing */
                fputf(outfile,"\n/* called via xcpp => %s */\n",gennode(p));

        switch(p->ndf.op) {
                case nop_ident:                
                        comp_ident_reference(p);
                        break;
                case nop_int:
                        comp_integer(p);
                        break;
                case nop_char:
                        comp_character(p);
                        break;
                case nop_real:
                        comp_real(p);
                        break;
                case nop_string:               
                        comp_strconst(p);
                        break;
                case nop_new:
                        comp_dyna_new(p);
                        break;
                case nop_null:
                        comp_null_reference();
                        break;
                case nop_lp:
                        comp_paren_expr(p);
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
                        comp_boolean_op(p);
                        break;
                case nop_neg:
                case nop_pos:
                case nop_not:
                        comp_unary_op(p);
                        break;
                case nop_dot:
                        comp_field_select(p);
                        break;
                case nop_call:                 
                        comp_call_form(p);
                        break;
                case nop_arg:
                        comp_subp_args(p);
                        break;
                case nop_asgn:    
                        indent(in);                
                        comp_assignment(p);
                        break;                     
                case nop_proc_call:
                        indent(in);
                        comp_procedure_call(p);
                        break;
                case nop_if:
                        comp_if_statement(p);
                        break;
                case nop_elsif:
                        comp_elsif_clauses(p);
                        break;
                case nop_loop:
                        comp_simple_loop(p);
                        break;
                case nop_while:
                        comp_while_loop(p);
                        break;
                case nop_return:         
                        comp_return_statement(p);
                        break;
                case nop_id_decl:        
                        comp_identifier_decl(p);
                        break;
                case nop_type:
                        comp_type_declaration(p);
                        break;
                case nop_subtype:
                        comp_subtype_declaration(p);
                        break;
                case nop_obj_decl:
                        comp_object_declaration(p);
                        break;
                case nop_param:         
                        comp_parameter_decl(p);
                        break;
        /*      case nop_array:
                        comp_array_decl(p);
                        break;  */
                case nop_index:
                        comp_array_index_elements(p);
                        break;
                case nop_record:
                        comp_record_decl(p);
                        break;
                case nop_access:
                        comp_access_decl(p);
                        break;
                case nop_procedure:
                case nop_function:
                        comp_subp_decl(p);
                        break;
                case nop_package:
                        comp_package_spec(p);
                        break;
                case nop_body:
                        comp_package_body(p);
                        break;
                case nop_enum:
                        comp_enumeration_decl(p);
                        break;                
                case nop_range:
                        comp_range_decl(p);
                        break;
                case nop_in:
                        comp_in_range_test(p);
                        break;
                case nop_for: 
                        comp_for_statement(p);
                        break;                            
                case nop_null_stmt:
                        comp_null_statement(p);
                        break;
                case nop_case:   
                        comp_case_statement(p);
                        break;
		case nop_exit:
			comp_exit_statement(p);
			break;
                default:
                        fputf(outfile,"/* case %s not implemented */", gennode(p));
                        pp_new_line();
                        break;
        }
}

comp_exit_statement(p) nodep p;
{	if (p->nd.exitx.loop_name) comperr("Can't name loops");
	if (p->nd.exitx.cond) comperr("Can't EXIT WHEN; use if ... exit; ...");
	indent(in);
	fputf(outfile, "break;");
	pp_new_line();
}

comp_ident_reference(p) 
    nodep p;
{
    fixp f;

    if (p -> ndf.op eq nop_ident)
	f = lookup(currtab, p->nd.ident.id, null);
    else
	comperr ("Not an identifier in comp_ident_reference");

    if (!f)
	comperr ("Undefined identifier %s", p -> nd.ident.id);

    if (f -> fxf.class eq type_id)
	fputf (outfile, "%s", p -> nd.ident.id);
    else if (f -> fxf.class eq proc_id or f -> fxf.class eq func_id)
	    fputf (outfile, "%s", p -> nd.ident.id);
    else if (f -> fxf.class eq param_id) {
		boolean is_ref_param _
		    f -> fxf.class eq param_id and f -> fx.var.param_is_out;
		if (not blocktype(f -> fxf.dyna -> sy.var.type) and 
			is_ref_param)
		    fputf (outfile, "*(int *)");
		fputf (outfile, "%s", p -> nd.ident.id);
	    }

    else if (f -> fxf.class eq obj_id)
		    fputf (outfile, "%s", p -> nd.ident.id);

    else comperr ("Unimp syf.class in ident_reference");
}

comp_integer(p)
nodep p;
{
                        fputf(outfile,"%d", p->nd.integer.val);
}

comp_character(p)
nodep p;
{
                        fputf(outfile,"'%c'", p->nd.integer.val);
}

comp_real(p)
nodep p;
{
	fputf(outfile,"%g", p->nd.real.val);
} 

comp_strconst(p)
nodep p;
{
	fputf(outfile,"STRCONST(\"%s\")", p->nd.ident.id);
}

comp_dyna_new(p)         /* allocate dynamic memory via new */
nodep p;
{
                        fputf(outfile,"(int)malloc %s", p->nd.newx.type_id);
}
              
comp_null_reference()
{
    fputf(outfile,"null");
}

comp_paren_expr(p)
nodep p;
{
                        fputf(outfile,"(");
                        xcpp(p->nd.uop.child);
                        fputf(outfile,")");
}               

comp_boolean_op(p)  
nodep p;
{
                        fputf(outfile,"(");
                        comp_expression(p->nd.binop.left);
                        fputf(outfile," %s ", gennode(p));
                        comp_expression(p->nd.binop.right);
                        fputf(outfile,")");
}

comp_unary_op(p)
nodep p;
{
                        fputf(outfile,"(%s ", gennode(p));
                        comp_expression(p->nd.uop.child);
                        fputf(outfile,")");
}

comp_field_select(p)
nodep p;
{
                        xcpp(p->nd.selectx.var);
                        if(p->nd.selectx.field != null)
                                fputf(outfile,".%s", p->nd.selectx.field);
                     /* else fputf(outfile,".all");    */
}


comp_call_form(p)       /* this covers anything in Ada that looks like */
nodep p;                /* name(stuff).  This includes functions, slices, */
                        /* arrays, etc.  Not included are procedure calls */
                        /* which are a type of statement. */
{
        char *id;       /* name of function or array */
	fixp f;		/* fixed part of symbol */
        nodep caller;

        caller _ p->nd.call.var;   /* id node */
	if (caller->ndf.op eq nop_ident) {
		id _ caller->nd.ident.id;
		/* print the identifier for the function or array */
		comp_ident_reference(caller);
	}			/* could be selected... */
	else if (caller->ndf.op eq nop_quote) {
		/* something like character'val => (character); it is */
		/* simply made into a cast */
		/* anything'pos => (int) */
		id _ caller->nd.selectx.var->nd.ident.id;
		fputf(outfile,"(");
		if (caller->nd.selectx.field eq intern("pos"))
			fputf(outfile, "int");
		else comp_ident_reference(caller->nd.selectx.var);
		fputf(outfile,")");
	}
        f _ lookup(currtab, id, null);
        if (!f)  comperr("Function or array or type %s not defined",id);
        if ((f->fxf.class eq func_id) or (f->fxf.class eq type_id)) {
                fputf(outfile,"(");	/* ... eq subtype too? */
                comp_subp_args(p->nd.call.args);
                fputf(outfile,")");
        }
        else            /* hopefully array reference */
            comp_array_args(p->nd.call.args);
}

comp_subp_args(p)       /* We have to do the work here of putting the */
nodep p;                /* arguments in the right order if named, and */
{                       /* of filling in defaults.  */
        if (not p)  return; /* this whole mess needs a lot of work */
        if(p->nd.arg.name != null) 
                fputf(outfile,"/* %s => */", p->nd.arg.name);
        comp_expression(p->nd.arg.exp);
        if(p->ndf.next != null)  {
                fputf(outfile,",");
                xcpp(p->ndf.next);
        };
}

comp_array_args(p)      /* We have to do the work here of putting the */
nodep p;                /* arguments in the right order if named, and */
{                       /* of filling in defaults.  */
                        /* arrays print much differently in C; instead of */
                        /* (1,2,3) they print [1][2][3]; very silly.  */
	if (p) {	/* first one must get past dope vector */
		fputf(outfile, "->arr_base[");
		comp_expression(p->nd.arg.exp);
		fputf(outfile, "]");
		p _ p->ndf.next;
	}
        while(p) {	/* now do the rest of 'em */
                if(p->nd.arg.name != null) 
                        fputf(outfile,"/* %s => */", p->nd.arg.name);
                fputf(outfile,"[");
                comp_expression(p->nd.arg.exp);
                fputf(outfile,"]");
                p _ p->ndf.next;
        };
}

comp_assignment(p)
nodep p;
{
        xcpp(p->nd.asgn_stmt.lhs);         /* this needs a name */
        fputf(outfile," = ");
        comp_expression(p->nd.asgn_stmt.rhs);   
        fputf(outfile,";");
        pp_new_line();
        xcpp(p->ndf.next);
}

comp_procedure_call(p)
nodep p;
{
	/* hack get and put overloading */
	if (p->nd.call.var -> ndf.op eq nop_ident) {
	 char *id _ p->nd.call.var -> nd.ident.id;
	 if(p->nd.call.args and
	    ((id eq intern("get")) or
	     (id eq intern("put")))) {
		typep arg1 _ typeonly(p->nd.call.args->nd.arg.exp);
		char *ending;
		if (not arg1)
			comperr("Can't get type of first arg to %s", id);
		if (compatible(strtype, arg1)) ending _ "str";
			/* strtype must be first arg to compatible
			   because it is unconstrained type */
		else if (compatible(inttype, arg1)) ending _ "int";
		else if (compatible(chartype, arg1)) ending _ "chr";
		else if (compatible(realtype, arg1)) ending _ "real";
		else comperr("Can't un-overload %s", id);
		fputf(outfile, "%s%s", id, ending);
	 }
         else comp_ident_reference(p->nd.call.var);
	}
        else comp_ident_reference(p->nd.call.var);

                        /* could be selected component notation */
        if(p->nd.call.args) {         /* arguments to the procedure? */
                fputf(outfile,"(");
                comp_subp_args(p->nd.call.args);
                fputf(outfile,")");
        }
        else fputf(outfile,"()");
        fputf(outfile,";");      
        pp_new_line();
        xcpp(p->ndf.next);     /* on to the next statement */
}

comp_if_statement(p)
nodep p;
{
                        indent(in);
                        fputf(outfile,"if(");
                        comp_expression(p->nd.elsifx.expr);
                        fputf(outfile,") {");
                        pp_new_line();
                        inby(2);
                        xcpp(p->nd.elsifx.then_part);
                        outby(2);
                        xcpp(p->nd.ifx.elsif_list);
                        indent(in);
                        fputf(outfile,"}");
                        pp_new_line();
                        xcpp(p->ndf.next);
}

comp_elsif_clauses(p) 
nodep p;
{
                        indent(in);
                        if(p->nd.elsifx.expr) {
                                fputf(outfile,"} else if(");
                                comp_expression(p->nd.elsifx.expr);
                                fputf(outfile,") {");
                                pp_new_line();
                        }
                        else {
                             fputf(outfile,"} else {");
                             pp_new_line();
                        }
                        inby(2);
                        xcpp(p->nd.elsifx.then_part);
                        outby(2);
                        xcpp(p->ndf.next);
}

comp_case_statement(p)
nodep p;
{                                                     
                        nodep q,m;

                        indent(in);
                        fputf(outfile, "switch(");
                        comp_expression(p->nd.casex.expr);
                        fputf(outfile, ")  {");
                        inby(4);
                        for (q = p->nd.casex.when_list; q != null; q = q->ndf.next) 
                        {           
                                for (m = q->nd.whenx.choice_list; 
                                     m != null;            
                                     m = m->ndf.next)        
                                {               
                                        pp_new_line();
                                        indent(in);
                                        if (m->nd.choice.choice == null)
                                                fputf(outfile,"default:");
                                        else
                                        {
                                                fputf(outfile, "case ");
                                                xcpp(m->nd.choice.choice);
                                                fputf(outfile, ":");
                                        }
                                }
                                inby(4);
                                pp_new_line();
                                xcpp(q->nd.whenx.stmt);
                                indent(in);
                                fputf(outfile, "break;");
                                outby(4);
                        }       
                        pp_new_line();
                        indent(in);
                        fputf(outfile,"}");
                        pp_new_line();
                        outby(4);
                        xcpp(p->ndf.next);
}  


comp_simple_loop(p)
nodep p;
{
                        indent(in);
                        fputf(outfile,"while(1) {");
                        pp_new_line();
                        inby(2);
                        xcpp(p->nd.whilex.bodyx);
                        outby(2);
                        indent(in);
                        fputf(outfile,"} /* end loop */");
                        pp_new_line();
                        xcpp(p->ndf.next);
}

comp_while_loop(p)              
nodep p;
{
                        indent(in);      
                        fputf(outfile,"while(");
                        comp_expression(p->nd.whilex.expr); 
                        fputf(outfile,") {");
                        pp_new_line();
                        inby(2);
                        xcpp(p->nd.whilex.bodyx);
                        outby(2);
                        indent(in);
                        fputf(outfile,"} /* end while */");
                        pp_new_line();
                        xcpp(p->ndf.next);
}

comp_return_statement(p)
nodep p;
{
                        indent(in);
                        fputf(outfile,"return");
                        if (p->nd.returnx.val)  {
                                fputf(outfile," ");
                                comp_expression(p->nd.returnx.val);
                        };
                        fputf(outfile,";");
                        pp_new_line();
                        xcpp(p->ndf.next);
}

comp_identifier_decl(p)         /* simply prins a list of identifiers */
nodep p;                        /* separated by commas */
{
        while (p) {
                fputf(outfile,"%s",p->nd.id_decl.id);
                p _ p->ndf.next;
                if (p)  fputf(outfile,",");
        };            /* removed tail recursion */
}

comp_type_declaration(p)        /* type arr is array(1..10) of integer; */
nodep p;                        /* type color is (red,green,blue); */
{                               /* that type of thing */
        register nodep typ;
        indent(in);
        typ _ p->nd.type_decl.typex;
        if(typ->ndf.op eq nop_array) 
                comp_array_type_def(typ, p->nd.type_decl.id);
        else if (typ->ndf.op eq nop_enum)  {
                fputf(outfile,"typedef enum ");
                comp_enumeration_decl(typ);
                fputf(outfile," %s",p->nd.type_decl.id);
                fputf(outfile,";\n");
        }
        else comperr("Only know about array and enumeration types");

/*      xcpp(p->ndf.next);     /* done in loop now */
}

comp_subtype_declaration(p)
nodep p;
{
                        indent(in);
                        fputf(outfile,"typedef /*%s*/ ", tokname(p->ndf.op));
                        fputf(outfile,"%s ", p->nd.type_decl.id);
                        pp_new_line();
                        inby(2);
                        xcpp(p->nd.type_decl.typex);
                        outby(2);       
                        fputf(outfile,";");
                        pp_new_line();
                /*      xcpp(p->ndf.next);     /* done in loop now */
}

comp_object_declaration(p)      /* i : integer; => integer i;   */
nodep p;                        
{
        indent(in);
        if (p->nd.obj_decl.subtypex->ndf.op eq nop_array)       
                comp_array_decl(p);
        else {
                xcpp(p->nd.obj_decl.subtypex);  /* integer, character; something */
                fputf(outfile," ");   /* easy like that */
                xcpp(p->nd.obj_decl.id_list);
                if(p->nd.obj_decl.initexp) {
                        fputf(outfile," = ");
                        comp_expression(p->nd.obj_decl.initexp);
                }
                if(p->nd.obj_decl.is_constant) fputf(outfile,"/* constant */");
                fputf(outfile,";");
                pp_new_line();
        }; 
    /*  xcpp(p->ndf.next);     /* done in loop now */
}

comp_parameter_decl(p)  /* f(a, b : integer; c,d : integer) => f(a,b,c,d) */
nodep p;                /* other stuff is handled elsewhere */
{
        while(p) {
                comp_identifier_decl(p->nd.obj_decl.id_list);
                p _ p->ndf.next;
                if (p)  fputf(outfile,",");
        };
}

comp_array_decl(p)      /* declare an array... ; we are passed the entire */
nodep p;                /* object declartion node, since we have to */
{                       /* change the order around for C.  */
                        /* a,b : array(1..10) of integer; => */
                        /* integer a[9],b[9];  yecch */
                        /* actually, a[10], but that's a hack */
                        /* This is the anonymous case; actual typedefs */
                        /* are done elsewhere */
        char *defname;  /* name of typedef symbol gensym'ed */
        defname _ gensym();     /* generate unique name */
        comp_array_type_def(p->nd.obj_decl.subtypex, defname);
                        /* create the actual typedef */
        indent(in);                
        fputf(outfile,"%s ",defname); /* gensym'ed name (which is typedef) */
        comp_identifier_decl(p->nd.obj_decl.id_list);
        fputf(outfile,";\n");
        free(defname);          /* don't need this anymore */
}

comp_array_type_def(arrp, defname)      /* given an array node and a name */
nodep arrp;             /* (which may be a made up one for anon types */
char *defname;          /* generate a suitable C typedef */
{
        fputf(outfile,"typedef ");
                        /* create a typedef for the array; this lets us */
                        /* declare any number of id's (a,b above) more */
                        /* easily.  =>typedef integer SHW0[10]; */
        xcpp(arrp->nd.arrayx.ele_type); 
                        /* only works for simple stuff now */
        fputf(outfile," %s", defname);
        comp_array_index_elements(arrp->nd.arrayx.index_list);
        fputf(outfile,";\n");
}
comp_array_index_elements(p)    /* only print a suitable range */
nodep p;                        /* this is a list of index_nodes */
                                /* which contain range-nodes */
{
        nodep rangep;
        rangep _ p;
        while (rangep) {
                fputf(outfile,"[");
                fputf(outfile,"%d",
                      rangep->nd.index.typex->nd.rangex.right->nd.integer.val+1);
                                /* this is a super hack; for now we */
                                /* just print the upper limit of the */
                                /* range */
                fputf(outfile,"]");
                rangep _ rangep->ndf.next;
        };
}

comp_record_decl(p)
nodep p;
{
                        indent(in);
                        fputf(outfile,"struct {");
                        pp_new_line();
                        inby(2);
                        xcpp(p->nd.recordx.field_list);
                        outby(2);
                        indent(in);
                        fputf(outfile,"};");
}

comp_access_decl(p)
nodep p;
{
                        indent(in);
                        fputf(outfile,"*"); 
                        xcpp(p->nd.accessx.target_type);
}

comp_package_spec(p)
nodep p;
{
	boolean bspec _ inspec;
	inspec _ true;
        indent(in);                     
        fputf(outfile,"/* package spec %s is */\n",
		p->nd.packagex.name);
        pp_new_line();
        if(p->nd.packagex.decl) {
            inby(2);
            xcpp_list(p->nd.packagex.decl);
            outby(2);
        }
        fputf(outfile,"\n/* end package spec %s */\n\n", 
		p->nd.packagex.name);
        pp_new_line();
	inspec _ bspec;
}

comp_package_body(p)
nodep p;
{
	boolean bspec _ inspec;
	inspec _ false;
        fputf(outfile,"/* package body %s is */",
		p->nd.packagex.name);
        pp_new_line();
        if(p->nd.packagex.decl) {
            inby(2);
            xcpp_list(p->nd.packagex.decl);
            outby(2);
        }
        if(p->nd.packagex.bodyx)   {
            indent(in);
            fputf(outfile,"\nmain() {");
            pp_new_line();
            inby(2);
            xcpp_list(p->nd.packagex.bodyx);
            outby(2);
        }   
        indent(in);
        fputf(outfile,"\n/* end package body %s */\n", 
		p->nd.packagex.name);
        pp_new_line();
	inspec _ bspec;
}

comp_enumeration_decl(p)    /* in type color is (red,green,blue); changes */
nodep p;                    /* (red,green,blue) => {red,green,blue} */
{
        fputf(outfile,"{");         
        comp_identifier_decl(p->nd.enumtype.ids);
        fputf(outfile,"}");
}

comp_range_decl(p)
nodep p;
{
        if (p->nd.rangex.type_id) 
                fputf(outfile,"--%s range ",p->nd.rangex.type_id->nd.ident.id);
        if (p->nd.rangex.not_constrained) 
                fputf(outfile,"<>");
        else {
                xcpp(p->nd.rangex.left);
                fputf(outfile,"..");
                xcpp(p->nd.rangex.right);
        };                    
}

comp_in_range_test(p)
nodep p;
{
        if(p->nd.inop.not_in)      /* negate result if that's what's wanted */
                fputf(outfile," not(");

        fputf(outfile," IN(");        /* function to do range test;   */
                                        /* must be done with a function */
                                        /* as side-effects must be avoided */
        comp_expression(p->nd.inop.expr);       /* first arg is expression */
        fputf(outfile,",");                   /* arg separator */
        comp_expression(p->nd.inop.rangex->nd.rangex.left);
                                        /* must be constrained and all that */
                                        /* lower bound */
        fputf(outfile,",");
        comp_expression(p->nd.inop.rangex->nd.rangex.right);
                                        /* upper bound */
        fputf(outfile,")");           /* closing paren for "IN(..." */

        if(p->nd.inop.not_in)      /* closing paren for "not", if needed */
                fputf(outfile,")");
}

comp_for_statement(p)
    nodep p;
{
    register char  *q = p -> nd.forx.cntrl_var;
    symtabp tab;		/* symbol table for for loop scope */
    fixp f;			/* fix part of loop control var */
    symp s;			/* symbol entry for loop control var */

    tab _ newtab (4, null);	/* no owner for for loops */
		/* FIX THIS */
 /* could probably be of size one */

    f = newfix(varfixform, obj_id);
    s _ newsym (varsymbolform, obj_id);
    f -> fxf.name _ p -> nd.forx.cntrl_var;
    s -> sy.var.type _ inttype;	/* subrange type for loop var */
    enter(currtab, f,s);		/* toss into symbol table */

    indent (in);		/* begin for loop */
    fputf (outfile, "{int %s;", q);/* make control var local */
    pp_new_line ();
    indent (in);
    fputf (outfile, "for(");
    fputf (outfile, "%s = ", q);/* initial value */

    if (p -> nd.forx.reversex) {/* count backwards */
	comp_expression (p -> nd.forx.lims -> nd.rangex.right);
	fputf (outfile, "; %s >= ", q);
	comp_expression (p -> nd.forx.lims -> nd.rangex.left);
	fputf (outfile, "; %s--)", q);
	fputf (outfile, " /* reverse loop */ ");
    }
    else {			/* count forwards */
	comp_expression (p -> nd.forx.lims -> nd.rangex.left);
	fputf (outfile, "; %s <= ", q);
	comp_expression (p -> nd.forx.lims -> nd.rangex.right);
	fputf (outfile, "; %s++)", q);
    };
    fputf (outfile, " {");	/* beginning of body */
    pp_new_line ();
    inby (2);
    xcpp (p -> nd.forx.bodyx);
    outby (2);
    indent (in);
    fputf (outfile, "}");
    fputf (outfile, "} /* end for loop */");
    pp_new_line ();
    xcpp (p -> ndf.next);
}

comp_null_statement(p)
nodep p;
{
                        indent(in);
                        fputf(outfile,"/* null*/;");
                        pp_new_line();
                        xcpp(p->ndf.next);
}


comp_subp_decl(p)       /* compile function or procedure declaration */
    nodep p;
{
    symtabp tab;
    fixp paramlist;
    nodep findsubp;
    symtabp savetab _ currtab;

  if (not inspec) {
    tab _ newtab (32, null);
    tab -> symtab_static_link _ currtab;
    parameter_table _ tab;	/* global for nop_param */

    paramlist _ (fixp)eval (p -> nd.subp.spec, decl_mode, &dummy, null);
				/* eval parameters into tab, also return
				   them as a list to paramlist */

 /* Need to add code to check that all params have been set */

    currtab _ tab;		/* presumably the static link is set somehow */
    eval (p -> nd.subp.decl, decl_mode, &dummy, null);
				/* add locals to symbol table. uses
				   current table */

    pp_new_line ();		/* generate C using the info in the symbol
				   table */

 /* Generating the locals must be done in three parts: */
 /* type definitions first, since every may depend on them */
 /* Next, object definitions, because they get packed into */
 /* a record representing the local stack frame (they do?) */
 /* Lastly, subprogram definitions, since they might reference */
 /* non-local objects, so they depend on the stack frame record */
 /* definition.  */

    if (p -> nd.subp.decl) {	/* do local declarations */
	pp_new_line ();		/* types and objects, but not */
	inby (2);		/* subprograms */
	{
	    register    nodep nosubps _ p -> nd.subp.decl;
	/* traverse declarations; omit subps */
	    for (; nosubps; nosubps _ nosubps -> ndf.next)
		if (nosubps -> ndf.op neq nop_function and
			nosubps -> ndf.op neq nop_procedure)
		    xcpp (nosubps);/* type, subtype or object */

	}
	outby (2);
    }


 /* now emit local procedures */
    for (findsubp _ p -> nd.subp.decl; findsubp; findsubp _
	    findsubp -> ndf.next)
	if (findsubp -> ndf.op eq nop_function or
		findsubp -> ndf.op eq nop_procedure)
	    comp_subp_decl (findsubp);

 /* all local definitions and objects have been done; */
 /* now produce this subprogram */
  }	/* end if (not inspec) */
    if (p -> nd.subp.ret_type) {/* if function, print return type */
	xcpp (p -> nd.subp.ret_type);
	fputf (outfile, " ");
    }
    fputf (outfile, "%s(", p -> nd.subp.name);
  if (inspec) fputf(outfile, ");\n");
  else {
 /* output subprogram name */
    if (p -> nd.subp.spec) {	/* declare arguments */
	inby (2);
	xcpp (p -> nd.subp.spec);
	outby (2);
    }
    fputf (outfile, ")\n");

 /* now traverse paramlist and print types of params */
    while (paramlist) {
	if (paramlist -> fxf.dyna -> sy.var.type)
	    xcpp (paramlist -> fxf.dyna -> sy.var.type -> tyf.int_form);
	else
	    comperr ("-- no type for parameter\n");
	if (paramlist -> fx.var.param_is_out)
	    fputf (outfile, " *");
	else
	    fputf (outfile, " ");
	fputf (outfile, "%s;\n", paramlist -> fxf.name);
	paramlist = paramlist -> fx.var.param_link;
    }

    if (p -> nd.subp.bodyx) {	/* if subp has a body part (isn`t a stub) 
				*/
	fputf (outfile, "{");
	pp_new_line ();
	inby (2);
	xcpp (p -> nd.subp.bodyx);/* generate statements */
	outby (2);
	indent (in);
	fputf (outfile, "} /* end %s */", p -> nd.subp.name);
    }
    else
	fputf (outfile, ";");	/* forward stub */
    pp_new_line ();
    currtab _ savetab;
  }  /* end else clause of if(inspec) */
}


comp_attribute(p)		/* attributes that aren't call forms 
				   like 'first, 'last, 'range, ... */
nodep p;
{	char *id _ p->nd.selectx.var->nd.ident.id;
	char *what _ p->nd.selectx.field;
	fputf(outfile, "%s->", id);
	if (what eq intern("first")) fputf(outfile,"arr_first");
	else if (what eq intern("last")) fputf(outfile, "arr_last");
	else comperr("Can't hack attribute %s", what);
}

comp_expression(p)              /* emit code for an expression */
nodep p;
{
        switch(p->ndf.op) {
                case nop_ident:                
                        comp_ident_reference(p);
                        break;
                case nop_int:
                        comp_integer(p);
                        break;
                case nop_char:
                        comp_character(p);
                        break;
                case nop_real:
                        comp_real(p);
                        break;
                case nop_string:               
                        comp_strconst(p);
                        break;
                case nop_lp:
                        comp_paren_expr(p);
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
                        comp_boolean_op(p);
                        break;
                case nop_neg:
                case nop_pos:
                case nop_not:
                        comp_unary_op(p);
                        break;
                case nop_call:                 
                        comp_call_form(p);
                        break;
                case nop_in:
                        comp_in_range_test(p);
                        break;
		case nop_quote:	/* attribute like a'first */
			comp_attribute(p);
			break;
                default: comperr("Node %s is not an expression node", 
                                  gennode(p));
        } /* end switch */
}

                                                                  
