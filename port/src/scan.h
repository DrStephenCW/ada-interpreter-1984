/* $Header:   /usr/ics/darpa/ada/RCS/scan.h.v  Revision 1.4  83/03/10  16:07:48  ada  Exp$ */

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


/* $Log:	/usr/ics/darpa/ada/RCS/scan.h.v $
Revision 1.4  83/03/10  16:07:48  ada
same as 1.3.2.1

Revision 1.3.2.1  83/03/10  14:57:43  willson
fixed boolean (ought to be removed)

Revision 1.2  83/01/12  09:04:31  ada
Added copyright notice.

Revision 1.1  82/12/12  21:37:55  tadman
Initial revision (Arcturus release 1.3E)
 */

# define true 1
# define false 0

typedef union {
    struct {
	char *val;
	boolean flag;		/* for designator */
	nodep attach; 
    } ident;

    struct {
	int  val;
	char *prval;
	nodep attach;
    }  integer;

    struct {
	float val;
	char *prval;
	nodep attach;
    }  real;
   
    struct {
	char val;
	nodep attach;
    } character;

    nodep nodepointer; 

    struct { nodep p, attach; } nodepointer1;

    struct { nodep p1, p2, attach; } nodepointer2;

    struct {
	enum tokentypes token;
	nodep attach;
    } token;

    struct {
	boolean in, out;
	nodep attach;
    } param_mode; 

}  YYSTYPE;

