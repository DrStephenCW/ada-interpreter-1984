/* $Header:   /usr/ics/darpa/ada/RCS/scan.h.v  Revision 1.4  83/03/10  16:07:48  ada  Exp$ */

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

