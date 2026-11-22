static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/command.v  Revision 1.7  84/03/16  16:57:41  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/command.v $
 * Revision 1.7  84/03/16  16:57:41  ada
 * same as 1.6.5.1
 * 
 * Revision 1.6.5.1  84/03/16  14:54:36  snider
 * added check for null file descriptor in all file i/o routines.  This
 * was leading to segmentation faults if an attempt was made to read or
 * write a file that wasn't opened.
 * 
 * Revision 1.6  83/09/21  09:47:38  ada
 * same as 1.5.5.1
 * 
 * Revision 1.5.5.1  83/09/20  16:06:41  snider
 * rearranged things in other places so that adash would restore from a
 * core file a little more cleanly.  This led to some different calls to
 * procedures in adash, and they now appear here as empty procedures.
 * 
 * Revision 1.5  83/03/10  15:25:41  ada
 * same as 1.4.2.1
 * 
 * Revision 1.4.2.1  83/03/10  14:43:25  willson
 * *** empty log message ***
 * 
 * Revision 1.3  83/01/12  09:12:33  ada
 * Added copyright notice.
 * 
 * Revision 1.2  83/01/04  14:04:15  willson
 * Made the output buffered again.
 * 
 * Revision 1.1  82/12/12  21:20:33  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include <signal.h>
#include "macros.h"
#include "adaio.h"

extern boolean append_next_string;      /* see parsestring() */
long lines_read;

long control_c_handler();

char *getenv();

long putchr(c) 
char c;
{
        fputc(c,adaout);
}

long fputchr(des, c)
   FILE *des;
   char c;
{
    if (des == null)
	error("status error: trying to write unopened file");
    else
	fputc(c, des);
}

long putstr(x) 
char *x;
{
        fputs(x, adaout);
}                              

long putf(fmt,a,b,c,d,e,f,g)
long fmt, a, b, c, d, e, f, g; {
    printf(fmt,a,b,c,d,e,f,g);
}

long fputf(des, fmt, a,b,c,d,e,f,g)
long des, fmt, a, b, c, d, e, f, g; {
    if (des == null)
	error("status error: trying to write unopened file");
    else
	fprintf(des, fmt, a,b,c,d,e,f,g);
}

long fgetf(des, fmt, a,b,c,d,e,f,g)
long des, fmt, a, b, c, d, e, f, g; {
    if (des == null)
	error("status error: trying to read unopened file");
    else
	fscanf(des, fmt, a,b,c,d,e,f,g);
}

long getchr(p)
char *p;
{                      
        *p = (getchar() & 0177); /* remove parity to avoid sign extension */
}

long fgetchr(des,p)
    FILE *des;
    char *p;            
{
    if (des == null)
	error("status error: trying to read unopened file");
    else
	*p = fgetc(des);
}
        
long fgetstr(f,s)
    FILE *f;
    char *s;
{
    if (f == null)
	error("status error: trying to write unopened file");
    else
	return (long)fgets(s,1024L,f);
}

long getstr(s)
char *s;
{
        char ch;
        long (*old_interrupt)();
        old_interrupt _ signal(SIGINT,control_c_handler);
        if ((ch = getchar()) == EOF) {
                signal(SIGINT, old_interrupt);
                return NULL;
        }
        else ungetc(ch,stdin);
        while ((*s++ = getchar()) != '\n') ;
        *s = 0;
        signal(SIGINT, old_interrupt);
        return true;
}

FILE_TYPE standard_input() {
        return (FILE_TYPE)stdin;
}

FILE_TYPE standard_output() {
        return (FILE_TYPE)stdout;
}

FILE_TYPE standard_msg() {
        return (FILE_TYPE)stderr;
}


long init_io()
{
}



long uui_init(argc, argv)
long argc;
char *argv[];
{
        cur_color.fg _ white;
        cur_color.bg _ black;
/*      setbuf(stdout, null);   /* unbuffer stdout, here so Ray unaffected */
		/* too slow to have it unbuffered, so do fflush's now 
		   when it needs to be flushed, like when calling csh. */
        reset_std_io();
}


long finit()
{
}



long uui_term()
{
}

long reset_std_io() {
        adain = standard_input();
        adaout = standard_output();
        adamsg = standard_msg();
}



long uui_restore()
{
}



long uui_reset()
{
        reset_std_io();
}

long add_sym_for_completion(s)
long s; {
}

extern FILE *stdbrk;

long prompt(f) 
    FILE *f;
{
    if (append_next_string) {
        fprintf (stderr, "+ ");
        fflush (stdout);
        lines_read++;
        return;
    }
    if (f == stdin) {
        fflush (stdout);
        fprintf (stderr, "\n> ");
    }
    else
        if (f == stdbrk) {
            fprintf (stderr, "\n: ");
            fflush (stdout);    /* stdbrk? */
        }
    lines_read = 0;
}

long error_xy(x,y)
    long x,y;
{
    if (x <= lines_read)
        printf("%*s^\n",y+3,"");
    else
        append_next_string = true;
}

long set_give_tool_name(val)
long val; {
}

long sysputf(s,a,b,c,d,e,f,g)
long s, a, b, c, d, e, f, g; {
     printf("\n");
     printf(s,a,b,c,d,e,f,g);
     printf("\n");
}

long set_color(c) color_pair c; {
        long a;
        long b; 
        if (!color_able()) return;
        a = cur_color.fg == c.fg;
        b = cur_color.bg == c.bg; 
                        
        if (a && b) return; 
        if (!(a || b))
                printf("%c[%d;%dm",27L,
                       color_to_bg(c.bg),color_to_fg(c.fg));
        else if (!a)
                printf("%c[%dm",27L,color_to_fg(c.fg));
        else
                printf("%c[%dm",27L,color_to_bg(c.bg));
        cur_color = c;
}

long set_both(f,b) color_name f,b;
{
    color_pair t;
    t.fg = f;
    t.bg = b;
    set_color(t); 
}


long what_color(cp) color_pair *cp; {
        cp->fg = cur_color.fg;
        cp->bg = cur_color.bg;
}

long color_able() {
        static long is_color_term = -1;
        if (is_color_term == -1) {
            char *term = getenv("TERM");
            is_color_term = (term && ((strcmp(term,"cs10") == 0) ||
                            (strcmp(term,"vt100") == 0)));
        }
        return is_color_term;
}
