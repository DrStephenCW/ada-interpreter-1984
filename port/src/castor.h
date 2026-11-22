/* $Header:   ../RCS/castor.h.v  Revision 1.2  83/01/12  09:03:32  ada  Exp$ */

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


/* $Log:	../RCS/castor.h.v $
Revision 1.2  83/01/12  09:03:32  ada
Added copyright notice.

Revision 1.1  82/12/12  21:31:03  tadman
Initial revision (Arcturus release 1.3E)
 */

  /* castor_class is an enumeration of the syntatic classes which can
     be used for macro substitution in Castor.  After the parser produces
     a tree, it must be classified into one of these types so that it
     can be checked against the type of the corresponding substitution
     point when it is referenced */

typedef enum  {
	cas_unkn,		/* unknown (illegal for macro substitution) */
	cas_stmt,		/* statement */
	cas_decl,		/* declaration */
	cas_name,		/* name */
	cas_type,		/* type */
} castor_class;

