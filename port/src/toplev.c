static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/toplev.v  Revision 1.8  83/11/30  14:42:21  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/toplev.v $
 * Revision 1.8  83/11/30  14:42:21  ada
 *
 * [RECONSTRUCTED 2026.  The original toplev.c did not survive on tape.
 *  This file was rebuilt from the compiled code and debugging symbols in
 *  the Arcturus 1.6-A executable "ada" (January 5, 1984).  Every line
 *  that generates code is placed on the line number the 1984 compiler
 *  recorded for it.  Names of functions, parameters, locals and globals
 *  are the original ones.  Comments, blank lines and the order of the
 *  declarations at the top are not recoverable and are written fresh.
 *  The earlier RCS log entries are lost.]
 */

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/vadvise.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "handler.h"
#include "version.h"

jmp_buf	topenv;			/* top level of the read-eval loop */

extern	long stack_level;	/* depth of the interpreter return stack */
extern	long *write_core_file_p;	/* points to "write_core_file" symbol */

extern	FILE *fdopen();
extern	char *parsestring();
extern	char *sbrk();
extern	long etext();

/*	Top level of Arcturus: the read-parse-interpret loop, the signal
	handlers, and saving and restoring the whole environment as a
	core image ("ada.core").				*/

/*	control-c handling: when a control-c arrives we set a flag and
	let the interpreter call control_c_handler at a safe point.  */















long check_control_c() {
	if (control_c_was_typed)
		control_c_handler();
}

long ignore_control_c() {
	signal(SIGINT, ignore_control_c);
	sysputf("Sorry, please wait for interrupt service.\n");
}

long set_flag() {
	signal(SIGINT, ignore_control_c);
	control_c_was_typed _ true;
	/* the interpreter will notice the flag */
}

long reinit() {
	if (currtab != usrtab)
		poplevel(currtab, null);
	popretstk(null);

	if (stack_level > 0)
		system_error("Error in interpreter stack!");
	currtab _ usrtab;
	uui_reset();
	clearinterp();
	longjmp(topenv, 0L);
}

long trap_handler(sig) long sig; {
	signal(SIGILL, SIG_IGN);
	signal(SIGBUS, SIG_IGN);
	signal(SIGSEGV, SIG_IGN);
	switch (sig) {
	case SIGBUS:	fatal_error("bus error");
	case SIGSEGV:	fatal_error("segmentation violation");
	case SIGXCPU:	break;
	default:	fatal_error("unknown signal %d", sig);
	}
}

long break_handler() {
	signal(SIGQUIT, break_handler);
	ada_break(true);
}

long control_c_handler() {
	clean_up_using_control_c_stack();
	signal(SIGINT, set_flag);
	reinit();
}

FILE	*stdbrk;		/* input stream while at a break */

/*	Enter a break: a nested read-eval loop at the point of an error
	or a quit signal.  End-of-file leaves the break.	*/
long ada_break(recoverable)
boolean recoverable;
{
	long	fildes;
	char	where_buf[1000];
	nodep	p;

	/* say where we are, then read and interpret commands until
	   the user types end-of-file */

	string_where(where_buf, currtab);
	putf("\nBreak in: %s\n", where_buf);

	while (true) {
		char	*ret;
		char	buf[1024];
		fildes _ dup(fileno(stdin));
		stdbrk _ fdopen(fildes, "r");
		setbuf(stdbrk, null);
		prompt(stdbrk);
		if (getstr(buf) == null) {
			fclose(stdbrk);
			if (recoverable) break;
			else
				sysputf("Can't continue from this type of error");
			if (feof(stdin)) exit(1L);	/* port: stop at end of piped input */
			continue;
		}

		curr_generation++;
		ret _ parsestring(buf, &p);
		if (ret)
			parser_error(ret);
		else	interp(p);
		fclose(stdbrk);
	}
}


long signalinit() {
	char *s;

	signal(SIGXCPU, trap_handler);
	signal(SIGILL, trap_handler);
	signal(SIGBUS, trap_handler);
	signal(SIGSEGV, trap_handler);
	signal(SIGQUIT, break_handler);
	/*
	 * SIGINT is set up by the caller, once
	 * the environment has been initialized.
	 */
}




long adainit() {
	clearinterp();
	syminit();
}

/*	Saving and restoring the environment.  A core file holds the
	time stamp of the executable that wrote it, the address of the
	end of the data segment, and the data segment itself.  Restoring
	reads the data segment back in over the running one.	*/

char	*write_core_flag;	/* set by write_core: save at top level */
char	*read_core_flag;	/* set by read_core: restore at top level */
extern	long time_stamp;		/* changes every time "ada" is linked */
extern	long pagesize;		/* data segment starts on a page boundary */











long read_core_file(s) char *s;
{	long fd, data_addr, time_in_file, idata; char fname[256];
	fd _ open(s, 0L);
	if (fd != -1) { close(fd); sysputf("[port: %s is a memory image; it can't be restored on this machine]\n", s); }
	return false;	/* port */
	/* start of the data segment: first page boundary after the text */

	idata _ (long)etext % pagesize ? (long)etext + pagesize - (long)etext % pagesize : (long)etext;
	/* s may point into the data segment we are about to overwrite,
	   so keep a copy of the name */

	strcpy(fname, s);
	if (fd == -1) return false;
	read(fd, &time_in_file, sizeof(long));
	if (time_in_file != time_stamp) {
		sysputf("[Core file '%s' doesn't match this version of Arcturus]", s);
		close(fd);
		return false;
	}
	read(fd, &data_addr, sizeof(long));
	brk(data_addr);
	read(fd, idata, data_addr - idata);
	close(fd);
	putf("[Context restored from '%s']\n", fname);
	return true;
}


long write_core_file(s) char *s;
{	long data_addr, idata, fd;
	sysputf("[port: saving a memory image is not supported on this machine]\n");
	return false;	/* port */
	data_addr _ (long)sbrk(0L);
	idata _ (long)etext % pagesize ? (long)etext + pagesize - (long)etext % pagesize : (long)etext;

	fd _ creat(s, 0644L);
	if (fd == -1) return false;
	write(fd, &time_stamp, sizeof(long));
	write(fd, &data_addr, sizeof(long));
	write(fd, idata, data_addr - idata);
	close(fd);
	putf("[Core file '%s' written]\n", s);
	return true;
}






long main(argc, argv)
long argc; char **argv;
{
	char	buf[1024];
	char	*ret;
	nodep	p;
	long	fd;

	vadvise(VA_ANOM);

	read_core_flag _ write_core_flag _ null;
	finit();
	if (read_core_file("ada.core")) {
		sysputf("%s  %s %d.%d-%c  %s", systemname, systemtype,
		    majorversion, minorversion, fixversion, sysdate);
		signalinit();
		uui_restore();
	}
	else {
		uui_init(argc, argv);
		sysputf("%s  %s %d.%d-%c  %s", systemname, systemtype,
		    majorversion, minorversion, fixversion, sysdate);
		signalinit();
		adainit();
	}

	signal(SIGINT, set_flag);

	setjmp(topenv);			/* errors come back here */
	while (true) {
		prompt(stdin);
		if (getstr(buf) == null) break;
		curr_generation++;
		ret _ parsestring(buf, &p);
		if (ret) parser_error(ret);
		else {
			enter_list(p);
			/* remember the command, then
			   run it */
			interp(p);
		}

		if (write_core_flag) {	char *s;
			s _ write_core_flag;
			write_core_flag _ null;
			write_core_file(s);
		}
		if (read_core_flag)
			read_core_file(read_core_flag);
		/* a restored environment carries on from
		   here with its own symbol tables */
	}
	putf("\n");
	quit(*write_core_file_p);
}
