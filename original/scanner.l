a	[Aa]
d	[Dd]
e	[Ee]
m	[Mm]
n	[Nn]
o	[Oo]
r	[Rr]
t	[Tt]
x	[Xx]

int	[0-9](_?[0-9])*
bint	[0-9A-Fa-f](_?[0-9A-Fa-f])*

	/* global definitions */
%{
# include <stdio.h>
# include <ctype.h>
# include "macros.h"
# include "nodes.h"
# include "int.h"
# include "tokens.h"
# include "scan.h"
# include "oblist.h"

# undef input
# undef unput
# ifdef DEBUG
#   define input()							\
	(((yytchar = yysptr > yysbuf ? U(*--yysptr) :			\
		getc(yyin))==10 ? (yycol = 1, yylineno++, yytchar) :	\
			(yycol++, yytchar))				\
	== EOF ? 0 : (putc(yytchar,stdout),yytchar))
# else
#   define input()							\
	(((yytchar = yysptr > yysbuf ? U(*--yysptr) :			\
		getc(yyin))==10 ? (yycol = 1, yylineno++, yytchar) :	\
			(yycol++, yytchar))				\
	== EOF ? 0 : yytchar)
# endif

# define unput(c) {				\
	yytchar = (c);				\
	yycol--;				\
	if (yytchar == '\n') yylineno--;	\
	*yysptr++=yytchar; }

extern int aatoi(), abatoi();
extern float aatof(), abatof();	/* aatof -- ada version of atof */
				/* which converts ascii to float */
				/* abatof -- same for based float */
char *strconv();
char *trans();
nodep comchk();

extern YYSTYPE yylval;
int	yycol = 1;
int     yyprline, yyprcol;	/* line and column of prevous token */
static boolean noclp, nocl = false;  /* true if no character literal allowed */
boolean in_call_form;		/* true if in calling form.  Causes all
				   keywords except begin and end to be
				   treated as identifiers */
%}

%%
%{
	yyprline = yylineno;
	yyprcol = yycol;
	noclp = nocl;
	nocl = false;
%}
{int}([Ee][+-]?{int})?			{ nocl = true;
					  yylval.integer.val = aatoi(yytext);
					  yylval.integer.prval = strsav(yytext);
					  yylval.integer.attach = comchk();
					  return(Integer);  }
{int}(\.{int})?([Ee][+-]?{int})?	{ nocl = true;
					  yylval.real.val = aatof(yytext); 
					  yylval.real.prval = strsav(yytext);
					  yylval.real.attach = comchk();
					  return(Real);	 }
{int}#{bint}#([Ee]{int})?		{ nocl = true;
					  yylval.integer.val = abatoi(yytext);
					  yylval.integer.prval = strsav(yytext);
					  yylval.integer.attach = comchk();
					  return(Integer);  }
{int}:{bint}:([Ee]{int})?		{ nocl = true;
					  trans(yytext,':','#');
					  yylval.integer.val = abatoi(yytext);
					  yylval.integer.prval = strsav(yytext);
					  yylval.integer.attach = comchk();
					  return(Integer);  }
{int}#{bint}(\.{bint})#([Ee][+-]?{int})?   { nocl = true;
					  yylval.real.val = abatof(yytext);
					  yylval.real.prval = strsav(yytext);
					  yylval.real.attach = comchk();
					  return(Real);  }
{int}:{bint}(\.{bint}):([Ee][+-]?{int})?   { nocl = true;
					  trans(yytext,':','#');
					  yylval.real.val = abatof(yytext);
					  yylval.real.prval = strsav(yytext);
					  yylval.real.attach = comchk();
					  return(Real);  }
'[ -~]'					{ if (noclp) REJECT; 
					  yylval.character.val = yytext[1];
					  yylval.character.attach = comchk();
					  return(Character);  }
\"[^"\n]*(\"\"[^"\n]*)*\"		{ yylval.ident.val =
						strsav(strconv(yytext));
					  yylval.ident.attach = comchk();
					  return(chkopr(yylval.ident.val)); }
\"[^"\n]*(\"\"[^"\n]*)*\n		perror("unterminated string");
%[^%"\n]*(%%[^%"\n]*)*%			{ yylval.ident.val =
						strsav(strconv(yytext));
					  yylval.ident.attach = comchk();
					  return(chkopr(yylval.ident.val)); }
%[^%"\n]*(%%[^%"\n]*)*\"		perror("%% delimited string contains double quote");
%[^%"\n]*(%%[^%"\n]*)*\n			perror("unterminated string");
"=>"					{ yylval.ident.attach = comchk();
					  return(RIGHT_ARROW); }
".."			 		{ yylval.ident.attach = comchk();
					  return(SUBRANGE_SYM); }
"**"			 		{ yylval.ident.attach = comchk();
					  return(EXPONENTIAL); }
":="			 		{ yylval.ident.attach = comchk();
					  return(ASSIGN_OP); }
"/="					{ yylval.ident.attach = comchk();
					  return(NEQ); }
">="					{ yylval.ident.attach = comchk();
					  return(GEQ); }
"<="					{ yylval.ident.attach = comchk();
					  return(LEQ); }
"<<"					{ yylval.ident.attach = comchk();
					  return(LEFT_LABEL_BRACKET); }
">>"					{ yylval.ident.attach = comchk();
					  return(RIGHT_LABEL_BRACKET); }
"<>"					{ yylval.ident.attach = comchk();
					  return(BOX); }
"&"					{ yylval.ident.attach = comchk();
					  return('&'); }
"'"					{ yylval.ident.attach = comchk();
					  return('\''); }
"("					{ yylval.ident.attach = comchk();
					  return('('); }
")"					{ nocl = true;
					  yylval.ident.attach = comchk();
					  return(')'); }
"["					{ yylval.ident.attach = comchk();
					  sysputf("warning: [ changed to (");
					  return('('); }
"]"					{ nocl = true;
					  yylval.ident.attach = comchk();
					  sysputf("warning: ] changed to )");
					  return(')'); }
"*"					{ yylval.ident.attach = comchk();
					  return('*'); }
"+"					{ yylval.ident.attach = comchk();
					  return('+'); }
","					{ yylval.ident.attach = comchk();
					  return(','); }
"-"					{ yylval.ident.attach = comchk();
					  return('-'); }
"."					{ yylval.ident.attach = comchk();
					  return('.'); }
"/"					{ yylval.ident.attach = comchk();
					  return('/'); }
":"					{ yylval.ident.attach = comchk();
					  return(':'); }
";"					{ yylval.ident.attach = comchk();
					  return(';'); }
"<"					{ yylval.ident.attach = comchk();
					  return('<'); }
"="					{ yylval.ident.attach = comchk();
					  return('='); }
">"					{ yylval.ident.attach = comchk();
					  return('>'); }
"/"					{ yylval.ident.attach = comchk();
					  return('/'); }
"|"|"!"					{ yylval.ident.attach = comchk();
					  return('|'); }
"{"					{ yylval.ident.attach = comchk();
					  return('{'); }
"}"					{ yylval.ident.attach = comchk();
					  return('}'); }
"$"					{ yylval.ident.attach = comchk();
					  return('$'); }  /* Castor */
"$$"					{ yylval.ident.attach = comchk();
					  return(DBL_DOLLAR); }
[A-Za-z_](_?[A-Za-z0-9])*		{ int idtype;
					  register char *y = yytext;

					  while (*y) {
						if (isupper(*y))
						    *y = *y + 'a' - 'A';
						y++;
					  }

					  idtype = rswcheck(yytext);
					  if (in_call_form &&
						idtype != XBEGIN &&
						idtype != END)
						idtype = Identifier;
					  if (idtype == Identifier) {
						yylval.ident.val =
						     intern(yytext);
						nocl = true;
					  }
					  yylval.ident.attach = comchk();
					  return(idtype); }
[ \t]					;
--.*\n					;
\n					;
.					perror(
					    "invalid character %s (%o)\n",
					    yytext, yytext[0]);
%%

scan_reset()  /* clears scanner look-ahead buffer */
{
    yysptr = yysbuf;
    yytchar = 0;
    yylineno = 1;
    yycol = 0;
    in_call_form = false;
}

char *trans(s,f,t)		/* replace all occurances of the character
				   f by the character t in string s */
    char *s, f, t;
{
    char *g, *p;
    int i;

    g = p = ++s;
    for (i = 1; i <= strlen(s); i++)
	if (s[i] == f) s[i] = t;
}

char *strconv(s)  char *s;		/* translates the external
					   representation of a string to its
					   internal value.  Strips off
					   delimiters and changes doubled
					   internal delimiters ("" and %%)
					   to a single occurance */
{
    char *g, *p, delim;

    delim = *s++;
    g = p = s;
    while (*g) {
	if (*g == delim) g++;
	*p++ = ((*g == '\0')? '\0' : *g++);
    }
    return(s);
}


/* comchk() scans ahead in the input stream looking for a comment
   which begins with "--".  If it finds one or more, it builds a
   list of comment nodes, one per comment, and returns it.
   Continuation lines which begin with "---" are combined with
   the comments they continue.  When comchk() returns one or two
   characters may have been unput for rereading by the scanner.

   The temporary buffer tcom is used to reduce the number of
   calls to strapnd and therefore to realloc.  Since I don't
   know how realloc works, I don't really know how expensive it
   is, but it seems like calling it for every character would
   be too inefficient (FPT) */

nodep comchk()
{
    char ch;
    char tcom[32], *t, *et = &tcom[31];	/* temporary for comment */
    char *c = null;			/* points to final comment */
    nodep e, l = null;			/* l points to comment list (nodes);
					   e points to end of list */

    while (1) {
	while ((ch = input()) == ' ' || ch == '\t' || ch == '\n');
	if (ch == '\0') break;
	if (ch != '-') { unput(ch); break; }
	else {
	    if ((ch = input()) != '-') { unput(ch); unput('-'); break; }
	    else {
		ch = input();
		if (ch != '-') {
		    if (c) { 
			put_attach(c, &l, &e);
		  	c = null;
		    }
		}
		else ch = input();
		while (ch != '\n' && ch != '\0') {
		    t = tcom;
		    while (ch != '\n' && ch != '\0' && t < et) {
		        *t++ = ch;
		        ch = input();
		    }
		    *t = '\0';
		    c = c ? strapnd(c, tcom) : strsav(tcom);
		    t = tcom;
		}
	    }
	}
    }
    if (c) put_attach(c, &l, &e);
    return l;
}

/* put_attach builds a list of comment nodes.  c is a saved string (i.e.
   saved via strsav()).  l is a pointer to the head of the list and
   e is a pointer to the end of the list.  Both are passed by reference. */

put_attach(c, l, e)
    char *c;
    nodep *l, *e;
{
    nodep n;		/* points to new comment node */

    n = newnode(comment_node, nop_comment);
    n->nd.comment.text = c;
    if (*l == null) *l = *e = n;
    else {
	(*e)->ndf.next = n;
	*e = n;
    }
}

w_input() { input(); }
w_unput(c) { unput(c); }

int debug;

scan_debug()
{
	/* if lexdebug is set in the environment and nonzero */
	/* set the lex trace flag			     */
	/* note that scanner.c must be compiled with LEXDEBUG defined */
# ifdef LEXDEBUG
    debug = getval("lexdebug");
# endif LEXDEBUG
}
