static char rcsid[] =
"$Header:   ../RCS/rsw.v  Revision 1.2  83/01/12  09:09:00  ada  Exp$";

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


/* $Log:	../RCS/rsw.v $
 * Revision 1.2  83/01/12  09:09:00  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:42:46  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include <stdio.h>
#include "macros.h"
# include "tokens.h"

struct {
	char *rsw_string;
	long rsw_tok;
} rswtab[] = {
	"abort",	ABORT,
	"accept",	ACCEPT,
	"access",	ACCESS,
	"all",		ALL,
	"and",		AND,
	"array",	ARRAY,
	"at",		AT,
	"begin",	XBEGIN,
	"body",		BODY,
	"case",		CASE,
	"constant",	CONSTANT,
	"declare",	DECLARE,
	"delay",	DELAY,
	"delta",	DELTA,
	"digits",	DIGITS,
	"do",		DO,
	"else",		ELSE,
	"elsif",	ELSIF,
	"end",		END,
	"entry",	ENTRY,
	"exception",	EXCEPTION,
	"exit",		EXIT,
	"for",		FOR,
	"function",	FUNCTION,
	"generic",	GENERIC,
	"goto",		GOTO,
	"if",		IF,
	"in",		IN,
	"is",		IS,
	"limited",	LIMITED,
	"loop",		LOOP,
	"macro",	MACRO,
	"mod",		MOD,
	"new",		NEW,
	"not",		NOT,
	"null",		XNULL,
	"of",		OF,
	"or",		OR,
	"others",	OTHERS,
	"out",		OUT,
	"package",	PACKAGE,
	"pragma",	PRAGMA,
	"private",	PRIVATE,
	"procedure",	PROCEDURE,
	"raise",	RAISE,
	"range",	RANGE,
	"record",	RECORD,
	"rem",		REM,
	"renames",	RENAMES,
	"return",	RETURN,
	"reverse",	REVERSE,
	"select",	SELECT,
	"separate",	SEPARATE,
	"subtype",	SUBTYPE,
	"task",		TASK,
	"terminate",	TERMINATE,
	"then",		THEN,
	"type",		TYPE,
	"use",		USE,
	"when",		WHEN,
	"while",	WHILE,
	"with",		WITH,
	"xor",		XOR,
};

#define numrsw (sizeof(rswtab) / sizeof(*rswtab))


long rswcheck(s)
    char *s;
{
	register long i;
	register long r = numrsw;
	register long l = 0;
	long c;

	while(1) {
		i = (l+r) / 2;
		if(i==l) return(Identifier);
		c = strcmp(s, rswtab[i].rsw_string);
		if(c==0)
			return(rswtab[i].rsw_tok);
		if (c<0) r=i;
		else l=i;
	}
}

char *optab[] = { "*", "+", "-", "/", "<", "=", ">",
	            "**", "<=", ">=", "or", 
		    "and", "mod", "not", "rem", "xor", null };

long chkopr(s)
    char *s;
{
    long l;
    char **t;

    if ((l = strlen(s)) <= 3 && l >= 1)
        for (t = optab; *t != null; t++)
		if (strcmp(s, *t) == 0) return Operator_String;
    return String;
}	     
