/* $Header:   ../RCS/tab.h.v  Revision 1.2  83/01/12  09:04:45  ada  Exp$ */

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


/* $Log:	../RCS/tab.h.v $
Revision 1.2  83/01/12  09:04:45  ada
Added copyright notice.

Revision 1.1  82/12/12  21:39:19  tadman
Initial revision (Arcturus release 1.3E)
 */

struct symtabform {              
        int     symtab_size;
        fixp    *symtab_tab;
        fixp    symtab_owner;
	struct  symtabform *symtab_static_link;
	usep	use_list;
};

/* each table has a use-list which is a list of other tables */
/* the tables are searched by lookuptab if the search of the */
/* main table failed.  note that tables in use lists may     */
/* themselves have use lists				     */

struct useform {
    usep link;
    struct symtabform *table;
};

symtabp currtab;
symtabp usrtab;

symtabp newtab();
symtabp gettab();

