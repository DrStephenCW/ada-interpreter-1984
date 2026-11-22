static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/init.v  Revision 1.13  83/11/30  16:27:57  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/init.v $
 * Revision 1.13  83/11/30  16:27:57  ada
 * changed type of parameter in funcinit
 * 
 * Revision 1.12  83/11/30  13:42:43  ada
 * same as 1.11.5.1
 * 
 * Revision 1.11.5.1  83/11/29  15:48:41  snider
 * Changed funcinit from a macro to a procedure.  The macro seemed (can't
 * trust sdb) to be having trouble with a cast of the form (int (*) ()).
 * 
 * Revision 1.11  83/09/08  15:56:31  ada
 * same as 1.10.9.1
 * 
 * Revision 1.10.9.1  83/08/25  14:26:08  ritter
 * Added Ada-83 exception 'program_error' for functions that run into their
 * end statement.
 * 
 * Revision 1.10  83/06/14  16:28:14  ada
 * Same as 1.9.4.1.
 * 
 * Revision 1.9.4.1  83/06/14  13:49:10  tadman
 * Hook in new procedures include, include_commands, and load.  Use
 * new C function commands_include to read standard.str and the SETUPNAME
 * and SWAPNAME files.
 * 
 * Revision 1.9  83/04/05  15:24:30  ada
 * *** empty log message ***
 * 
 * Revision 1.8  83/04/05  14:37:15  ada
 * included call to funcinit for putsym procedure.
 * 
 * Revision 1.7  83/03/10  15:29:53  ada
 * Same as 1.6.2.1.
 * 
 * Revision 1.6.2.1  83/03/10  14:44:50  willson
 * switch to ada 82 i/o
 * 
 * Revision 1.5.2.1  83/02/04  14:01:13  willson
 * Added package text_io with ada82 i/o and exceptions
 * Added save and restore coredump routines
 * 
 * Revision 1.5  83/01/12  09:15:48  ada
 * Added copyright notice.
 * 
 * Revision 1.4  83/01/11  14:48:22  ada
 * Same as 1.3.2.1.
 * 
 * Revision 1.3.2.1  83/01/11  09:51:55  willson
 * Changed str_ada_to_c to use new arr.h
 * Moved str_c_to_ada from office.c to here
 * (Office.c is no longer needed.)
 * 
 * Revision 1.3  83/01/04  14:10:31  willson
 * put call to fflush around csh because output is buffered again
 * (this really speeds it up).
 * 
 * Revision 1.2  82/12/30  13:58:31  ada
 * Same as 1.1.1.1
 * 
 * Revision 1.1.1.1  82/12/20  15:18:53  willson
 * Changed callcsh to take new boolean flag; true => saves terminal state, 
 *  false => allow subcsh to change it.
 * 
 * Revision 1.1  82/12/12  21:27:29  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include <signal.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "arr.h"
#include "adaio.h"
#include "count.h"
#include "oblist.h"
#include "eval.h"
/* port: block-scope externs, repeated at file scope */
extern  boolean argmatch ();

extern char *get_editor_name();
extern nodep ada_parse_file();
extern nodep ada_parse();
extern long ada_include(), ada_include_commands(), ada_load();
extern nodep int_form(), body_int_form();
extern char *macro_pname();		/* Castor */
extern long reorder_and_dump();	/* export laundry, sort of */
extern long dump_history();		/* dump record of user input */

nodep root;		/* root of predefined environment package */
long *croak_okp;		/* points to value slot of "croak_ok" symbol. */
long *write_core_file_p;	/* points to "write_core_file" symbol. */
symtabp standard_tab;	/* used by UUI to detect recursive def of standard */
static nodep userp;	/* root of user package internal form */

long symbol_table();	/* symbolic symbol table listing */
long static_link();	/* symbolically list all tables on the static link */

/* stuff for package internal */

/* nodep */ 
long ada_eval();		/* the interpreter */
long ada_pp();		/* the prettyprinter */
long ada_ifp();		/* the internal form prettyprinter */
nodep ada_copytree();	/* copy an internal form tree */
long macro_substitute();	/* macro substitution [$thing]	  (Castor) */

long getchr();		/* I/O functions */
long fgetchr();
long putchr();
long fputchr();

	/* prettyprinter, backtrace,  directory, and 
	   cross reference procedures */

extern long symifp(), sympp(), bktr(), symcref(), symdir();
extern long expand(), unexpand();
extern long putsym();

extern FILE_TYPE standard_input(), standard_output(), standard_msg();	/* port */
extern char *getenv();

FILE_TYPE current_input() { return adain; }
FILE_TYPE current_output() { return adaout; }
FILE_TYPE current_msg() { return adamsg; }

long set_input(f) FILE_TYPE f; { adain = f; }
long set_output(f) FILE_TYPE f; { adaout = f; }
long set_msg(f) FILE_TYPE f; { adamsg = f; }

static FILE_TYPE fd_out(astr)
arrayp astr;
{	char cstr[100];
	FILE *f;
	str_ada_to_c(cstr,astr);
	f _ fopen(cstr,"w");
	setbuf(f,NULL);
	if (not f) error("Can't open file %s for writing",cstr);
	return (FILE_TYPE)f;
}

static FILE_TYPE fd_in(astr)
arrayp astr;
{	char cstr[100];
	FILE *f;
	str_ada_to_c(cstr,astr);
	f _ fopen(cstr,"r");
	if (not f) error("Can't open file %s for reading", cstr);
	return (FILE_TYPE)f;
}

long str_ada_to_c(cstr, astr)
char *cstr;
arrayp astr;
{
        register long len, i;

        len = arr_size(astr);
        for(i = 0 ; i < len; i++) {
                cstr[i] = astr->arr_base[i];
        }
        cstr[len] = 0;
}

long str_c_to_ada(astr,cstr)         /* copy c string into ada string array */
arrayp astr;			/* null terminate if possible */
char *cstr;
{
        register long i;
        register long len _ strlen(cstr);        /* length of C string */
        register long size _ arr_size(astr);     /* size of ada array */

        if (len > size)  error("String overflow (copying c str to ada)");

        for (i _ 0 ; i < len; i++)              /* copy non-blank part */
                astr->arr_base[i] _ cstr[i];

	if (len < size) astr->arr_base[len] _ 0;
				/* null terminate */
}

extern	char *write_core_flag;
extern	char *read_core_flag;

long check_top_level() {
	if (currtab != usrtab)
	  error("This procedure must be executed at the top-level");
}

static	long symwrite_core(a)
arrayp a;
{	static	char s[256];
	check_top_level();
	str_ada_to_c(s,a);
	write_core_flag = s;
}

static	long symread_core(a)
arrayp a;
{	static	char s[256];
	check_top_level();
	str_ada_to_c(s,a);
	read_core_flag _ s;
}

long	memreport();	/* report on memory usage */
long	quick_memreport();	/* only print total usage */

long	total_size_of_tree;
long	number_of_nodes;
extern	long nill();
extern	long sizetab[];

static long treecounter(p)
nodep p;
{	if (p) {
		total_size_of_tree += sizetab[(long)(p->ndf.op)];
		number_of_nodes++;
	}
}

static long treesize(f)
    fixp f;
{
	nodep p;
	total_size_of_tree _ 0;
	number_of_nodes _ 0;

	if (f->fxf.class neq proc_id and f->fxf.class neq func_id) {
		putf("Can only get size of proc's or func's for now");
		return;
	}
	p _ f->fx.func.int_form;
	maptree(p, treecounter, nill, nill, nill);
	putf("%d nodes, %d bytes\n", number_of_nodes,total_size_of_tree);
}

extern long etext();		/* definitions for profiling */
#define BUFSIZE 10000L
static short BUF[BUFSIZE];

long profile_on() {
	monitor((long) 2, etext, BUF, BUFSIZE, 200L);
}

long profile_off() {
	monitor(0L);
}

/*      set to leave Ada environment and dump "core" if bus error or */
/*	segmentation fault or illegal instruction */

long dumpcore() {
        signal(SIGBUS, SIG_DFL);
        signal(SIGSEGV, SIG_DFL);
        signal(SIGILL, SIG_DFL);
}

/*	leave ada environment */

long die() {
	if (*croak_okp == 0)
	  error("Can't die unless croak_ok is true.");
	quick_memreport();
	uui_term();	
	exit(1L);	/* doesn't dump usr table, so good way to get out
			   when you don't want to save anything from your
			   current session.  */
}

long quit(write_core) 
long write_core;
{ 
	if (write_core) {
	   check_top_level();
	   write_core_file("ada.core");
	}
	quick_memreport();	/* report memory used */
        uui_term();	/* finish up any interface processing */
        dumpusr();	/* dump "usr" symbol table */
        exit(0L); 	/* leave and close files */
}

/* explicit call to break package */

static long symbreak() {
        ada_break(true);        /* recoverable */
}

long dir();			/* directory of package */
long symdel();			/* delete a symbol */
long clear_counters();		/* clear execution frequency counters */
long question();			/* ask a question via mail */
long bug();			/* make a bug report via mail */

static long symcat(t)		/* print a file on the terminal */
arrayp t;
{
        char command[256];
	char name[256];
	str_ada_to_c(name,t);
        sprintf(command, "more %s", name);
        system(command);
}

/*	fix_last_command calls an editor on the file Fix_File; Fix_File
	contains the last input stream before the one that invoked "fix;",
	which is in Input_File.  This way, typing "fix;" will get you the
	last thing you screwed up, and you can edit it.  Then the newly
	edited text is included back into the system.  Of course, if you
	screw up the "fix;" command, that's too bad.
*/

extern	char *Fix_File;

       long fix_last_command()
{
	textedit(Fix_File);
	command_include(Fix_File, true);
}

static long symedit(f)       /* edit the value of a symbol */
    fixp f;                /* (functions only for now) */
{
	char name[256];
        switch(f->fxf.class) {
            case func_id:
            case proc_id:
	    case pack_id:	/* BUG: for pack_id, must edit use list to use
				   new definition of package! */
				/* also, I don't think freetree is called on
				   the old package.  */
		savename(name, f);
		textedit(name);
		include(name, true);
		break;
	    case macro_id:			/* Castor */
                edit(f->fx.func.int_form);
                break;
            default:
                error("Can't edit that");
        }
}

       long textedit(t)	/* edit a text object in the environment */
char *t;
{
        char command[256];
        char *editor;

        editor = get_editor_name();
        sprintf(command, "%s %s", editor, t);
        system(command);
}

static long symtextedit(a) arrayp a;
{	char name[256];
	str_ada_to_c(name,a);
	textedit(name);
}

       long symdel(f)        /* remove a symbol entry */
    fixp f;
{
    check_top_level();
    deltab (f);  /* only can delete in usr package */
}


       long callcsh(ada_s,term_flag)	/* call a csh to hack around in or */
arrayp ada_s;			/* execute the given command and return */
boolean term_flag;
/*	much of the code in here was swiped from "system.c", the c library
	procedure.  I don't pretend to understand this, but the simplistic
	version I had figured out had the side-effect that signals to the
	ada environment didn't work somehow once this routine was called.
	The system couldn't even
	commit suicide with kill(getpid(),9).  So I put this in.  The main
	change is saving the INT and QUIT handlers while we wait for the
	child to terminate.
*/
{
    char    s[256];
    char    command[256];
    long    (*istat)(), (*qstat)(), status;

    str_ada_to_c (s, ada_s);
    fflush(stdout);
    if (term_flag) set_users_terminal();
    if (*s) {			/* command given as arg to "csh" */
	long     pid, w;
	if ((pid = vfork ()) == 0) {
	    execl ("/bin/csh", "csh", "-c", s, 0L);
	    _exit (127L);
	}
	istat _ signal (SIGINT, SIG_IGN);
	qstat _ signal (SIGQUIT, SIG_IGN);
	while ((w = wait (&status)) neq pid and w neq - 1);
	if (w eq - 1)
	    status _ - 1;
	signal (SIGINT, istat);
	signal (SIGQUIT, qstat);
    }
    else
	status = system ("csh");
    if (term_flag) reset_users_terminal();
    return status;
}

static long sympsyms()       /* ugly dump of the symbol table */
{       
    printall();
}

/*	text_io stuff	*/

static fixp	status_error, mode_error, name_error, use_error, device_error,
		end_error, data_error, layout_error;
			/* io_exceptions	*/

#ifdef whitehill

long symcreate(fp, mode, t, form)	/* create a text object */
    FILE **fp;
    file_mode mode;	/* must be out */
    char *t;
    char *form;		/* ignored */
{
    register    FILE * f;
    if (mode neq out_file)
	error("mode error: must be out_file");
    if (*fp != null)
	error ("status error: file already open %s", t);
    f = fopen (t, "w");
    if (f == null)
	error ("name error: can't create %s", t);
    *fp = f;
}

long symopen(fp, mode, t, form)	/* open a text object for reading */
    FILE **fp;
    file_mode mode;	/* must be in_file */
    char *t;
    char *form;		/* ignored */
{
    register    FILE * f;
    if (mode neq in_file)
	error("mode error: must be in_file");
    if (*fp != null)
	error ("status error: file already open %s", t);
    f = fopen (t, "r");
    if (f == null)
	error ("name error: can't open %s", t);
    *fp = f;
}

long symclose(fp)		/* close a text object */
    FILE **fp;
{
    if (*fp == null)
	error ("status error: file already closed");
    fclose (*fp);
    *fp = null;
}

#else whitehill

/*	versions more like manual definition */

long symcreate(fp, mode, t, form)	/* create a text object */
    FILE **fp;
    file_mode mode;	/* must be out_file */
    arrayp t;
    arrayp form;	/* ignored */
{
    register    FILE * f;
    char name[256];
    if (mode neq out_file)
	sysraise(mode_error, "must be out_file");
    str_ada_to_c(name,t);
    if (*fp != null)
	sysraise (status_error, "file already open %s", name);
    f = fopen (name, "w");
    if (f == null)
	sysraise (name_error, "can't create %s", name);
    *fp = f;
}

long symopen(fp, mode, t, form)	/* open a text object for reading */
    FILE **fp;
    file_mode mode;	/* must be in_file */
    arrayp t;
    arrayp form;	/* ignored */
{
    register    FILE * f;
    char name[256];
    if (mode neq in_file)
	sysraise(mode_error, "must be in_file");
    str_ada_to_c(name,t);
    if (*fp != null)
	sysraise (status_error, "file already open %s", name);
    f = fopen (name, "r");
    if (f == null)
	sysraise (name_error, "can't open %s", name);
    *fp = f;
}

long symclose(fp)		/* close a text object */
    FILE **fp;
{
    if (*fp == null)
	sysraise (status_error, "file already closed");
    fclose (*fp);
    *fp = null;
}

#endif whitehill

/* i/o on files: */

long fgetint(f, ip)
FILE *f;
long *ip;
{
    fgetf(f, "%d", ip);
}

static long getint(p)                  /* get an integer */
long *p;
{                        
        fgetint(adain, p);
}
          
static long check_limit(actual, upper)
long actual, upper;
{
    if (actual >= upper)
	sysraise(data_error,
	 "Short-term limitation: string length (which is %d) must be < %d", 
	 actual, upper);
}

static long symfgetstr(f,a)
FILE *f;
arrayp a;
{	long len _ arr_size(a);
	long count;
	char data[256];
	check_limit(len,256L);
	for (count _ 0; count < len; count++)
		fgetchr(f, &data[count]);
	data[count] _ '\000';
	str_c_to_ada(a, data);
}

static long symgetstr(a)
arrayp a;
{	symfgetstr(adain, a);
}

static long symfget_line(f,s,n)
FILE *f;
arrayp s;
long *n;
{
	long len	_ arr_size(s);
	long amt;
	char data[256];
	check_limit(len,256L);
	fgets(data, len + 1, f);
	amt _ strlen(data);
	if (data[amt - 1] eq '\n') {
		data[amt - 1] _ '\000';
		amt--;
	}
	str_c_to_ada(s, data);
	*n _ amt;
}

static long symget_line(s,n)
arrayp s;
long *n;
{	symfget_line(adain, s, n);
}

long fgetreal(f, rp)
FILE *f;
float *rp;
{
   fgetf(f, "%f", rp);
}

static long getreal(p)        /* get a floating value */
float *p;                     
{
   fgetreal(adain, p);
}

static long fputadastr(f, s)
FILE *f;
arrayp s;
{
        char buffer[256];
	long len _ arr_size(s);
	check_limit(len, 256L);
        str_ada_to_c(buffer, s);
        fputf(f, buffer);
}

static long putadastr(s)     /* put an Ada string to standard output */
arrayp s;
{
        fputadastr(adaout, s);
}

static long symput_line(s)
arrayp s;
{	putadastr(s);
	newline(1L);
}

static long symfput_line(f, s)
FILE *f;
arrayp s;
{	fputadastr(f, s);
	fnewline(f,1L);
}

boolean symfeof(f)		/* return true if at end of file */
FILE *f;
{	long ch;
	ch _ getc(f);
	ungetc(ch,f);
	if (feof(f)) return true;
	else return false;
}

long fputint(f, x, width)
FILE *f;
long x;
long width;
{
        fputf(f, "%*d", width, x);
}

long putint(x, w) long x, w; {              /* put an integer on the terminal */
        fputint(adaout, x, w);
}

static long fputreal(f, xbits, width, prec)
FILE *f;
long xbits;	/* port: float bits, from call() */
long width;
long prec;
{
        char buf[128]; float x = *(float *)&xbits;	/* port */
        if(width == 0 && prec == 0) 
                sprintf(buf, "%g", x);
        else sprintf(buf, "%*.*f", width, prec, x);
        fputf(f, buf);
}                              

static long putreal(x, width, prec)        /* put a real on the terminal */
long x;	/* port: float bits, passed on as they are */
long width;
long prec;
{
        fputreal(adaout, x, width, prec);
}                              

       long fnewline(f, n)
FILE *f;
long n;
{
	while(n--)
         fputf(f, "\n");
}
                            
       long newline(n) long n; {     /* print n newlines; n defaults to 1 */
        fnewline(adaout, n);
}
                            
/*	print a string describing a path from "standard" to current scope */

static long string_where1(s,tab)
    char *s;
    symtabp tab;
{
    symtabp next;

    if (tab -> symtab_owner &&
	    tab -> symtab_owner -> fxf.class == pack_id) {
	next = tab -> symtab_static_link;
	if (next && next -> symtab_owner &&
		tab -> symtab_owner == next -> symtab_owner)
	    tab _ next;
    }
    next _ tab -> symtab_static_link;

    if (next) {
	string_where1 (s, next);
	strcat (s, ".");
    }
    if (tab -> symtab_owner)
	if (tab -> symtab_owner -> fxf.class == macro_id) {
	/* Castor */
	    strcat (s, "{");
	    strcat (s, macro_pname (tab -> symtab_owner -> fx.func.int_form ->
			nd.macrox.designator));
	    strcat (s, "}");
	}
	else
	    strcat (s, tab -> symtab_owner -> fxf.name);
    else
	strcat (s, "Block-or-Loop");
}

long string_where(s,currtab)
char *s;
symtabp currtab;
{
	s[0] = 0;
	string_where1(s,currtab);
}

long where()         /* static backtrace */
{   
	char buff[256];
	string_where(buff,currtab);
        putf("%s\n",buff);  
}

static long print_owner(t)
    symtabp t;
{
    if (t->symtab_owner)
	putf("%s", t->symtab_owner->fxf.name);
    else putf("Block-or-Loop");
    putf("(%x)", t);
}

long stack_trace()
{
    register    retsp r;
    register    symtabp sym;

    putf ("Stack trace:\n");
    for (r _ retstk; r; r _ r -> link) {
	sym = r -> currtab;
	if (sym) {
	    print_owner (sym);
	    putf (" -- static link --> ");
	    if (sym -> symtab_static_link)
		print_owner (sym -> symtab_static_link);
	    else
		putf ("Null");
	}
	else
	    putf ("<no table>");
	putf ("\n");
    }
}

/* 	execution backtrace */

extern nodep intstk[], *intsp;

long how()                  /* dynamic backtrace */
{   
        register nodep *sp;
        register nodep p;

        for(sp = intsp; sp >= intstk; sp--) {
                p = *sp;
                if(p->ndf.op == nop_call || p->ndf.op == nop_proc_call)
                        dopp(p, standard_output(), true, false);
        }
}

static fixp findfix(name)
    char    *name;          /* only "standard" exists when this is called */
{
    register fixp fix;

    fix = lookup (currtab, intern(name), null);
    if (fix == null)
	error ("standard symbol %s not found", name);
    return(fix);
}
                                  

long funcinit(name, funct)
    char *name;
    long (*funct)();
{
    findfix(name)->fx.func.code = funct;
}



long ovlinit(name, params, func)	/* initialize a symbol which has */
     char *name;			/* overloaded meanings */
    char *params;
    long (*func)();
{
    register fixp fix, paramfix;
    extern  boolean argmatch ();

    paramfix = lookup(currtab, intern (params), null);
    if (paramfix == null)
	error ("standard function %s not found", params);
    fix = lookup(currtab,
		  intern (name), 
		  argmatch, 
		  paramfix -> fx.func.int_form -> nd.subp.spec);
    if (fix == null)
	error ("standard function %s not found", name);
    fix -> fx.func.code = func;
}

nodep mkidnode(name)	/* make an internal form identifier node */
    char *name;
{
    register    nodep p;
    p = newnode (ident_node, nop_ident);
    p -> nd.ident.id = intern (name);
    return (p);
}

long *objinit(name)	/* used to lookup type identifiers for initializing */
char    *name;		/* predefined object types */
{
    register fixp fix;

    fix = lookup(currtab, intern(name), null);
    if (fix == null)
	error ("standard type %s not found", name);
    return (fix -> fxf.dyna -> sy.var.value);
}
                                  
/*	main workhorse: */

long syminit () {			/* initialize standard.usr symbol table
				   and predefined types */
    register    fixp fix;
    register    symp sym;
    register    symtabp stand_vis_tab, stand_body_tab;

    long     dummy = 0;

    intdebugp = &dummy; countingp = &dummy;	/* port: was a read of address 0 */		/* until intdebugp points to ada symbol;
				/* necessary, because we go through the
				   interpreter loop before intdebug is
				   defined. */

    undeftype = newtype (undefform);
    undeftype -> tyf.size = 0;

/*	Create a symbol for the predefined environment, and hang the package
	we just created off of it.  Make a symbol table for the package,
	which gets hung off the internal form and also off of the symbol
	for the package.  In turn, the symbol table hangs off the internal
	form.  whew

	We also have to create the scoping information, since this package
	must appear to have been interpreted, so its scope would be 
	visible as the outter-most scope.  
*/

    stand_vis_tab _ newtab (16L, fix);	/* contains a few typedefs only */

    root _ ada_parse_file ("standard");
    root -> nd.packagex.table _ stand_vis_tab;

    fix = newfix(packfixform, pack_id);
    sym = newsym (packsymbolform, pack_id);
    fix -> fxf.name = intern ("standard");
    fix -> fx.pack.int_form _ root;
    fix -> fx.pack.body_int_form _ root->ndf.next;
    stand_vis_tab->symtab_owner = fix;
    currtab = stand_vis_tab;

    eval (root -> nd.packagex.decl, decl_mode, &dummy, null);
    
    enter(stand_vis_tab,fix,sym);  /* standard has a reference to itself */
    fix -> fx.pack.int_form -> nd.packagex.table _ stand_vis_tab;
    standard_tab _ stand_vis_tab;	/* used by UUI to get at standard */

 /* create references to internal typep`s; used in interpreter to
    reference and compare type information */

    functype = newtype (undefform);
    functype -> tyf.size = 0;	/* object is a function */

    typetype = newtype (undefform);
    typetype -> tyf.size = 0;	/* object is a type */

    packtype = newtype (undefform);
    packtype -> tyf.size = 0;	/* object is a package */

    excepttype = newtype (undefform);
    excepttype -> tyf.size = 0;	/* object is an exception name */

    nulltype = newtype (accessform);
    nulltype -> ty.access.targettype = undeftype;/* pointer to null */

 /* now hook in standard types.  previous types are internal to the */
 /* system only; the following ones have Ada equals.  */

 /* The types file_type and symbol have to be in standard because
    we have to fill them in before we can use them, and due to certain
    timing vulgarities, this is the way we do it.
 */
    filetype = findfix ("file_type") -> fxf.dyna -> sy.var.type;
    filetype -> tyf.class = undefform;
    filetype -> tyf.int_form = mkidnode ("file_type");
    filetype -> tyf.size = 0;

    symboltype = findfix ("symbol") -> fxf.dyna -> sy.var.type;
    symboltype -> tyf.class = accessform;
    symboltype -> tyf.int_form = mkidnode ("symbol");
    symboltype -> tyf.size = sizeof (long *);
    symboltype -> ty.access.targettype = newtype (undefform);

    texttype = findfix ("text") -> fxf.dyna -> sy.var.type;
    texttype -> tyf.class = undefform;
    texttype -> tyf.int_form = mkidnode ("text");
    texttype -> tyf.size = 0;	/* whitehill`s flexible text object */

    chartype = findfix ("character") -> fxf.dyna -> sy.var.type;
    chartype -> tyf.class = enumform;
    chartype -> tyf.int_form = mkidnode ("character");
    chartype -> tyf.size = sizeof (long);
    chartype -> ty.srange.lbound = 0;/* characters */
    chartype -> ty.srange.ubound = 127;

    inttype = findfix ("integer") -> fxf.dyna -> sy.var.type;
    inttype -> tyf.class = intform;
    inttype -> tyf.int_form = mkidnode ("integer");
    inttype -> tyf.size = sizeof (long);
    inttype -> ty.srange.lbound = -256 * 256 * 256 * 128;
    inttype -> ty.srange.ubound = 256 * 256 * 256 * 128 - 1;
    inttype -> ty.srange.basetype = inttype;/* range of itself */

    realtype = findfix ("float") -> fxf.dyna -> sy.var.type;
    realtype -> tyf.class = realform;
    realtype -> tyf.int_form = mkidnode ("float");
    realtype -> tyf.size = sizeof (long);	/* port: was sizeof (float) */
 /* float must take up one word */

    booltype = findfix ("boolean") -> fxf.dyna -> sy.var.type;
 /* the usual */


/*	Strings and the subtype natural must be defined after the simple
	predefined types have been given sizes and what not.  Now read
	in the definition of string and natural and hook it all up.
	Since standard is on the top of the stack, these definitions will
	get entered into it. */

    command_include ("standard.str", true);
    strtype = findfix ("string") -> fxf.dyna -> sy.var.type;
 /* unconstrained array type */

/*	Find the addresses of the standard exceptions so they can
	be raised by the interpreter. */

    constraint_error = findfix ("constraint_error");
    numeric_error = findfix ("numeric_error");
    storage_error = findfix ("storage_error");
/* program error added by PAR 22-Aug-83 */
    program_error = findfix ("program_error");

/*	Everything has now been entered into the visible part of 
	standard.  Now we can build the table for the body (which is
	empty), thus completing package standard. */

    fix _ findfix ("standard");
    stand_body_tab _ newtab (4L, fix);	/* contains only packages like usr,
					   uci, etc. */
    stand_body_tab -> symtab_static_link = stand_vis_tab;
    stand_body_tab -> symtab_owner = fix;
    fix -> fx.pack.body_int_form -> nd.packagex.table = stand_body_tab;

    currtab = stand_body_tab;

/*      hook in functions and procedures that are written in C into the 
	package that holds all this stuff */

    include ("uci_ada_fns", true);
    fix _ findfix ("uci");
    currtab = fix->fx.pack.body_int_form->nd.packagex.table;

/*      hook in the address of functions and procedures written in C */

    funcinit ("die", die);
    funcinit ("history", dump_history);
    funcinit ("export", reorder_and_dump);
    funcinit ("memreport", memreport);
    funcinit ("treesize", treesize);
    funcinit ("profile_on", profile_on);
    funcinit ("profile_off", profile_off);
    funcinit ("stack_trace", stack_trace);
    funcinit ("csh", callcsh);
    funcinit ("edit", symedit);
    { extern long delete_sym(); funcinit ("rm", delete_sym); }	/* port: as in the 1984 binary */
    funcinit ("where", where);
    funcinit ("how", how);
    { extern long printall(); funcinit ("printall", printall); }	/* port: as in the 1984 binary */
    funcinit ("symbol_table", symbol_table);
    funcinit ("static_link", static_link);
    funcinit ("bktr", bktr);
    funcinit ("cref", symcref);
    funcinit ("quit", quit);
    funcinit ("type_out", symcat);
    funcinit ("textedit", symtextedit);
    funcinit ("edit", symedit);
    funcinit ("break", symbreak);
    funcinit ("dumpcore", dumpcore);
    funcinit ("clear_counters", clear_counters);
    funcinit ("question", question);
    funcinit ("bug", bug);
    funcinit ("fix", fix_last_command);
    funcinit ("restore", symread_core);
    funcinit ("save", symwrite_core);
    funcinit ("include", ada_include);
    funcinit ("include_commands", ada_include_commands);
    funcinit ("load", ada_load);
    funcinit ("putsym", putsym);
    { extern long touch(); funcinit ("touch", touch); }	/* port: as in the 1984 binary */

    intdebugp = objinit ("intdebug");
    countingp = objinit ("counting");
    croak_okp = objinit ("croak_ok");
    write_core_file_p = objinit("write_core_file");

    /* read in text_io */
    currtab = stand_body_tab;
    include("text_io", true);
    fix _ findfix("text_io");
    currtab _ fix -> fx.pack.body_int_form->nd.packagex.table;


    /* hook in text_io functions */
    funcinit ("create", symcreate);
    funcinit ("open", symopen);
    funcinit ("standard_input", standard_input);
    funcinit ("standard_output", standard_output);
    funcinit ("standard_msg", standard_msg);
    funcinit ("current_input", (long (*) ()) current_input);
    funcinit ("current_output", (long (*) ()) current_output);
    funcinit ("current_msg", (long (*) ()) current_msg);
    funcinit ("set_input", set_input);
    funcinit ("set_output", set_output);
    funcinit ("set_msg", set_msg);
    funcinit ("close", symclose);
    funcinit ("putstr", putadastr);
    funcinit ("putreal", putreal);
    funcinit ("putchr", putchr);
    funcinit ("putint", putint);
    funcinit ("fputint", fputint);
    funcinit ("fputreal", fputreal);
    funcinit ("fputchr", fputchr);
    funcinit ("fputstr", fputadastr);
    funcinit ("new_line", newline);
    funcinit ("fnew_line", fnewline);
    funcinit ("getint", getint);
    funcinit ("getchr", getchr);
    funcinit ("getreal", getreal);
    funcinit ("getstr", symgetstr);
    funcinit ("fgetstr", symfgetstr);
    funcinit ("fgetint", fgetint);
    funcinit ("fgetchr", fgetchr);
    funcinit ("fgetreal", fgetreal);
    funcinit ("put_line", symput_line);
    funcinit ("fput_line", symfput_line);
    funcinit ("get_line", symget_line);
    funcinit ("fget_line", symfget_line); 
    funcinit ("end_of_file", (long (*) ()) symfeof);

 /* also read in io_exceptions; hook them up	*/

    fix _ findfix("io_exceptions");
    currtab _ fix -> fx.pack.body_int_form->nd.packagex.table;

    status_error _ findfix("status_error");
    mode_error _ findfix("mode_error");
    name_error _ findfix("name_error");
    use_error _ findfix("use_error");
    device_error _ findfix("device_error");
    end_error _ findfix("end_error");
    data_error _ findfix("data_error");
    layout_error _ findfix("layout_error");
				/* whew */

 /*	read in pp_functions	*/

    currtab = stand_body_tab;
    include ("pp_functions", true);
    fix _ findfix ("pp_functions");
    currtab = fix->fx.pack.body_int_form->nd.packagex.table;

/*      hook in the address of functions and procedures written in C */

    funcinit ("pp", sympp);
    funcinit ("ifp", symifp);
    funcinit ("dir", symdir);

    currtab = stand_body_tab;
    include ("user_package", true);
    fix _ findfix ("usr");
    usrtab = currtab = fix->fx.pack.body_int_form->nd.packagex.table;

 /* Now load the internal_fns package.  This package contains subprograms
    which provide access to the Arcturus internal form.  The package
    internal uses these functions.  This package will go away when
    it is possible to load packages which reference symbols in the base
    system. */

    currtab = stand_body_tab;
    include ("internal_fns", false);	/* ok if it's not there */
    fix = lookup (currtab, intern("internal_fns"), null);
    if (fix) {
        currtab = fix->fx.pack.int_form->nd.packagex.table;

/*      hook in the address of functions and procedures written in C */

 /* hooks for package internal_form and package castor */

	funcinit ("t_parse", (long (*) ()) ada_parse);
	funcinit ("t_eval", ada_eval);
	funcinit ("t_pp", ada_pp);
	funcinit ("t_ifp", ada_ifp);
	funcinit ("t_copytree", (long (*) ()) ada_copytree);
	funcinit ("t_macro_subst", macro_substitute);
	funcinit ("t_expand", expand);
	funcinit ("t_unexpand", unexpand);
	funcinit ("t_int_form", (long (*) ()) int_form);
    	funcinit ("t_body_int_form", (long (*) ()) body_int_form);

    }

    currtab = stand_body_tab;

 /* now initialize the package for the user's objects. */

    include ("user_package", true);
    fix _ findfix ("usr");
    usrtab = currtab = fix->fx.pack.body_int_form->nd.packagex.table;

 /* ADA.SETUP is a hack that should go away when we get AVOS.  It is
    counter to the "continuous session" philosophy of AVOS, but this isn`t
    the first temporary hack in the system, so it should be ok. It is a
    file which like ADA.SAVE gets read in when ada is started.  Unlike
    ADA.SAVE, it is not rewritten upon exit from the environment. It is an
    appropriate place to put "pragma load", "pragma include", and "use"
    statements which do not get written into ADA.SAVE. */

    command_include(SETUPNAME, false);
    command_include(SWAPNAME, false);
}

