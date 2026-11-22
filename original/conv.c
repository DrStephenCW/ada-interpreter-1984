static char rcsid[] =
"$Header:   ../RCS/conv.v  Revision 1.2  83/01/12  09:12:57  ada  Exp$";

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


/* $Log:	../RCS/conv.v $
 * Revision 1.2  83/01/12  09:12:57  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:21:17  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include "macros.h"

/*	Numeric conversion routines			*/	

/* 	This routines are used by the scanner to convert numbers  	*/
/*	to internal form.  Since the scanner has already roughly  	*/
/*	validated the format of the number, these routines only	  	*/
/*	check for things which are not checked for by the scanner 	*/
/*	They are 1) number too large or small to represent and	  	*/
/*	2) invalid digit in specified base.				*/  
/*	note: this preliminary version doesn't even do these checks 	*/
/*	Further error checking will be required if these are to be	*/
/*	used as part of an I/O package					*/

int aatoi(s)		/* ada version of atoi (ascii to integer) */
    char *s;
{
    int n = 0, e = 0;

    while ((*s >= '0' && *s <= '9') || *s == '_') {
	if ( *s != '_')  n = 10 * n + *s - '0';   /* check for overflow */
	s++;
    }
    if (*s == 'E' || *s == 'e') {
	s++;
	while ((*s >= '0' && *s <= '9') || *s == '_') {
	    if (*s != '_') e = 10 * e + *s - '0';
	    s++;
	}
	while (e-- > 0)
	    n = 10 * n;			/* check for overflow */
    }
    return n;
}

int abatoi(s)	/* converts an ada based integer (in ascii) to an integer */
    char *s;
{
    int b = 0, n = 0, e = 0;

    while ((*s >= '0' && *s <= '9') || *s == '_') {
	if ( *s != '_')  b = 10 * b + *s - '0';   /* check for overflow */
	s++;
    }
    s++;	/* skip the # */
    while ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F') ||
	   (*s >= 'a' && *s <= 'f') || *s == '_') {
	if ( *s != '_')  
	    n = b * n +    		/* check for overflow */
		(*s >= '0' && *s <= '9' ? *s - '0' :
		 *s >= 'A' && *s <= 'F' ? *s - 'A' + 10 :
		 *s - 'a' + 10);
	s++;
    }
    s++;	/* skip the # */
    if (*s == 'E' || *s == 'e') {
	s++;
	while ((*s >= '0' && *s <= '9') || *s == '_') {
	    if (*s != '_') e = 10 * e + *s - '0';
	    s++;
 	}
	while (e-- > 0)
	    n = b * n;			/* check for overflow */
    }
    return n;
}

float aatof(s)	/* ada version of atof (ascii to float) */
    char *s;
{
    float n = 0.0, f = 0.0;
    int e = 0;
    boolean negexp = false;

    while ((*s >= '0' && *s <= '9') || *s == '_') {
	if ( *s != '_')  n = 10.0 * n + *s - '0';   /* check for overflow */
	s++;
    }
    if (*s == '.') {
	s++;		/* skip the decimal point */
	f = 0.1;
	while ((*s >= '0' && *s <= '9') || *s == '_') {
	    if (*s != '_') {
		n = n + f * (*s - '0');
	    	f = f / 10.0;
	    }
	    s++;
	}
    }
    if (*s == 'E' || *s == 'e') {
	s++;
	if (*s == '-') { s++; negexp = true; }
	else if (*s == '+') s++;
	while ((*s >= '0' && *s <= '9') || *s == '_') {
	    if (*s != '_') e = 10 * e + *s - '0';
	    s++;
	}
	if (negexp)
	    while (e-- > 0)
		n = n / 10.0;
	else
	    while (e-- > 0)
	        n = 10.0 * n;	/* check for overflow */
    }
    return n;
}

float abatof(s)	/* converts an ada based float (in ascii) to a float */
    char *s;
{
    float n = 0.0, f = 0.0;
    int b = 0, e = 0;
    boolean negexp = false;

    while ((*s >= '0' && *s <= '9') || *s == '_') {
	if ( *s != '_')  b = 10 * b + *s - '0';   /* check for overflow */
	s++;
    }
    s++;	/* skip the # */
    while ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F') ||
	   (*s >= 'a' && *s <= 'f') || *s == '_') {
	if ( *s != '_')  
	    n = b * n +    		/* check for overflow */
		(*s >= '0' && *s <= '9' ? *s - '0' :
		 *s >= 'A' && *s <= 'F' ? *s - 'A' + 10 :
		 *s - 'a' + 10);
	s++;
    }
    if (*s == '.') {
	s++;		/* skip the decimal point */
	f = 1.0/b;
        while ((*s >= '0' && *s <= '9') || (*s >= 'A' && *s <= 'F') ||
	       (*s >= 'a' && *s <= 'f') || *s == '_') {
	    if (*s != '_') {
	        n = n + f *    		/* check for overflow */
		    (*s >= '0' && *s <= '9' ? *s - '0' :
		     *s >= 'A' && *s <= 'F' ? *s - 'A' + 10 :
		     *s - 'a' + 10);
	    	f = f / b;
	    }
	    s++;
	}
    }
    s++;	/* skip the # */
    if (*s == 'E' || *s == 'e') {
	s++;
	if (*s == '-') { s++;  negexp = true; }
	else if (*s == '+') s++;
	while ((*s >= '0' && *s <= '9') || *s == '_') {
	    if (*s != '_') e = 10 * e + *s - '0';
	    s++;
	}
	if (negexp)
	    while (e-- > 0)
		n = n / b;
	else
	    while (e-- > 0)
	        n = b * n;			/* check for overflow */
    }
    return n;
}

