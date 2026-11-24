/* port.c -- small things the 1984 system got from 4.2BSD. */
#define _GNU_SOURCE	/* for fopencookie */
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

/* -e or --echo (or ARCTURUS_ECHO=1): echo each line of input as it is
   read, so a script run with "./arcturus -e < file" shows what was typed.
   stdin is replaced by a stream that reads fd 0 a line at a time and
   copies each line to the output.  The 1984 program ignores its
   arguments, so the flag is seen here, before main. */
#ifndef __wasi__	/* port: the web build echoes input in JavaScript instead */
#include <string.h>
#include <sys/types.h>
ssize_t read(int, void *, size_t), write(int, const void *, size_t);	/* not <unistd.h>: its sbrk clashes with decls64.h */
static ssize_t echo_read(void *c, char *buf, size_t size)
{
	size_t n = 0;
	while (n < size && read(0, buf + n, 1) == 1)
		if (buf[n++] == '\n') break;
	if (n) {
		fflush(stdout);
		write(1, buf, n);
	}
	return n;
}
__attribute__((constructor)) static void echo_input(int argc, char **argv)
{
	int i, on = 0;
	char *e = getenv("ARCTURUS_ECHO");
	if (e && *e && strcmp(e, "0")) on = 1;
	for (i = 1; i < argc && argv; i++)
		if (!strcmp(argv[i], "-e") || !strcmp(argv[i], "--echo")) on = 1;
	if (on) {
		cookie_io_functions_t io = { echo_read, 0, 0, 0 };
		FILE *f = fopencookie(0, "r", io);
		if (f) stdin = yyin = f;
	}
}
#endif /* __wasi__ */
