/* port.c -- small things the 1984 system got from 4.2BSD. */
#include <stdio.h>
extern FILE *yyin, *yyout;

/* old lex initialized these statically, which modern C does not allow */
__attribute__((constructor)) static void init_lex_streams(void)
{
	yyin = stdin;
	yyout = stdout;
}

/* let a prebuilt binary find its run-time files: ARCTURUS_LIB=/path/to/lib/ */
extern char *Library;
char *getenv();
__attribute__((constructor)) static void lib_from_env(void)
{
	char *e = getenv("ARCTURUS_LIB");
	if (e && *e) Library = e;
}

/* where the heap started, for memreport (misc.c) */
long port_brk0;
char *sbrk();
__attribute__((constructor)) static void note_brk0(void)
{
	port_brk0 = (long)sbrk(0);
}
