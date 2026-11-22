/* missing.c -- functions the 1984 binary has but the tape's sources lack,
   plus the few library routines 4.2BSD supplied (libl, monitor).

   docref, enter_list, dump_history and reorder_and_dump were rebuilt from
   the compiled code in the Arcturus 1.6-A binary (cref.c rev 1.2 and the
   working copy of dump.c rev 1.7 that was linked); tail is copied from
   object.c rev 1.6, whose other functions loaded VAX object files. */
#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"

FILE *fopen();

docref(p, fn)
nodep p; char *fn;
{
	sysputf("Cref is not implemented.");
}

nodep list_head, list_tail;	/* every command typed, for dump_history */

enter_list(p)
nodep p;
{
	if (list_head == null)
		list_head = p;
	else
		list_tail->ndf.next = p;
	list_tail = p;
}

dump_history(filename)
char *filename;
{
	register FILE *f;
	register nodep p;
	char s[256];

	str_ada_to_c(s, filename);
	f = fopen(s, "w");
	if (f == null)
		error("Can't open %s for dumping history", s);
	for (p = list_head; p; p = p->ndf.next)
		dopp(p, f, 1, 1);
	fclose(f);
}

reorder_and_dump()
{
	putf("Dump not implemented yet.  Need more funding!\n");
}

char *tail(s)
char *s;	/* returns tail of file pathname s */
{
	char *p;
	p = s;
	while (*s)
		if (*s++ == '/')
			p = s;
	return p;
}

/* symtab.c, 1984 working copy: "rm" and "touch" in package uci */
delete_sym(fix)
int *fix;
{
	check_top_level();
	*fix = 0;		/* fix->fxf.class = undef_id */
}

touch(f)
int *f;
{
	if (*f == 4 || *f == 5)		/* func_id, proc_id */
		subp_swapin(f);
	else if (*f == 10)		/* pack_id */
		package_swapin(f);
}

/* -p profiling */
monitor() { }

/* from the 4.2BSD lex library (libl) */
yywrap() { return 1; }

extern struct yysvf *yylstate[], **yylsp, **yyolsp;
extern int *yyfnd, yyleng, yyprevious;
extern char yytext[];
struct yysvf { struct yywork *yystoff; struct yysvf *yyother; int *yystops; };

yyreject()
{
	for (; yylsp < yyolsp; yylsp++)
		yytext[yyleng++] = yyinput();
	if (*yyfnd > 0)
		return *yyfnd++;
	while (yylsp-- > yylstate) {
		yyunput(yytext[yyleng-1]);
		yytext[--yyleng] = 0;
		if (*yylsp != 0 && (yyfnd = (*yylsp)->yystops) && *yyfnd > 0)
			return *yyfnd++;
	}
	if (yytext[0] == 0)
		return 0;
	yyoutput(yyprevious = yyinput());
	yyleng = 0;
	return -1;
}
