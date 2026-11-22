/* $Header:   /ub/darpa/ada/RCS/eval.h.v  Revision 1.3  83/09/08  16:03:59  ada  Exp$ */

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


/* $Log:	/ub/darpa/ada/RCS/eval.h.v $
Revision 1.3  83/09/08  16:03:59  ada
same as 1.2.9.1

Revision 1.2.9.1  83/08/25  14:34:12  ritter
Added entry in return stack so that exceptions that occur while already
inside of a handler are treated correctly

Revision 1.2  83/01/12  09:03:46  ada
Added copyright notice.

Revision 1.1  82/12/12  21:33:00  tadman
Initial revision (Arcturus release 1.3E)
 */

enum int_modes {        /* second arg to interp */
        eval_mode,      /* get value of variables (normal) */
        decl_mode,      /* ident nodes are being declared */
        addr_mode,      /* get address of variables */
        symbol_mode,    /* return symbol, no eval */
        ovl_mode,       /* eval as subprogam symbol or array address */
	macro_mode,	/* evaluate macro, but do not interpret result (Castor) */
};
             
typedef union {
	long *ptr;	/* used for indirecting through word */
	symp sptr;
	typep tptr;
	nodep nptr;
	fixp fptr;
        long integer;
        float real;
} word;

typedef struct {
    typep typ;
    word val;
} typedval;


 /* The return stack saves the state of certain variables for subprogram
    calls, and block and loop executions.  Making it explicit allows
    the implementation of exit statements and exception handling */

struct retstkform {
    long *retenv;		/* return environment for setjmp/longjmp */
    typedval *retvalsav;	/* pointer to result for return */
    nodep *intsp_save;		/* return stack pointer */
    symtabp currtab;		/* value of currtab upon entry */
    nodep p;			/* internal form of caller */
    fixp exception;		/* last exception handled on this level */
    long must_prop_exception;	/* true if exceptions must be propogated */
				/* PAR 22-Aug-83 */
    struct retstkform *link;	/* stack link */
};

typedef struct retstkform *retsp;

retsp retstk;

 /* the following are pointers to the predefined exceptions in package
    standard.  They are used as arguments to sysraise */

/* program error added by PAR 22-Aug-83 */

fixp constraint_error, numeric_error, storage_error, program_error;

typedval dummy;

#define throw longjmp 

typedval *retval;               /* global used by return to return current */
                                /* type and value */

#define MAXARGS 20L
