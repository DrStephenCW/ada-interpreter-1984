static char rcsid[] =
"$Header:   ../RCS/intform.v  Revision 1.2  83/01/12  09:16:52  ada  Exp$";

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


/* $Log:	../RCS/intform.v $
 * Revision 1.2  83/01/12  09:16:52  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:29:41  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include <stdio.h>
# include "macros.h"
# include "arr.h"
# include "nodes.h"
# include "int.h"
# include "sym.h"
# include "tab.h"
# include "parser.h"
/* port: block-scope externs, repeated at file scope */
extern nill();
extern nodep maptree();
extern nodep maptreelist();

extern char *adaread(), *parsething();

extern int sizetab[];		/* size of each type of node indexed by
				   nop_?? type */

nodep ada_parse(ada_str,construct) /* ada callable function to parse
				      a string and return a pointer to the
				      internal form */
    arrayp ada_str;
    enum parser_construct construct;
{
    char ss[2000], *s = ss;
    char *err_mess;
    nodep p;

    str_ada_to_c(s, ada_str);
    err_mess = parsething(s,&p,construct);
    if (err_mess) error("parse: %s", err_mess);
    if (p->ndf.op == nop_expr) {	/* if top level expression
					   remove the root */
	nodep q = p;							
	p = p->nd.expr.expr;
	free(q);
    }
    return p;
}

ada_eval(p)
    nodep p;
{
    interp(p);
}

nodep ada_parse_file(fname)	/* parse the given file and
				   and return a pointer to the
				   internal form */
    char *fname;
{
    char *err_mess;
    nodep p;

    err_mess = adaread(fname, &p, interactive_input, true);
    if (err_mess) sysputf("parse_file: %s", err_mess);
    return p;
}

nodep int_form(f)
    fixp f;
{
   switch (f -> fxf.class) {
	case func_id: 
	case proc_id: 
	case macro_id:			/* Castor */
	    return f->fx.func.int_form;
	case type_id: 
	case subtype_id: 
	case obj_id: 
	case except_id:
	    return f->fx.var.int_form;
	case pack_id: 
	    return f->fx.pack.int_form;
	default: 
	    return null;
    }
}

nodep body_int_form(f)
    fixp f;
{
    if (f->fxf.class == pack_id)  return f->fx.pack.body_int_form;
    else return null;
}

  /* ada_copytree copies a node of the internal form.  It does not
     traverse the ndf.next link of the root.  The ndf.next link of the root
     of the copy is set to null to prevent multiple pointers to a single
     node.

     ada_copytreelist does traverse the ndf.next link of the root.  It
     thus copies a list of items.		*/

static pre_copy_tree(n, p)
    nodep n, *p;
{
    register int i;
    register char *m, *nn;
    int size;
        
    if (n) {
	size = sizetab[(int)n->ndf.op];   /* get a node for copy */
	m = (char *) (*p =(nodep)zalloc(size,false));
	nn = (char *)n;
        for (i = 0; i < size; i++)  *m++ = *nn++; /* copy the node */
    }
}	

nodep ada_copytree(t)
    nodep t;
{
    nodep p;
    extern nill();
    extern nodep maptree();

    p = maptree(t, pre_copy_tree, nill, nill, nill);
    p->ndf.next = null;
    return p;
}

nodep ada_copytreelist(t)
    nodep t;
{
    extern nill();
    extern nodep maptreelist();
    return maptreelist(t, pre_copy_tree, nill, nill, nill);
}

 /* The function free_tree(t) frees the storage for the internal form
    tree pointed to by t.  It does traverse the ndf.next field of the
    parent, so care must be taken to set this to null if the tree has
    been spliced out of a list */

static free_attach(n)	/*  frees an attachment, and recursively
			    attachments of attachments, etc. */
    nodep n;
{
    if (n) {
        apply_all_attach(n, free_attach);
	free(n);
    }
}

static free_node(n)
    nodep n;
{
    apply_all_attach(n, free_attach);
    free_attach(n);
    free(n);
}

free_tree(r)
    nodep r;
{
    maptreelist(r, nill, free_node, nill, nill);
}


