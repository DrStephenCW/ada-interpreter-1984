# include "stdio.h"
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0L
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1L
# define YYLMAX 200L
# define output(c) putc(c,yyout)
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
# define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
long yyleng; extern char yytext[];
long yymorfg;
extern char *yysptr, yysbuf[];
long yytchar;
FILE *yyin, *yyout;
extern long yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	long *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
	/* global definitions */
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

extern long aatoi(), abatoi();
extern float aatof(), abatof();	/* aatof -- ada version of atof */
				/* which converts ascii to float */
				/* abatof -- same for based float */
char *strconv();
char *trans();
nodep comchk();

extern YYSTYPE yylval;
long	yycol = 1;
long     yyprline, yyprcol;	/* line and column of prevous token */
static boolean noclp, nocl = false;  /* true if no character literal allowed */
boolean in_call_form;		/* true if in calling form.  Causes all
				   keywords except begin and end to be
				   treated as identifiers */
# define YYNEWLINE 10L
long yylex(){
long nstr; extern long yyprevious;
	yyprline = yylineno;
	yyprcol = yycol;
	noclp = nocl;
	nocl = false;
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:
		{ nocl = true;
					  yylval.integer.val = aatoi(yytext);
					  yylval.integer.prval = strsav(yytext);
					  yylval.integer.attach = comchk();
					  return(Integer);  }
break;
case 2:
{ nocl = true;
					  yylval.real.val = aatof(yytext); 
					  yylval.real.prval = strsav(yytext);
					  yylval.real.attach = comchk();
					  return(Real);	 }
break;
case 3:
	{ nocl = true;
					  yylval.integer.val = abatoi(yytext);
					  yylval.integer.prval = strsav(yytext);
					  yylval.integer.attach = comchk();
					  return(Integer);  }
break;
case 4:
	{ nocl = true;
					  trans(yytext,':','#');
					  yylval.integer.val = abatoi(yytext);
					  yylval.integer.prval = strsav(yytext);
					  yylval.integer.attach = comchk();
					  return(Integer);  }
break;
case 5:
  { nocl = true;
					  yylval.real.val = abatof(yytext);
					  yylval.real.prval = strsav(yytext);
					  yylval.real.attach = comchk();
					  return(Real);  }
break;
case 6:
  { nocl = true;
					  trans(yytext,':','#');
					  yylval.real.val = abatof(yytext);
					  yylval.real.prval = strsav(yytext);
					  yylval.real.attach = comchk();
					  return(Real);  }
break;
case 7:
				{ if (noclp) REJECT; 
					  yylval.character.val = yytext[1];
					  yylval.character.attach = comchk();
					  return(Character);  }
break;
case 8:
	{ yylval.ident.val =
						strsav(strconv(yytext));
					  yylval.ident.attach = comchk();
					  return(chkopr(yylval.ident.val)); }
break;
case 9:
	ada_perror("unterminated string");
break;
case 10:
		{ yylval.ident.val =
						strsav(strconv(yytext));
					  yylval.ident.attach = comchk();
					  return(chkopr(yylval.ident.val)); }
break;
case 11:
	ada_perror("%% delimited string contains double quote");
break;
case 12:
		ada_perror("unterminated string");
break;
case 13:
				{ yylval.ident.attach = comchk();
					  return(RIGHT_ARROW); }
break;
case 14:
		 		{ yylval.ident.attach = comchk();
					  return(SUBRANGE_SYM); }
break;
case 15:
		 		{ yylval.ident.attach = comchk();
					  return(EXPONENTIAL); }
break;
case 16:
		 		{ yylval.ident.attach = comchk();
					  return(ASSIGN_OP); }
break;
case 17:
				{ yylval.ident.attach = comchk();
					  return(NEQ); }
break;
case 18:
				{ yylval.ident.attach = comchk();
					  return(GEQ); }
break;
case 19:
				{ yylval.ident.attach = comchk();
					  return(LEQ); }
break;
case 20:
				{ yylval.ident.attach = comchk();
					  return(LEFT_LABEL_BRACKET); }
break;
case 21:
				{ yylval.ident.attach = comchk();
					  return(RIGHT_LABEL_BRACKET); }
break;
case 22:
				{ yylval.ident.attach = comchk();
					  return(BOX); }
break;
case 23:
				{ yylval.ident.attach = comchk();
					  return('&'); }
break;
case 24:
				{ yylval.ident.attach = comchk();
					  return('\''); }
break;
case 25:
				{ yylval.ident.attach = comchk();
					  return('('); }
break;
case 26:
				{ nocl = true;
					  yylval.ident.attach = comchk();
					  return(')'); }
break;
case 27:
				{ yylval.ident.attach = comchk();
					  sysputf("warning: [ changed to (");
					  return('('); }
break;
case 28:
				{ nocl = true;
					  yylval.ident.attach = comchk();
					  sysputf("warning: ] changed to )");
					  return(')'); }
break;
case 29:
				{ yylval.ident.attach = comchk();
					  return('*'); }
break;
case 30:
				{ yylval.ident.attach = comchk();
					  return('+'); }
break;
case 31:
				{ yylval.ident.attach = comchk();
					  return(','); }
break;
case 32:
				{ yylval.ident.attach = comchk();
					  return('-'); }
break;
case 33:
				{ yylval.ident.attach = comchk();
					  return('.'); }
break;
case 34:
				{ yylval.ident.attach = comchk();
					  return('/'); }
break;
case 35:
				{ yylval.ident.attach = comchk();
					  return(':'); }
break;
case 36:
				{ yylval.ident.attach = comchk();
					  return(';'); }
break;
case 37:
				{ yylval.ident.attach = comchk();
					  return('<'); }
break;
case 38:
				{ yylval.ident.attach = comchk();
					  return('='); }
break;
case 39:
				{ yylval.ident.attach = comchk();
					  return('>'); }
break;
case 40:
				{ yylval.ident.attach = comchk();
					  return('/'); }
break;
case 41:
				{ yylval.ident.attach = comchk();
					  return('|'); }
break;
case 42:
				{ yylval.ident.attach = comchk();
					  return('{'); }
break;
case 43:
				{ yylval.ident.attach = comchk();
					  return('}'); }
break;
case 44:
				{ yylval.ident.attach = comchk();
					  return('$'); }
break;
case 45:
				{ yylval.ident.attach = comchk();
					  return(DBL_DOLLAR); }
break;
case 46:
	{ long idtype;
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
break;
case 47:
				;
break;
case 48:
				;
break;
case 49:
				;
break;
case 50:
				ada_perror(
					    "invalid character %s (%o)\n",
					    yytext, yytext[0]);
break;
case -1:
break;
default:
fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */

long scan_reset()  /* clears scanner look-ahead buffer */
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
    long i;

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

long put_attach(c, l, e)
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

long w_input() { input(); }
long w_unput(c) long c; { unput(c); }

long debug;

long scan_debug()
{
	/* if lexdebug is set in the environment and nonzero */
	/* set the lex trace flag			     */
	/* note that scanner.c must be compiled with LEXDEBUG defined */
# ifdef LEXDEBUG
    debug = getval("lexdebug");
# endif LEXDEBUG
}
long yyvstop[] ={
0,

50,
0,

47,
50,
0,

49,
0,

41,
50,
0,

50,
0,

44,
50,
0,

50,
0,

23,
50,
0,

24,
50,
0,

25,
50,
0,

26,
50,
0,

29,
50,
0,

30,
50,
0,

31,
50,
0,

32,
50,
0,

33,
50,
0,

34,
40,
50,
0,

1,
2,
50,
0,

35,
50,
0,

36,
50,
0,

37,
50,
0,

38,
50,
0,

39,
50,
0,

46,
50,
0,

27,
50,
0,

28,
50,
0,

42,
50,
0,

43,
50,
0,

9,
0,

8,
0,

45,
0,

12,
0,

11,
0,

10,
0,

15,
0,

14,
0,

17,
0,

1,
2,
0,

16,
0,

20,
0,

19,
0,

22,
0,

13,
0,

18,
0,

21,
0,

46,
0,

7,
0,

48,
0,

2,
0,

1,
2,
0,

3,
0,

4,
0,

2,
0,

3,
0,

5,
0,

6,
0,

4,
0,

5,
0,

6,
0,
0};
# define YYTYPE char
struct yywork { YYTYPE verify, advance; } yycrank[] ={
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,4,	1,6,	1,7,	
1,3,	1,8,	1,9,	1,10,	
1,11,	1,12,	1,13,	1,14,	
1,15,	1,16,	1,17,	1,18,	
1,19,	1,20,	8,34,	14,40,	
17,41,	18,42,	33,59,	38,60,	
39,61,	0,0,	0,0,	1,21,	
1,22,	1,23,	1,24,	1,25,	
1,3,	1,3,	1,26,	19,43,	
21,50,	24,54,	1,26,	65,73,	
1,26,	25,55,	25,56,	2,3,	
2,8,	68,77,	2,10,	2,11,	
2,12,	2,13,	2,14,	65,74,	
2,16,	2,17,	2,18,	2,19,	
23,51,	23,52,	23,53,	64,71,	
1,27,	1,3,	1,28,	1,3,	
1,26,	1,3,	2,21,	2,22,	
2,23,	2,24,	2,25,	2,3,	
2,3,	67,76,	7,31,	63,68,	
74,82,	68,77,	80,86,	78,84,	
83,90,	9,35,	7,31,	7,32,	
89,93,	64,72,	63,69,	81,87,	
65,75,	9,35,	9,36,	64,71,	
1,29,	84,91,	1,30,	1,3,	
95,98,	87,92,	97,99,	2,27,	
2,3,	2,28,	2,3,	0,0,	
2,3,	7,31,	7,31,	7,33,	
74,82,	0,0,	7,31,	0,0,	
9,35,	9,35,	9,37,	0,0,	
7,31,	9,38,	0,0,	0,0,	
0,0,	7,31,	0,0,	9,35,	
81,88,	84,91,	0,0,	0,0,	
9,35,	87,92,	0,0,	2,29,	
0,0,	2,30,	2,3,	63,70,	
0,0,	0,0,	7,31,	78,85,	
0,0,	0,0,	7,31,	0,0,	
7,31,	9,35,	0,0,	0,0,	
0,0,	9,35,	0,0,	9,35,	
45,64,	45,64,	45,64,	45,64,	
45,64,	45,64,	45,64,	45,64,	
45,64,	45,64,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
7,31,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	9,35,	
11,0,	11,0,	11,0,	11,0,	
11,0,	11,0,	11,0,	11,0,	
11,0,	11,0,	11,0,	11,0,	
11,0,	11,0,	11,0,	11,0,	
11,0,	11,0,	11,0,	11,0,	
11,0,	11,0,	11,0,	11,0,	
11,0,	11,0,	11,0,	11,0,	
11,0,	11,0,	11,0,	11,39,	
11,39,	11,39,	0,0,	20,44,	
11,39,	0,0,	0,0,	0,0,	
0,0,	0,0,	11,39,	0,0,	
0,0,	0,0,	20,45,	11,39,	
20,46,	20,46,	20,46,	20,46,	
20,46,	20,46,	20,46,	20,46,	
20,46,	20,46,	20,47,	0,0,	
0,0,	0,0,	0,0,	0,0,	
11,39,	0,0,	0,0,	0,0,	
11,39,	20,48,	11,39,	44,63,	
44,63,	44,63,	44,63,	44,63,	
44,63,	44,63,	44,63,	44,63,	
44,63,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
44,63,	44,63,	44,63,	44,63,	
44,63,	44,63,	11,39,	20,49,	
0,0,	0,0,	0,0,	0,0,	
0,0,	20,48,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
44,63,	44,63,	44,63,	44,63,	
44,63,	44,63,	11,0,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	0,0,	0,0,	0,0,	
0,0,	26,58,	0,0,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	26,57,	26,57,	26,57,	
26,57,	41,41,	0,0,	0,0,	
0,0,	0,0,	48,66,	0,0,	
48,66,	41,41,	41,62,	48,67,	
48,67,	48,67,	48,67,	48,67,	
48,67,	48,67,	48,67,	48,67,	
48,67,	49,46,	49,46,	49,46,	
49,46,	49,46,	49,46,	49,46,	
49,46,	49,46,	49,46,	0,0,	
41,41,	41,41,	41,41,	0,0,	
0,0,	41,41,	0,0,	0,0,	
0,0,	0,0,	0,0,	41,41,	
0,0,	0,0,	0,0,	0,0,	
41,41,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	47,65,	
47,65,	47,65,	47,65,	47,65,	
47,65,	47,65,	47,65,	47,65,	
47,65,	41,41,	0,0,	0,0,	
0,0,	41,41,	0,0,	41,41,	
47,65,	47,65,	47,65,	47,65,	
47,65,	47,65,	66,67,	66,67,	
66,67,	66,67,	66,67,	66,67,	
66,67,	66,67,	66,67,	66,67,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	41,41,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
47,65,	47,65,	47,65,	47,65,	
47,65,	47,65,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	58,57,	58,57,	58,57,	
58,57,	59,59,	0,0,	0,0,	
60,60,	0,0,	0,0,	0,0,	
71,79,	59,59,	71,79,	0,0,	
60,60,	71,80,	71,80,	71,80,	
71,80,	71,80,	71,80,	71,80,	
71,80,	71,80,	71,80,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
59,59,	59,59,	0,0,	60,60,	
60,60,	59,59,	0,0,	0,0,	
0,0,	0,0,	0,0,	59,59,	
0,0,	0,0,	60,60,	0,0,	
59,59,	0,0,	0,0,	60,60,	
77,83,	77,83,	77,83,	77,83,	
77,83,	77,83,	77,83,	77,83,	
77,83,	77,83,	0,0,	0,0,	
0,0,	59,59,	0,0,	0,0,	
60,60,	59,59,	0,0,	59,59,	
60,60,	0,0,	60,60,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	69,78,	69,78,	
69,78,	69,78,	69,78,	69,78,	
69,78,	69,78,	69,78,	69,78,	
0,0,	0,0,	0,0,	59,59,	
0,0,	0,0,	60,60,	69,78,	
69,78,	69,78,	69,78,	69,78,	
69,78,	73,81,	73,81,	73,81,	
73,81,	73,81,	73,81,	73,81,	
73,81,	73,81,	73,81,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	73,81,	73,81,	
73,81,	73,81,	73,81,	73,81,	
0,0,	0,0,	0,0,	69,78,	
69,78,	69,78,	69,78,	69,78,	
69,78,	79,80,	79,80,	79,80,	
79,80,	79,80,	79,80,	79,80,	
79,80,	79,80,	79,80,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	73,81,	73,81,	
73,81,	73,81,	73,81,	73,81,	
82,89,	82,89,	82,89,	82,89,	
82,89,	82,89,	82,89,	82,89,	
82,89,	82,89,	91,94,	0,0,	
91,94,	0,0,	0,0,	91,95,	
91,95,	91,95,	91,95,	91,95,	
91,95,	91,95,	91,95,	91,95,	
91,95,	92,96,	0,0,	92,96,	
0,0,	0,0,	92,97,	92,97,	
92,97,	92,97,	92,97,	92,97,	
92,97,	92,97,	92,97,	92,97,	
94,95,	94,95,	94,95,	94,95,	
94,95,	94,95,	94,95,	94,95,	
94,95,	94,95,	96,97,	96,97,	
96,97,	96,97,	96,97,	96,97,	
96,97,	96,97,	96,97,	96,97,	
0,0};
struct yysvf yysvec[] ={
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-40,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+0,	0,		yyvstop+8,
yycrank+-105,	0,		yyvstop+11,
yycrank+14,	0,		yyvstop+13,
yycrank+-112,	0,		yyvstop+16,
yycrank+0,	0,		yyvstop+18,
yycrank+-207,	0,		yyvstop+21,
yycrank+0,	0,		yyvstop+24,
yycrank+0,	0,		yyvstop+27,
yycrank+9,	0,		yyvstop+30,
yycrank+0,	0,		yyvstop+33,
yycrank+0,	0,		yyvstop+36,
yycrank+7,	0,		yyvstop+39,
yycrank+7,	0,		yyvstop+42,
yycrank+6,	0,		yyvstop+45,
yycrank+208,	0,		yyvstop+49,
yycrank+7,	0,		yyvstop+53,
yycrank+0,	0,		yyvstop+56,
yycrank+28,	0,		yyvstop+59,
yycrank+7,	0,		yyvstop+62,
yycrank+12,	0,		yyvstop+65,
yycrank+270,	0,		yyvstop+68,
yycrank+0,	0,		yyvstop+71,
yycrank+0,	0,		yyvstop+74,
yycrank+0,	0,		yyvstop+77,
yycrank+0,	0,		yyvstop+80,
yycrank+0,	yysvec+7,	0,	
yycrank+0,	0,		yyvstop+83,
yycrank+20,	0,		yyvstop+85,
yycrank+0,	0,		yyvstop+87,
yycrank+0,	yysvec+9,	0,	
yycrank+0,	0,		yyvstop+89,
yycrank+0,	0,		yyvstop+91,
yycrank+18,	0,		yyvstop+93,
yycrank+17,	0,		0,	
yycrank+0,	0,		yyvstop+95,
yycrank+-392,	0,		0,	
yycrank+0,	0,		yyvstop+97,
yycrank+0,	0,		yyvstop+99,
yycrank+231,	0,		0,	
yycrank+136,	0,		0,	
yycrank+0,	yysvec+20,	yyvstop+101,
yycrank+399,	0,		0,	
yycrank+355,	0,		0,	
yycrank+365,	0,		0,	
yycrank+0,	0,		yyvstop+104,
yycrank+0,	0,		yyvstop+106,
yycrank+0,	0,		yyvstop+108,
yycrank+0,	0,		yyvstop+110,
yycrank+0,	0,		yyvstop+112,
yycrank+0,	0,		yyvstop+114,
yycrank+0,	0,		yyvstop+116,
yycrank+0,	yysvec+26,	yyvstop+118,
yycrank+454,	0,		0,	
yycrank+-576,	yysvec+7,	0,	
yycrank+-579,	yysvec+9,	0,	
yycrank+0,	0,		yyvstop+120,
yycrank+0,	0,		yyvstop+122,
yycrank+72,	yysvec+44,	0,	
yycrank+22,	yysvec+45,	yyvstop+124,
yycrank+25,	yysvec+47,	0,	
yycrank+422,	0,		0,	
yycrank+10,	yysvec+66,	yyvstop+126,
yycrank+8,	0,		yyvstop+129,
yycrank+610,	0,		0,	
yycrank+0,	yysvec+44,	0,	
yycrank+541,	0,		0,	
yycrank+0,	yysvec+45,	0,	
yycrank+633,	0,		0,	
yycrank+39,	0,		yyvstop+131,
yycrank+0,	yysvec+47,	0,	
yycrank+0,	yysvec+66,	0,	
yycrank+580,	0,		0,	
yycrank+76,	yysvec+69,	0,	
yycrank+665,	0,		0,	
yycrank+15,	yysvec+79,	yyvstop+133,
yycrank+61,	yysvec+73,	0,	
yycrank+688,	0,		0,	
yycrank+17,	yysvec+77,	yyvstop+135,
yycrank+56,	0,		yyvstop+137,
yycrank+0,	yysvec+69,	0,	
yycrank+0,	yysvec+79,	0,	
yycrank+60,	0,		yyvstop+139,
yycrank+0,	yysvec+73,	0,	
yycrank+21,	yysvec+82,	yyvstop+141,
yycrank+0,	yysvec+77,	0,	
yycrank+703,	0,		0,	
yycrank+718,	0,		0,	
yycrank+0,	yysvec+82,	0,	
yycrank+728,	0,		0,	
yycrank+33,	yysvec+94,	yyvstop+143,
yycrank+738,	0,		0,	
yycrank+35,	yysvec+96,	yyvstop+145,
yycrank+0,	yysvec+94,	0,	
yycrank+0,	yysvec+96,	0,	
0,	0,	0};
struct yywork *yytop = yycrank+795;
struct yysvf *yybgin = yysvec+1;
char yymatch[] ={
00  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,011 ,012 ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
01  ,01  ,01  ,01  ,01  ,01  ,01  ,01  ,
040 ,'!' ,'"' ,'!' ,'!' ,'%' ,'!' ,'!' ,
'!' ,'!' ,'!' ,'+' ,'!' ,'+' ,'!' ,'!' ,
'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,'0' ,
'0' ,'0' ,'!' ,'!' ,'!' ,'!' ,'!' ,'!' ,
'!' ,'A' ,'A' ,'A' ,'A' ,'E' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'!' ,'!' ,'!' ,'!' ,'_' ,
'!' ,'A' ,'A' ,'A' ,'A' ,'E' ,'A' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,'G' ,
'G' ,'G' ,'G' ,'!' ,'!' ,'!' ,'!' ,01  ,
0};
char yyextra[] ={
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
long yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
char yytext[YYLMAX];
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
long *yyfnd;
extern struct yysvf *yyestate;
long yyprevious = YYNEWLINE;
long yylook(){
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	long yych;
	struct yywork *yyr;
# ifdef LEXDEBUG
	long debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank){		/* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
			*yylastch++ = yych = input();
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (long)yyt > (long)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((long)yyt < (long)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
long yyback(p, m)
	long *p;
long m; {
if (p==0) return(0);
while (*p)
	{
	if (*p++ == m)
		return(1);
	}
return(0);
}
	/* the following are only used in the lex library */
long yyinput(){
	return(input());
	}
long yyoutput(c)
  long c; {
	output(c);
	}
long yyunput(c)
   long c; {
	unput(c);
	}
