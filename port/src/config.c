static char rcsid[] = 
"$Header:   /ub/darpa/ada/RCS/config.v  Revision 1.5  83/09/21  09:48:57  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/config.v $
 * Revision 1.5  83/09/21  09:48:57  ada
 * same as 1.4.5.1
 * 
 * Revision 1.4.5.1  83/09/20  16:09:32  snider
 * added pagesize.
 * 
 * Revision 1.4  83/07/14  11:59:04  ada
 * updated paths to reflect new disk partitions.
 * 
 * Revision 1.3  83/01/14  11:38:48  ada
 * same as 1.2.2.1  
 * 
 * Revision 1.2.2.1  83/01/14  11:28:20  willson
 * Added Mail_Program string.
 * 
 * Revision 1.2  83/01/12  09:12:40  ada
 * Added copyright notice.
 * 
 * Revision 1.1  83/01/04  14:30:05  willson
 * Initial revision
 *  */

/************************************************************************/
/*									*/
/*		The Arcturus Programming Environment was		*/
/*		produced at the Programming Environment Project		*/
/*		under the Direction of Professor Thomas A. Standish 	*/
/*									*/
/*		Department of Information and Computer Science		*/
/*		University of California, Irvine			*/
/*		Irvine, Ca. 92717					*/
/*									*/
/*		Written by:						*/
/*			Scott C. Auchmoody				*/
/*			Raymond Klefstad III				*/
/*			Craig Snider					*/
/*			Frank Tadman					*/
/*			Steve Whitehill					*/
/*			Stephen H. Willson				*/
/*									*/
/************************************************************************/
/* This work was sponsored in part by the Defense Advanced Research
   Projects Agency of the United States Department of Defense under
   contract MDA-903-82-C-0039 to the Irvine Programming Environment
   Project. The views and conclusions contained herein are those of the
   author and should not be interpreted as necessarily representing the
   official policies, either expressed or implied, of the Defense
   Advanced Research Projects Agency or the United States
   Government. 								*/
/************************************************************************/
/*									*/
/*	See CREDITS.md.  [2026: copyright line replaced]		*/
/*									*/
/************************************************************************/

/*									*/
/*	Arcturus Programming Environment configuration information 	*/
/*									*/

char	*Default_Editor = "vi";	/* port: was "/ub/darpa/bin/nf" */
			/* Editor to use if the shell environment variable
			   EDITOR isn't set.
			*/

#ifndef ARCTURUS_LIB
#define ARCTURUS_LIB "lib/"
#endif
char	*Library = ARCTURUS_LIB;	/* port: was "/ub/darpa/ada/cur/" */
			/* Arcturus looks in this directory for library files
			   if they aren't present in the local directory.
			*/
char	*AdaErrMl =  "/ub/darpa/ada/cur/adaerr.ml";
			/* File containing special Emacs Mlisp code used for
			   reporting parser lookahead set on syntax errors
			*/

char	*Mail_Program = "mail ucivax\!ada";
			/* The commands bug, and question, and the auto-
			   matic bug reports use this string to construct
			   a command to send mail to the maintainer
			   of your system to report 
			   a problem.  The string will look like this:

				Mail_Program < filename

			   The "<" and the filename are supplied by Arcturus.
			   You should set the string Mail_Program to be a
			   string which invokes a local mail program and
			   includes the name of the local maintainer.  Make
			   sure that your mail program can read from standard
			   input.
			*/

char	*Input_File = ",ada.input";
			/* File used to store text input from the terminal
			   for use by Yacc parser.
			*/

char	*Fix_File = ",ada.fix";
			/* File used by "fix;" subroutine to allow editing
			   of last command.
			*/
char	*Edit_File = ",ada.edit";
			/* File used to edit internal Arcturus objects: they
			   are pretty printed to this file, and then re-
			   included.
			*/

int 	pagesize = 1024;
			/* Used by save and restore.  The initialized data
			   segment starts on a page boundary, and pagesize
			   is used to calculate this address from etext, 
			   the end of the program text segment.
			*/
