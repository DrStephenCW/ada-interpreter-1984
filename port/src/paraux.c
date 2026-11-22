static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/paraux.v  Revision 1.9  83/11/30  14:19:14  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/paraux.v $
 * Revision 1.9  83/11/30  14:19:14  ada
 * same as 1.8.5.1
 * 
 * Revision 1.8.5.1  83/11/29  16:06:22  snider
 * Initialized first_edit in incl.
 * 
 * Revision 1.8  83/08/09  15:13:51  ada
 * can now get out of an editor even though there is still a syntax error
 * in the code being edited. (incl(...)) w. thanks to brindle@aerospace.
 * 
 * Revision 1.7  83/08/08  17:12:29  ada
 * same as 1.6.5.1
 * 
 * Revision 1.6.5.1  83/08/08  16:02:35  snider
 * increased the size of some character buffers from 20 to 80 characters -
 * were overwriting memory in cases with long file name strings.
 * 
 * Revision 1.6  83/06/14  21:26:13  tadman
 * Make edit function (explicit call of the editor from ada) check for
 * $EDITOR == none and print an error message if it is.
 * 
 * Revision 1.5  83/06/14  16:41:20  ada
 * Same as 1.4.4.1.
 * 
 * Revision 1.4.4.1  83/06/14  14:05:42  tadman
 * Fixed command to invoke emacs.  Added new $EDITOR option "none."
 * "none" causes no editor to be invoked on syntax errors.  Added
 * code for new ada procedures include, include_commands, and load which
 * replace pragma include and pragma load.
 * 
 * Revision 1.4  83/01/12  09:07:37  ada
 * Added copyright notice.
 *
 * Revision 1.3  83/01/11  14:56:50  ada
 * Same as 1.2.2.1
 *
 * Revision 1.2.2.1  83/01/11  09:56:52  willson
 * Fixed bug in adaread re: library files.
 *
 * Revision 1.2  83/01/04  14:16:42  willson
 * Edit functions will use a default editor if no EDITOR shell
 * variable exists.
 * Includes also look in the directory specified by the string
 * "Library" if they aren't present in the current directory.
 * Adaread also moves the Input_File to Fix_File so the fix;
 * procedure can edit the last command.
 *
 * Revision 1.1  82/12/12  21:39:43  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include <stdio.h>
# include "macros.h"
# include "nodes.h"
# include "int.h"
# include "parser.h"
# include "arr.h"
# define throw longjmp

extern FILE *yyin;
extern long yylineno, yycol;
extern long yytchar, yychar;
extern char *yysptr, yysbuf[];
extern char *Library, *Default_Editor;
extern char *tail();		/* returns last component of a pathname */


boolean append_next_string = false;	/* see parsestring below */
long *parsenv;                   /* for parse errors */
                                /* allows us to return gracefully from */
                                /* anywhere in the parser */
extern FILE *yyin;              /* input file for scanner (lex) */
extern long yylineno, yycol;     /* current line and col while parsing */

extern nodep parser();			/* the ada parser */
extern char *getenv();

 /* The following code has been removed because files are no longer
    nested during parsing.  An entire file is parsed into a list of items
    and executed.  Pragma include and pragma load are executed when
    they are encountered in the internal form, just like statements.  If
    files are to be nested, this code must be restored.  However, the
    code to save yytchar and yysbuf is wrong.  My understanding now is
    that yytchar should be pushed into yysbuf and then yysbuf saved.
    It may be necesary to save more than one character from yysbuf.
    This is just conjecture; I haven't actually tried it. (FT) */

# ifdef STACKFILES
struct inclform {
        FILE *filesave;
        struct inclform *incllink;
        long linesave;
        long colsave;
	char chsave;
};

struct inclform *inclstack;

long pushfile () {
    register struct inclform   *p;

    p = (struct inclform   *) zalloc (sizeof (struct inclform), false);
    p -> filesave = yyin;
    p -> incllink = inclstack;
    p -> linesave = yylineno;
    p -> colsave = yycol;
    p -> chsave = yytchar == 0 ? 0 : w_input ();
    yylineno = 1;
    yycol = 0;
    inclstack = p;
}

long popfile () {
    register struct inclform   *p;
    p = inclstack;
    if (p == null)
	sysputf ("include stack underflow");
    else {
	inclstack = p -> incllink;
	fclose (yyin);
	yyin = p -> filesave;
	yylineno = p -> linesave;
	yycol = p -> colsave;
	if (p -> chsave)
	    w_unput (p -> chsave);
	free (p);
    }
}
# endif STACKFILES

long err_row, err_col;
extern long yyprline, yyprcol;

/*VARARGS1*/
char *ada_perror(s,a,b,c,d,e)       /* come where if a parsing error occured */
long s, a, b, c, d, e; {
        static char error[256];
        sprintf(error,s,a,b,c,d,e);     /* translate error into simple string */
        err_row = yyprline;
        err_col = yyprcol;
        throw(parsenv, error);          /* and return to adaread */
}

/* Adaread opens a file and then calls the parser to parse it.  adaread1
   is an auxiliary function which allows the parser to abort from any level
   deep in the parsing process.  To do so, it calls perror which throws
   control to the catch in adaread.  adaread1 ensures that the error
   message returned to the catch is null in the case of a correct parse.

   The flag printerr controls whether the error function is called if the
   file is not present.  If it is true, then adaread can be called with
   only three arguments.  In this case, if the file cannot be opened then
   control will be thrown to the top level of Arcturus with an appropriate
   error message.  If it is false, then the flag opened is set to
   indicate whether the file was opened or not, and no error message
   will be returned in either case. */

char *adaread1(p, construct)
    nodep *p;
    enum parser_construct construct;
{
    *p = parser(construct);
    return null;
}

/*VARARGS4*/		/* for lint */
char *adaread(fn,p,construct,printerr,opened)
    char *fn;
    nodep *p;
    enum parser_construct construct;
    boolean printerr, *opened;
{
    char *s;
    FILE *f;
    char lib_file_name[256];

	/* The following block checks to see if the file requested exists
	   either in the local directory or in the site specific library
	   directory.

	   Also, note that we only look in the library directory if the name
	   specified was a simple name, that is, didn't have more than one
	   component to the path name.  So, include("user/bozo") won't look
	   in the library, but include("bozo") will.
	
	   The important thing here if you decide to change this is that it
	   is getting ridiculous, all the files you need to link to just
	   to start up the environment, so we need some sort of library type
	   mechanism.	(shw)
	*/

    f = fopen(fn, "r");
    if (not f) 		/* try again with library file */
	    if (strcmp(tail(fn), fn) eq 0) {
		sprintf(lib_file_name, "%s%s", Library, fn);
		f _ fopen(lib_file_name, "r");
	    }
    if (not f) {
	if (printerr)
	    error ("file not found: %s", fn);
	*opened = false;
	return null;
    }
    if (!printerr) *opened = true;
# ifdef STACKFILES
    pushfile();
# endif STACKFILES
    yyin = f;
    s = (char *)catch(&parsenv, adaread1, p, construct);
# ifdef STACKFILES
    popfile();
# endif STACKFILES
    fclose(f);
    return s;
}


 /* parsestring parses the supplied string.  If the parse was successful,
    p returns a pointer to the internal form.  If not, parsestring returns
    a pointer to an appropriate error message.

    The append_next_string global flag is  *horrible* style.  It's only
    excuse for existing is the only way I could think of to let command.c
    handle multi-line input without affecting Ray's interface.  I hope
    someone (or me) can do better soon. (FT) */

extern char *Input_File;
extern char *Fix_File;

char *parsething(s,p,construct)
    char *s;
    nodep *p;
    enum parser_construct construct;
{
    char   *ret;
    FILE *f;

    if (append_next_string) {
	f = fopen(Input_File, "a");
	append_next_string = false;
    }
    else {
	/* Save the current Input_File in Fix_File for use by "fix;"
	   procedure, which allows editing of the last input stream. */
	unlink(Fix_File);
	link(Input_File, Fix_File);	/* Don't worry if link fails. */
	unlink(Input_File);		/* Don't worry if unlink fails. */
					/* These would fail if Input_File
					   doesn't exist. */
        f = fopen (Input_File, "w");
    }
    if (f == null)
	error("Cannot write file %s", Input_File);
    while (*s)
	putc (*s++, f);
    fclose (f);
    ret = adaread(Input_File, p, construct, true);

    return (ret);
}

char *parsestring(s,p)
    char *s;
    nodep *p;
{
    return parsething(s,p,interactive_input);
}

       long incl(name, printerr, construct)
    char *name;
    boolean printerr;
    enum parser_construct construct;
{
    char ch, *s;
    nodep p;
    boolean opened;
    boolean first_edit = true;


	/* The while condition is rather strange for the following reason:
	   adaread only sets the opened flag if printerr is false because
	   the opened argument can be omitted if printerr is true.  If
	   printerr is true, adaread will not return if the input file
	   cannot be opened; rather it will call the error function which
	   throws control back to the top level.  Thus what this test says
	   is: if an error message is returned and either printerr was
	   set (indicating that the file must have been opened) or printerr
	   was not set, but opened was (indicating the same thing). */

    while ((s = adaread(name,&p,construct,printerr,&opened)) &&
	   (printerr || opened)) {
	if (!first_edit) {
	    putf("There is still a syntax error.\n");
	    putf("Cancel parser, or re-enter editor? Type e for editor: ");
	    ch = getchar();
	    if (ch != '\n') while (getchar() != '\n');
	    if (ch != 'e') {
	      putf("\nParser cancelled. If there was already an internal");
	      putf("\nversion of this item, it will still be around.\n");
	      return;
	    }
	}
	first_edit = false;
        if (!call_edit (name, s)) return;
    }
    if (printerr || opened) interp(p);
}

long include(name, printerr)
    char *name;
    boolean printerr;
{
    incl(name, printerr, compilation);
}

long command_include(name, printerr)
    char *name;
    boolean printerr;
{
    incl(name, printerr, interactive_input);
}



/* The following stuff is used to talk to an editor */

extern	char *Default_Editor;

/*	get_editor_name returns the full path name of the editor to call
	to edit stuff from the environment.  This is dug up from the shell
	environment variable EDITOR if there is one, otherwise the default
	(installation dependent) name that was compiled into Arcturus is used.
*/

char	*get_editor_name()
{	char *editor _ getenv("EDITOR");
	return editor ? editor : Default_Editor;
}

/* call_edit will call an editor, passing it x y coords (if possible)
   and an error message that can be printed at the bottom.  It returns
   true if the file should be re-included and false if not.  Right
   now this is used only if $EDITOR is none, but it could be used
   if the editor returned non-zero status.  */

extern char *AdaErrMl;

boolean call_edit(name,s)       /* s = error string */
    char *name, *s;
{
    char    command[256];
    char   *editor_path;
    char   *editor;

    editor_path = get_editor_name();
    editor _ tail(editor_path);	/* check last component of pathname */
    if (strcmp (editor, "vi") == 0) {
	    putf ("error: %s\n", s);
	    sleep (2L);
	    sprintf (command, "%s +%d %s", editor_path, err_row, name);
    }
    else if (strcmp (editor, "emacs") == 0) {
		write_help_file ();
		sprintf (command, "%s -l%s %d %d '%s' %s", editor_path,
			AdaErrMl, err_row, err_col, s, name);
    }
    else if ((strcmp (editor, "fu") == 0) || (strcmp (editor, "nf") == 0)
			|| (strcmp (editor, "ash") == 0)) {
		    sprintf (command, "%s -r%d -c%d \"-e%s\" %s", editor_path,
			    err_row - 1, err_col - 1, s, name);
    }
    else if (strcmp(editor,"none") == 0) {
		write_help_file();
		sysputf("%s on line %d, column %d", s, err_row, err_col);
		return false;
    }
    else {		/* unknown editor */
		putf ("Unknown editor; error: %s\n", s);
		sleep (2L);
		sprintf (command, "%s %s", editor_path, name);
    }
    system (command);
    return true;
}

extern char *Edit_File;
    /* edit has to pretty print the object to a file
       and then call the editor.  It then removes the temporary file. */
long edit(iform) 
    long *iform;  
{
    FILE * f;
    char   *editor;
    char    command[256];

    
    editor = get_editor_name ();
    if (strcmp(editor, "none") == 0) {
	putf("can't call an editor -- EDITOR environment variable is none\n");
	return;
    }
    f = fopen (Edit_File, "w");
    dopp (iform, f, true, true);
    fclose (f);
    sprintf (command, "%s %s", editor, Edit_File);
    system (command);
    include (Edit_File, true);
    unlink (Edit_File);
}

long parser_error(err)
    char *err;
{
    error_xy(err_row-1, err_col-1);     /* tell Ray where the error is */
    if (!append_next_string) sysputf("%s", err);	/* kludge -- see
					parsestring for details */
}

/* The following two procedures can be called in ada to load files of
   code.  include(...) loads an ada compilation unit.  This means (among
   other things) that use clauses that preceed a unit apply to that unit.
   command_include(...) loads a sequence of commands which behave
   as if they were typed to the ada top level. */


long ada_include(ada_file_name)
    arrayp ada_file_name;
{
    char file_name[80];

    str_ada_to_c(file_name, ada_file_name);
    include(file_name, true);
}

long ada_include_commands(ada_file_name)
    arrayp ada_file_name;
{
    char file_name[80];

    str_ada_to_c(file_name, ada_file_name);
    command_include(file_name, true);
}

long ada_load(ada_file_name)
    arrayp ada_file_name;
{
    char file_name[80];

    str_ada_to_c(file_name, ada_file_name);
    load_object(file_name);
}
