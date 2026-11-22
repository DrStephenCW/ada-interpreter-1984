/* $Header:   ../RCS/parser.h.v  Revision 1.2  83/01/12  09:04:24  ada  Exp$ */

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


/* $Log:	../RCS/parser.h.v $
Revision 1.2  83/01/12  09:04:24  ada
Added copyright notice.

Revision 1.1  82/12/12  21:37:09  tadman
Initial revision (Arcturus release 1.3E)
 */

    /* 	This enumeration type defines the various types of constructs which
	can be accepted by the parser. */

enum parser_construct {
	interactive_input,	/* input valid at the top level of Ada */
	expression,
	name,
	declarative_part,
	sequence_of_statements,
	type_definition,
	subtype_indication,
	compilation
};
