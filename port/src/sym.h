/* $Header:   /usr/ics/darpa/ada/RCS/sym.h.v  Revision 1.6  83/03/10  16:18:10  ada  Exp$ */

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


/* $Log:	/usr/ics/darpa/ada/RCS/sym.h.v $
Revision 1.6  83/03/10  16:18:10  ada
same as 1.4.2.1

Revision 1.4.2.1  83/03/10  15:29:32  willson
switched to ada 82 i/o

Revision 1.3  83/01/12  09:04:37  ada
Added copyright notice.

Revision 1.2  83/01/11  15:38:47  ada
Same as 1.1.2.1.

Revision 1.1.2.1  83/01/11  15:32:07  ada
Changed SETUP
NAME and SWAPNAME to be ./name

Revision 1.1  82/12/12  21:38:48  tadman
Initial revision (Arcturus release 1.3E)
 */

typedef struct symbolform *symp;
typedef struct typeform *typep;
typedef struct useform *usep;

 /* Entries in the symbol table are one of the following types: */

enum symbolclasses { 
        undef_id,       /* undefined (deleted) variable */
        obj_id,         /* object variable */
        enum_id,        /* enumeration constant */
        param_id,       /* parameter */
        func_id,        /* function */
        proc_id,        /* procedure */
	macro_id,	/* macro  (Castor) */
        type_id,        /* type identifier */
        subtype_id,     /* subtype identifier */
        field_id,       /* field identifier (in a record) */
        pack_id,        /* package identifier */
	except_id	/* exception identifier */
};

/*	The symbols are stored in hash tables; one hash table per level
	of static scoping (except for packages, where there is one table
	for the visible part and one for the implementation part).
	After hashing on the identifier for the symbol, one gets a pointer
	to a list of "fixp's", which point to the fixed part of symbol
	entries.  The fixp's are linked together to form a bucket for
	that part of the hash table.

	The fixed information in a symbol is as follows:
*/

typedef struct fixform *fixp;

struct fixform {
	/* every symbol contains the following information */
    struct FXF {
        enum symbolclasses class;	/* the type of symbol */
        char    *name;			/* the identifier associated */
	fixp next;		/* next one in this bucket */
	nodep ag;		/* pointer to attachment for symbol */
	symp dyna;		/* pointer to dynamic data */	
	symtabp static_link;	/* points to this particular symbol's
				   parent symbolform; so from any symbol
				   we can figure out a static link */
	boolean is_visible; 		/* not used; will mean that the symbol
					is part of the implementation part of
					a package if false; otherwise true */
    } fxf;

  /* depending on the type of symbol, it will contain one of the following: */

    union {
	struct varfixform {		/* object or parameter */
            struct typeform *type;	/* its type (integer, float, ...) */
            nodep int_form;              /* points to the internal form used to
					create this symbol entry */
            boolean is_const;		/* true if constant */
	    boolean is_enum_lit;	/* obvious (only used by "dir") */
 	    boolean param_is_in;	/* true if in parameter */
            boolean param_is_out;	/* true if out parameter */
            struct typeform *param_base_type; /* for unconstrained 
					array parameters; this is 
					unconstrained, sym_type isn't*/
            boolean param_has_value;	/* used to make sure all parameters
					get a value (default or given) */
			            /* used for overloading resolution */
            struct fixform *param_link; /* used when matching formals 
					to actual values */
        } var;

	struct exceptfixform {	/* exception identifier */
	    char   waste;
	} except;

        struct funcfixform {		/* function or procedure */
            struct typeform *type;  /* return type */           
            nodep   int_form;    /* pointer to internal form of decl's
				and body */
            long     (*code)();  /* pointer to compiled code */
        } func;

   	struct packfixform {	/* package */
            nodep int_form;		/* points to defining internal form 
					of specification */
            nodep body_int_form;	/* points to defining internal form
					of the package body */
        } pack;                                          
    } fx;
};

/*	A symbol contains the type of the symbol and information about
	the symbol (like its type and its current value).  This information
	is dynamic; that is, on each invocation of a (recursive) subprogram
	this information is subject to change.  So hanging off of each
	fixp is a list of symp's.  The head of this list is the most symbol
	entry for the most recent invocation of the subprogram.

	A symbol contains the following information:
*/

struct symbolform {
    /* every symbol contains the following information: */
    struct SYF {
        symp link;		/* a link to the previous invocation */
	long level;		/* nesting level in dynamic stack */
    } syf;

  /* depending on the type of symbol, it will contain one of the following: */
    union {
	struct varsymbolform {		/* object or parameter */
            struct typeform *type;	/* its type (integer, float, ...) */
            long    *value;              /* points to actual value, the size */
                                        /*  of which can vary - integer,    */

	    long value_space;	/* saves malloc for scalars */
				/* value is initialized to point */
				/* here by newsym1() */
        } var;

	struct exceptsymbolform {	/* exception identifier */
	    char   waste;
	} except;

        struct funcsymbolform {		/* function or procedure */
            struct typeform *type;  /* return type */           
	    symp    paramlist;	/* saved parameter list in static symbol
				table */
        } func;

   	struct packsymbolform {	/* package */
	    char waste;
        } pack;                                          
    } sy;
};

/*	newsym is a macro that should be used to allocate a new symbolform */

#define newsym(tag, class) \
	newsym1(sizeof(struct SYF)+sizeof(struct tag), class) 
#define newfix(tag, class) \
	newfix1(sizeof(struct SYF)+sizeof(struct tag), class) 

/*	The following list enumerates the possible types an object can have */

enum typeforms {undefform, 	/* incomplete type declaration */
		intform,	/* integer */
		realform, 	/* float */
		strconstform,	/* constant string (pack the way C packs 'em) */
		strform,	/* string (Ada style) */
                srangeform,	/* subrange constraint */
		urangeform,	/* subrange constraint (unevaluated) */
		enumform,	/* enumeration */
		con_arrayform,	/* constrained array */
		un_arrayform,	/* unconstrained array */
		recordform,	/* record */
                indexform,	/* array index */
		accessform,	/* access */
		subtypeform,	/* subtype (unconstrained) */
		derivedform,	/* derived */
};
                                                      
/*	A typep points to a typeform which contains the "class" of the type
	(one of the above), a pointer to the internal form that was used
	to define the type, and the size of objects of the type. Other
	information depends on the particular type.
*/

struct typeform {
    struct TYF {
        enum typeforms class;
        nodep int_form;
        long size;
    } tyf;

    union {
	struct srangetypeform {         
            long lbound;		/* lower bound of range */
            long ubound;		/* upper bound */
            typep basetype;	/* base type (character, integer,...) */
	} srange;
                             
	struct urangetypeform {         
            nodep lbound;		/* lower bound of range */
            nodep ubound;		/* upper bound */
            typep basetype;	/* base type (character, integer,...) */
	} urange;
                             
	struct enumtypeform {
            long lbound;		/* lower bound (always zero?) */
            long ubound;		/* number of elements in enumeration - 1 */
            char **names;	/* the names of the elements of the enum */
	} enumer;

	struct indextypeform {	/* list of array indices */
            typep range;	/* each one is a range */
            typep next;		/* pointer to next index */
            boolean not_constrained;	/* ?? */
        } index;
                             
	struct un_arraytypeform {          
            typep index_list;	/* a list(?) of types for each index (w/o
				constraints) */
            typep ele_type;	/* type of the elements in the array */
	} un_array;
                             
	struct con_arraytypeform {          
            typep index_list;	/* ?? */
            typep base_type;	/* ?? */
        } con_array;
                             
	struct recordtypeform {
            struct symtabform *fieldtab;
				/* a record has a symbol table hanging off
				it; the value of each symbol is the offset
				of that field of the record */
	} record;

	struct accesstypeform {
            typep targettype;	/* type of the object to access */
	} access;

	struct subtypetypeform {
	    typep basetype;	/* type of the base type.  subtypetypeform
				   is used for unconstrained subtypes
				   only.  For constrained subtypes, a
				   srangetypeform is used. */
  	} subtype;

	struct derivedtypeform {
	    typep parenttype;	/* the parent type of the derived type */
	} derived;
    } ty;
};

/* this should be fixed to work the way newsym does..... */
union maxtypeform {           /* for computing size of largest type struct */
        struct srangetypeform maxtype1;
        struct enumtypeform maxtype2;
        struct un_arraytypeform maxtype3;
        struct con_arraytypeform maxtype4;
        struct recordtypeform maxtype5;
        struct accesstypeform maxtype6;
	struct subtypetypeform maxtype7;
	struct derivedtypeform maxtype8;
};

#define maxtypesize (sizeof(struct TYF)+sizeof (union maxtypeform))

/* 	Certain type pointers must be known to the interpreter.  These
	are stored in the following variables: */

typep   inttype,		/* integer */
	realtype,		/* float */
	booltype,		/* boolean */
	strtype,		/* string */
	undeftype,		/* incomplete type definition */
        chartype,		/* character */
	nulltype,		/* access to null pointer; compatible with
				all access types */
        texttype,		/* text file */
	filetype,		/* file descriptor */

/*	The rest of these "types" only make sense to the interpreter. 
	When passing around "typedval's" in the interpreter we sometimes
	need to indicate when the value being returned is one of these: 
*/
	functype,		/* function or procedure */
	packtype,		/* package */
        typetype,		/* type */
	excepttype,		/* exception */
	symboltype;		/* symbol table entry */

/*	nifty forward declarations: */

fixp newfix1();			/* allocate new fixform */
symp newsym1();			/* allocate new symbolform */
fixp lookup();			/* lookup identifier in symbol table */
fixp lookuptab();		/* lookup in specific table */
typep newtype();		/* allocate space for type descriptor */
typep lookuptypeid();		/* lookup a type descriptor given an id */
typep base();			/* return the base type of the type given */


/*	These two files are read in when the system starts up.  SWAPNAME
	is written by Arcturus, and SETUPNAME is written by the user.
	They are named ./file to trick the include mechansim into NOT
	looking into the Library if they're not found locally.
*/

#define SWAPNAME "./ADA.SAVE"
#define SETUPNAME "./ADA.SETUP"
