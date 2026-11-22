#include <stdio.h>
#include <signal.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"                        
#include "arr.h"
#include "adaio.h"
#include "color.h"
#include <stdio.h>
#include <sgtty.h>

char *getenv();

putchr(c) 
char c;
{
        putchar(c);
}

fputchr(des, c)
   FILE *des;
   char c;
{
    fputc(c, des);
}

putstr(x) 
char *x;
{
        fputs(x, adaout);
}                              

putf(fmt,a,b,c,d,e,f,g)
{
    printf(fmt,a,b,c,d,e,f,g);
}

fputf(des, fmt, a,b,c,d,e,f,g)
{
    fprintf(des, fmt, a,b,c,d,e,f,g);
}

fgetf(des, fmt, a,b,c,d,e,f,g)
{
    fscanf(des, fmt, a,b,c,d,e,f,g);
}

getchr(p)
char *p;
{                      
        *p _ (getchar() & 0177); /* remove parity to avoid sign extension */
}

fgetchr(des,p)
    FILE *des;
    char *p;
{
    *p _ fgetc(des);
}
        
static	struct sgttyb termfix;
static	struct sgttyb oldterm;

getstr(s)
char *s;
{
        char ch;
	char *orig _ s;
        stty(0,&termfix);
	while (1) {
		ch _ getchar();
		switch(ch) {
		case '\n': printf("\n");
			*s++ _ '\n';
			goto HELL;
		case 4: /* control-d */
		        stty(0,&oldterm);
			return NULL;
		case 21: /* control-u */
			s _ orig;
			printf(" XXX\n");
			continue;
		case 27: /* escape */
			complete(&s,orig);
			continue;
		case 8: /* backspace */
		case 127: /* rubout */
			if (s > orig) {
				printf("%c %c",8,8);
				s--;
			}
			continue;
		case 18: /* control-r */
			*s _ 0;
			printf("^R\n%s",orig);
			continue;
		default: *s++ _ ch;
			putchar(ch);
			continue;
		} /* end switch */
	} /* end while */
HELL:   *s _ 0;
        stty(0,&oldterm);
        return true;
}

struct list_of_syms {
        struct list_of_syms *cdr;
        symp car;
};

typedef struct list_of_syms *symlist;

int str_ada_to_c();
symlist getsyms();
symlist pattern_lookup();
symlist pattern_tablookup();

idstuff(ch)
char ch;
{	boolean res = false;
	if (((ch >= '0') and (ch <= '9')) or
	    ((ch >= 'a') and (ch <= 'z')) or
	    ((ch >= 'A') and (ch <= 'Z')) or
	    (ch eq '_')) res _ true; 
	return res;
}

complete(s,orig)
char **s;
char *orig;
{	char extras[255];
	char pattern[255];
	char *traceback _ *s;
	char *p;
	symlist syms;

	while ((traceback neq orig) && (idstuff(*--traceback))) ;
			/* skip back to previous delimiter or beginning */
	if (traceback neq orig) traceback++;
	for (p _ pattern; traceback != *s; traceback++) *p++ _ *traceback;
			/* then copy characters into pattern array */
	*p _ 0;
	syms _ getsyms(pattern,extras, NULL, NULL);
	getchars(syms, pattern, extras); /* figure out usable characters */
	for (p _ extras; *p; **s _ *p++, *s _ *s + 1) ;
	printf("%s", extras);
}

getchars(syms, pattern, extras)
symlist syms;
char *pattern;
char *extras;
{
	char *p, *q;
	if (not syms) {
		*extras _ 0;
		putf("%c",7);  /* bell */
		return;
	}
	if (syms->cdr) {
		*extras _ 0;
		putf("%c",7);  /* bell */
		return;
	}
	/* now do simple diff on pattern and characters in syms->car, */
	/* put characters beyond pattern in syms->car into extras */
	for (p _ pattern, q _ syms->car->syf.name; *p; p++, q++) ;
	for (p _ extras; *q; *p++ _ *q++) ;
	*p _ 0;
}
		
symlist getsyms(pattern, newchars, typ, tab)
char *pattern;          /* string to use for lookup */
char *newchars;         /* room for ambiguous part, if any */
typep typ;              /* pick things of this type, if possible */
symtabp tab;            /* if not null, only look in this table */
{                                   
        symlist pos;    /* possible symbols based only on prefix string */
        if (tab neq null)  /* look only in specified table */
                pos _ pattern_tablookup(tab, pattern);
        else pos _ pattern_lookup(pattern);
	return pos;	/* return list for further processing */
};
       
printpos(pos)
symlist pos;
{
        if (pos) {
                putf("%s\n", pos->car->syf.name);
                printpos(pos->cdr);
        };
}

symlist pattern_tablookup(tab, s)
symtabp tab;
char *s;
{
        register symp p;
        int tabsize _ tab->symtab_size;
        int bucketnum;
        symlist syms _ null;
        symlist nsym;

        for (bucketnum _ 0; bucketnum < tabsize; bucketnum++) {
          p _ tab->symtab_tab[bucketnum];
          while (p neq null) {
                if (match(s, p->syf.name)) {
                        nsym _ (symlist)
				zalloc(sizeof(struct list_of_syms), true);
                        nsym->car _ p;
                        nsym->cdr _ syms;
                        syms _ nsym;
                };
          p _ p->syf.link;
          };
        };
        return syms;
};

symlist pattern_lookup(s)
char *s;
{                                                         
        register symtabp tab _ currtab;
        register symp p;
        int tabsize;
        int bucketnum;
        symlist syms _ null;
        symlist nsym;

        while (tab neq null) {
          tabsize _ tab->symtab_size;
          for (bucketnum _ 0; bucketnum < tabsize; bucketnum++) {
            p _ tab->symtab_tab[bucketnum];
            while (p) {
                if (match(s, p->syf.name)) {
                        nsym _ (symlist)
				zalloc(sizeof(struct list_of_syms), true);
                        nsym->car _ p;
                        nsym->cdr _ syms;
                        syms _ nsym;
                };
            p _ p->syf.link;
            };
          };
          tab _ tab->symtab_stat_link;
        };
        return syms;
}  

match(s, t)             /* check if first chars of t are s */
char *s, *t;
{
        int size _ strlen(s);
        int i;
        if (size > strlen(t)) return false;
        for (i _ 0; i < size; i++) {
                if (s[i] neq t[i]) return false;
        };
        return true;
}

FILE *standard_input() {
        return stdin;
}

FILE *standard_output() {
        return stdout;
}

FILE *standard_msg() {
        return stderr;
}

int controlz_exit();
int controlz_return();

uui_init(argc, argv)
int argc;
char *argv[];
{
        cur_color.fg _ (char)cyan;
        cur_color.bg _ (char)black;
        setbuf(stdout, null);   /* unbuffer stdout, here so Ray unaffected */
        reset_std_io();
        gtty(0,&oldterm);
        termfix _ oldterm;      /* structure copy! */
        termfix.sg_flags &= ~ECHO;
        termfix.sg_flags |= CBREAK;
	signal(SIGTSTP, controlz_exit);
	signal(SIGCONT, controlz_return);
}

controlz_exit()
{
	stty(0,&oldterm);
	kill(getpid(),SIGTSTP);
}

controlz_return()
{	
	signal(SIGTSTP, controlz_exit);
	signal(SIGCONT, controlz_return);
	stty(0,&termfix);
}

uui_term()
{
        stty(0,&oldterm);
}

reset_std_io() {
	adain _ (int **)standard_input();
	adaout _ (int **)standard_output();
	adamsg _ (int **)standard_msg();
}

uui_reset()
{
        reset_std_io();
}

extern FILE *stdbrk;

prompt(f) 
FILE *f;
{
        if(f == stdin) {
                fprintf(stderr, "\n> ");
                fflush(stdout);
        }
        else if(f == stdbrk)  {
                fprintf(stderr, "\n: ");
                fflush(stdout);         /* stdbrk? */
        }
}

error_xy(x,y)
    int x,y;
{
    if (x == 0)
        printf("%*s^\n",y+2,"");
}

set_give_tool_name(val)
{
}

sysputf(s,a,b,c,d,e,f,g)
{
     printf("\n");
     printf(s,a,b,c,d,e,f,g);
     printf("\n");
}

set_color(c) color_pair c; {
        int a;
        int b; 
        if (!color_able()) return;
        a = cur_color.fg == c.fg;
        b = cur_color.bg == c.bg; 
                        
        if (a && b) return; 
        if (!(a || b))
                printf("%c[%d;%dm",27,
                       color_to_bg(c.bg),color_to_fg(c.fg));
        else if (!a)
                printf("%c[%dm",27,color_to_fg(c.fg));
        else
                printf("%c[%dm",27,color_to_bg(c.bg));
        cur_color = c;
}

set_both(f,b) color_name f,b;
{
    color_pair t;
    t.fg = (char)f;
    t.bg = (char)b;
    set_color(t); 
}


what_color(cp) color_pair *cp; {
        cp->fg = cur_color.fg;
        cp->bg = cur_color.bg;
}

color_able() {
        static int is_color_term = -1;
        if (is_color_term == -1) {
            char *term = getenv("TERM");
            is_color_term = (term && ((strcmp(term,"cs10") == 0) ||
                            (strcmp(term,"vt100") == 0)));
        }
        return is_color_term;
}
