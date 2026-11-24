/* wasm_compat.c -- what 4.2BSD gave Arcturus that WASI does not.
   Only the web build links this file. */
#include <stdio.h>
#include <string.h>

/* stdout is unbuffered so prompts (stderr), echoed input and output
   come out in the order they happened */
__attribute__((constructor)) static void wasm_unbuffer(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
}

/* the page's WASI layer makes a second descriptor for the same stream;
   ada_break() uses this to read stdin unbuffered */
__attribute__((import_module("arcturus"), import_name("dup")))
extern int js_dup(int fd);
int dup(int fd) { return js_dup(fd); }

/* no processes in a browser */
FILE *popen(const char *cmd, const char *mode) { return NULL; }
int pclose(FILE *f) { return -1; }
/* the one command Arcturus needs from the shell is "mkdir user" (dump.c) */
#include <sys/stat.h>
int system(const char *cmd)
{
	if (cmd && !strncmp(cmd, "mkdir ", 6)) return mkdir(cmd + 6, 0777) ? 1 : 0;
	return -1;
}
int vfork(void) { return -1; }
int execl(const char *path, ...) { return -1; }
int wait(int *status) { return -1; }

/* signals never arrive in a browser; Arcturus's own raise() (Ada
   exceptions) collides with libwasi-emulated-signal, so signal() is here */
#include <signal.h>
void (*signal(int sig, void (*fn)(int)))(int) { return SIG_DFL; }
void __SIG_IGN(int sig) {}
void __SIG_ERR(int sig) {}

/* a removed C library function the 1984 code still calls */
char *gets(char *s)
{
	char *r = fgets(s, 1024, stdin);
	if (r) {
		size_t n = strlen(s);
		if (n && s[n - 1] == '\n') s[n - 1] = 0;
	}
	return r;
}

/* end of the text segment; only used by the disabled core-image code */
long etext(void) { return 0; }

/* clang insists main returns int; the 1984 main is "long main" */
#undef main
long arcturus_main(long argc, char **argv);
int main(int argc, char **argv) { return (int)arcturus_main(argc, argv); }
