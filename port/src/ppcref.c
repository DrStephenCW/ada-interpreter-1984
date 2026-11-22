static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/ppcref.v  Revision 1.4  83/02/04  14:57:01  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/ppcref.v $
 * Revision 1.4  83/02/04  14:57:01  ada
 * same as 1.3.4.1
 * 
 * Revision 1.3.4.1  83/02/04  13:01:23  tadman
 * Types were prettyprinting with the terminating semicolon on the
 * seperate line.  The enf argument to prprint has been corrected.
 * 
 * Revision 1.3  83/01/12  09:08:45  ada
 * Added copyright notice.
 * 
 * Revision 1.2  83/01/04  14:23:45  willson
 * Added code to support extra pp parameter "export".
 * Changed bktr to not backtrace the bktr command.
 * Changed a call to pr_print which had four extra (ignored)
 * arguments.
 * 
 * Revision 1.1  82/12/12  21:41:48  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "arr.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "adaio.h"
#include "count.h"
#include "pp.h"

extern  nodep intstk[],
       *intsp;

enum { fcur, ffile, fpipe } ftype;

bktr (levels)			/* backtrace of all forms on interpreter
				   stack */
int     levels;
{
    nodep * sp = intsp - 1;	/* skip bktr(n); procedure call */
    int     depth = intsp - intstk;

    while (levels-- > 0 && sp >= intstk) {
	putf ("%4d.  ", depth--);
	tracepp (*sp, (sp == intsp ? null : *(sp + 1)), current_output ());
	sp--;
    }
}

FILE *popen();		/* open a pipe */

static FILE *open_file(fname, nroff)
    char *fname;
    boolean nroff;
{
    char    cmd[80];
    FILE *f;

    if (nroff) {
	if (fname[0])
	    sprintf (cmd, "nroff -Ttn300 >%s", fname);
	else
	    sprintf (cmd, "nroff | ul");
	f = popen (cmd, "w");
	ftype = fpipe;
    }
    else
	if (fname[0]) {
	    f = fopen (fname, "w");
	    ftype = ffile;
	}
	else {
	    f = (FILE *)current_output ();
	    ftype = fcur;
	}
    return f;
}

static close_file(f)
    FILE *f;
{
    switch (ftype) {
	case ffile: 
	    fclose(f);
	    break;
	case fpipe:
	    pclose(f);
	    break;
	case fcur:
	    break;
    }
}

sympp(fix, ada_fname, nroff, vert_bars, comments, attachments,
    width, attach_column, color, identifiers, reserved, number,
    indent, expt)   /* pretty printer */
    register fixp fix;
    arrayp ada_fname;
    boolean nroff, vert_bars, comments, attachments, number, expt;
    int width, attach_column, indent;
    enum pp_color_use color;
    enum pp_capitalize identifiers, reserved;
{
    char    fname[256];
    FILE    *f;

    str_ada_to_c (fname, ada_fname);
    if (fname[0] != 0 && color != ppc_none)
	error("Frank says to tell you that we don't have color files here!");
    f = open_file(fname, nroff);
    switch (fix -> fxf.class) {
	case func_id: 
	case proc_id: 
	case macro_id:			/* Castor */
	    if (fix -> fx.func.int_form)
		prprint(fix->fx.func.int_form, f, nroff, vert_bars,
			comments, attachments, width, attach_column,
			true, color, false,
			identifiers, reserved, number, indent, expt);
	    else
	        error ("No internal form for func, proc, or macro");
	    break;
	case type_id: 
	case subtype_id: 
	    if (fix -> fx.var.int_form) {
		fputf (f, "%s %s is ",
			fix -> fxf.class == type_id ?
			"type" : "subtype",
			fix -> fxf.name);
   	        prprint(fix->fx.var.int_form, f, nroff, vert_bars,
			comments, attachments, width, attach_column, 
			false, color, false,
			identifiers, reserved, number, indent, expt);
		putf (";\n");
	    }
	    else
		error ("No internal form for type (incomplete type)");
	    break;
	case obj_id: 
	case except_id:
	    fdirsym(f, fix);
	    fputf(f, "\n");
	    break;
	case pack_id: 
 	    if (fix -> fx.pack.int_form) {
   	        prprint(fix->fx.pack.int_form, f, nroff, vert_bars,
			comments, attachments, width, attach_column, 
			true, color, false,
			identifiers, reserved, number, indent, expt);
		if (fix -> fx.pack.body_int_form) {
		    fputf(f, "\n");
		    prprint(fix->fx.pack.body_int_form, f, nroff, vert_bars,
			comments, attachments, width, attach_column, 
			true, color, false,
			identifiers, reserved, number, indent, expt);
		}
	    }
	    else
		error ("no package specification");
	    break;
	default: 
	    error ("can't print this class");
    }
    close_file(f);
}

symifp (fix, ada_fname, nroff, vert_bars, comments,
	attachments, width, attach_column)	/* internal form printer */
    fixp fix;
    arrayp ada_fname;
    boolean vert_bars, comments, attachments;
    int width, attach_column;
{
    char    fname[256];
    FILE    *f;

    str_ada_to_c (fname, ada_fname);
    f = open_file(fname, nroff);

    switch (fix -> fxf.class) {
	case func_id: 
	case proc_id: 
	case macro_id:			/* Castor */
	    if (fix -> fx.func.int_form)
		ifp (fix -> fx.func.int_form, f, nroff, vert_bars,
			comments, attachments, width, attach_column);
	    else
	    	error ("No internal form for func, proc, or macro");
	    break;
	case type_id: 
	case subtype_id: 
	    if (fix -> fx.var.int_form) {
		fputf (f, "[%s %s is ",
			fix -> fxf.class == type_id ?
			"type" : "subtype",
			fix -> fxf.name);
   	        ifp (fix->fx.var.int_form, f, nroff, vert_bars,
			comments, attachments, width, attach_column);
		putf ("]\n");
	    }
	    else
		error ("No internal form for type (incomplete type)");
	    break;
	case pack_id: 
 	    if (fix -> fx.pack.int_form) {
   	        ifp(fix->fx.pack.int_form, f, nroff, vert_bars,
			comments, attachments, width, attach_column);
		if (fix -> fx.pack.body_int_form) {
		    fputf(f, "\n");
		    ifp(fix->fx.pack.body_int_form, f, nroff, vert_bars,
			comments, attachments, width, attach_column);
		}
	    }
	    else
		error ("no package specification");
	    close_file(f);
	    break;
	default: 
	    error ("can't ifp this class");
    }
}

symcref (fix, ada_fname, nroff)		/* cross reference */
    fixp fix;
    arrayp ada_fname;
    boolean nroff;
{
    char    fname[256];
    FILE    *f;

    str_ada_to_c (fname, ada_fname);
    f = open_file(fname, nroff);
    switch (fix -> fxf.class) {
	case func_id: 
	case proc_id: 
	case macro_id:			/* Castor */
	    if (fix -> fx.func.int_form)
		docref (fix -> fx.func.int_form, f);
	    else
	    	error ("No internal form for func, proc, or macro");
	    break;
	case type_id: 
	case subtype_id: 
	    error ("can't cref a type");
	    break;
	case obj_id: 
	    error ("can't cref an object");
	    break;
	case pack_id: 
  	    if (fix -> fx.pack.int_form) {
   	        docref(fix->fx.pack.int_form, f, nroff);
		if (fix -> fx.pack.body_int_form) {
		    fputf(f, "\n");
		    docref(fix->fx.pack.body_int_form, f, nroff);
		}
	    }
	    else
		error ("no package specification");
	    close_file(f);

	default: 
	    error ("can't cref this class");
    }
}

ada_pp(p, ada_fname, nroff, vert_bars, comments, attachments,
    width, attach_column, color, identifiers, reserved,
    number, indent, expt)   /* pretty printer */
    register nodep p;
    arrayp ada_fname;
    boolean nroff, vert_bars, comments, attachments, number, expt;
    int     width, attach_column, indent;
    enum pp_color_use color;
    enum pp_capitalize identifiers, reserved;
{
    char    fname[256];
    FILE    *f;

    str_ada_to_c (fname, ada_fname);
    if (fname[0] != 0 && color != ppc_none)
	error("we don't have color files here");
    f = open_file(fname, nroff);
    prprint(p, f, nroff, vert_bars,
	comments, attachments, width, attach_column,
	true, color, true,
	identifiers, reserved, number, indent, expt);
    close_file(f);
}

ada_ifp(p, ada_fname, nroff, vert_bars, comments,
	attachments, width, attach_column)	/* internal form printer */
    nodep p;
    arrayp ada_fname;
    boolean vert_bars, comments, attachments;
    int width, attach_column;
{
    char    fname[256];
    FILE    *f;

    str_ada_to_c (fname, ada_fname);
    f = open_file(fname, nroff);

    ifp(p, f, nroff, vert_bars,
	comments, attachments, width, attach_column);
}

symdir(s, ada_fname, color)
    register symp s;
    arrayp ada_fname;
    enum pp_color_use color;
{
    char    fname[256];
    FILE    *f;

    str_ada_to_c (fname, ada_fname);
    if (fname[0] != 0 && color != ppc_none)
	error("we don't have color files here");
    f = open_file(fname, false);
    
    dir(s,f,color);

    close_file(f);
}
