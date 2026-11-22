static char rcsid[] =
"$Header:   ../RCS/parerr.v  Revision 1.2  83/01/12  09:07:47  ada  Exp$";

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


/* $Log:	../RCS/parerr.v $
 * Revision 1.2  83/01/12  09:07:47  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:40:04  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

/* Error handling functions for the parser */

# include <stdio.h>
# include "macros.h"
# include "oblist.h"

# define YYLAST 1884		/* kludge for now */

extern char *parstok[];
extern int yychar, yystate;

char *perrtok[100], **erp = perrtok;	/* probably much too large */
 
/* yacc tables */

extern short yyexca[], yyact[], yypact[], yypgo[], yyr1[], yyr2[];
extern short yychk[], yydef[];


char *tokstring(token)
    int token;
{
    char tempstring[2];

    if (token == 0) return "EOF";
    if (token < 257) {
	tempstring[0] = token;
	tempstring[1] = '\0';
	return strsav(tempstring);
    }
    return parstok[token - 257];
}

vlookahead(yystate)		/* Computes the valid lookahead set when */
    int yystate;		/* an error occurs */
{
    int m, n, ch;

    erp = perrtok;
    if (yydef[yystate] != -2) {
	n = yypact[yystate];
	for (ch = 0; ch < 500; ch++) {	/* determine last token */
	    m = ch + n;
	    if (m >= 0 && m < YYLAST && yychk[yyact[m]] == ch)
		*erp++ = tokstring(ch);
	}
   }
# ifdef DEBUG
   else	sysputf("Exception table failure.\n");	/* leave for debugging */
# endif DEBUG
    *erp = null;
# ifdef DEBUG
    parser_help(current_output());
# endif DEBUG
}


parser_help(f)
    FILE *f;
{
    int     col = 1;

    erp = perrtok;
    while (*erp != null) {
	fputf (f, "%-12s", *erp++);
	if (col++ == 6) {
	    col = 1;
	    fputf(f, "\n");
	}
    }
    if (col > 1)
	fputf (f, "\n");

}

write_help_file()
{
    FILE *f;
    
    f = fopen(",ADA.help", "w");
    if (f != null) {
        parser_help(f);
        fclose(f);
    }
}
