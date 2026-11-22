static char rcsid[] =
"$Header:   /usr/ics/darpa/ada/RCS/error.v  Revision 1.3  83/01/14  11:38:15  ada  Exp$";

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


/* $Log:	/usr/ics/darpa/ada/RCS/error.v $
 * Revision 1.3  83/01/14  11:38:15  ada
 * same as 1.2.2.1  
 * 
 * Revision 1.2.2.1  83/01/14  11:27:32  willson
 * Added Mail_Program string.
 * 
 * Revision 1.2  83/01/12  09:14:16  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:23:51  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include <stdio.h>
# include <sgtty.h>
# include "macros.h"
# include "version.h"

extern	char	*Mail_Program;

 /* fatal_error is for errors like segmentation fault which are not
    continuable.  It just prints the message and then gets an automatic
    bug report.  The health variable is to try to prevent infinite
    loops.  When this routine is entered, heath is downgraded to sick
    and cleanup is attempted.  If this attempt causes another failure
    health is downgraded again to dead and no more cleanup is attempted */

long fatal_error(msg,a,b,c,d,e) 
    char *msg;
long a, b, c, d, e; {       
    static enum { alive, sick, dead } health = alive;   
    char fmt[100], err_mess[200];

    if (msg) {                  /* if called with null argument wont print
                                   anything. this way calling routine can
                                   do all the printing. */
        strcpy (fmt, "FATAL ERROR: ");
        strcat (fmt, msg);
        sprintf (err_mess, fmt, a, b, c, d, e);
        sysputf (err_mess);
    }

    if (health == alive) {
        health = sick;
        uui_term ();            /* finish up any interface processing */
        dumpusr ();                     /* dump "usr" symbol table */
    }
  
    if (health == sick) {
        health = dead;
        automatic_bug_report (err_mess);
    }

     /* at this point, we could re-execl the system */
    exit (1L);                   /* exit and close files */
}

/*      System_error prints an error message and calls the break package.
        Before doing so, it calls automatic_bug_report and asks for 
        information about the error.  It is
        not possible to continue at this point: if it we allowed that,
        most of our code that assumes error doesn't return won't work. 
        Implementors should use this call instead of "error" for reporting
        unexpected inconsistencies in the interpreter.
*/

/*VARARGS1*/		/* for lint */
long system_error(msg,a,b,c,d,e) long msg, a, b, c, d, e; {       
        char fmt[100], err_mess[200];

        if (msg) {      /* if called with null argument won't print
                        anything; this way calling routine can do all the
                        printing. */
                strcpy(fmt,"INTERNAL ERROR: ");
                strcat(fmt, msg);
                sprintf(err_mess, fmt, a,b,c,d,e);
                sysputf(err_mess);
        }
        automatic_bug_report(err_mess);
        ada_break(true);	/* recoverable, because the user may wish
				   to try to execute some commands before
				   exiting. */
        exit(1L);
}

/*      Error prints an error message and calls the break package.  It is
        not possible to continue at this point: if it we allowed that,
        most of our code that assumes error doesn't return won't work. 
*/

/*VARARGS1*/		/* for lint */
long error(msg,a,b,c,d,e) 
    char *msg;
long a, b, c, d, e; {
    char    fmt[100];

    if (msg) {			/* if called with null argument won`t
				   print anything; this way calling
				   routine can do all the printing. */
	strcpy (fmt, "Error: ");
	strcat (fmt, msg);
	sysputf (fmt, a, b, c, d, e);
    }

    ada_break (false);		/* not recoverable */
}

/* set term ensures that we have:
        echoing of input characters, 
        buffering and editing of input, 
        and ^M translated to linefeed.
*/

#define save_term(termsave) gtty(0,&termsave)
#define reset_term(termsave) stty(0,&termsave)
#define set_term(termsave, newterm)  \
{ \
        newterm = termsave;  \
        newterm.sg_flags |= ECHO; \
        newterm.sg_flags &= ~CBREAK; \
        newterm.sg_flags |= CRMOD; \
        stty(0,&newterm); \
}


static struct sgttyb tsave,newt;

long set_users_terminal()
{
	save_term(tsave);
	set_term(tsave,newt);
}

long reset_users_terminal()
{
	reset_term(tsave);
}

long get_user_comment(prompt,f)
    char *prompt;
    FILE *f;
{
        char s[200];

        set_users_terminal();

	fputf(standard_output(),"%s",prompt);

        while (gets(s) != NULL) {
		/* old code: fgetstr(standard_input(),s) didn't work. */
            fprintf(f, "%s\n", s);
        }

	reset_users_terminal();

	sysputf("Sending message, please wait ...");
}

/* This function mails string "s" to a mail box somewhere.  The current
	destination and delivery system should be derived by examining
	the code.  */

long automatic_bug_report(s)
    char *s;
{
    char    mail_command[80],
            file_name[80];
    FILE * f;

    if (strcmp (systemtype, "release") != 0) {
	printf ("Not a released version of %s. No automatic bug reports.\n",
		systemname);
	return;
    }
    sprintf (file_name, "/tmp/arcturus.%d", getpid ());
    if (!(f = fopen (file_name, "w"))) {
	printf ("Can't open %s for bug report\n", file_name);
	return;
    }
    fprintf (f, "Automatic bug report for %s %s %d.%d-%c   %s\n\n",
	    systemname, systemtype, majorversion,
	    minorversion, fixversion, sysdate);
    fprintf (f, "Error message: %s\n\n", s);

    get_user_comment (
	"Please comment on how you got this error (end with ^D):\n", f);
    fclose (f);
    sprintf (mail_command, "%s < %s", Mail_Program,
	    file_name);
    system (mail_command);
    unlink (file_name);
    sysputf ("Mail sent.");
}

        /* bug asks the user for a description of the bug and mails it
           to ada-bugs. */

long bug()
{
        char file_name[80], mail_command[80];
        FILE *f;

        sprintf(file_name, "/tmp/arcturus.%d", getpid());
        if (!(f = fopen(file_name, "w"))) {
		printf("Can't open %s for bug comment\n", file_name);
		return;
	}
        fprintf(f, "Bug report for ");
        fprintf(f, "%s %s %d.%d-%c   %s\n\n", systemname, systemtype,
		majorversion, minorversion, fixversion, sysdate);
        
        get_user_comment("Please describe the bug (end with ^D):\n", f);
        fclose(f);
        sprintf(mail_command, "%s < %s", Mail_Program, file_name);
        system(mail_command);
	unlink(file_name);
        sysputf("Mail sent.");
}

        /* question, while not strictly an error procedure is similar
           to bug, so it is included here.  It asks the user for his/her
           questions and mails it to ada-questions */

long question()
{
        char file_name[80], mail_command[80];
        FILE *f;

        sprintf(file_name, "/tmp/arcturus.%d", getpid());
        if (!(f = fopen(file_name, "w"))) {
		printf("Can't open file %s for question\n",file_name);
		return;
	}
        fprintf(f, "Question on  %s %s %d.%d-%c   %s\n\n", systemname, 
	    systemtype, majorversion, minorversion, fixversion, sysdate);
        get_user_comment("Please type your question (end with ^D):\n",f);
        fclose(f);
        sprintf(mail_command, 
            "%s < %s", Mail_Program, file_name);
        system(mail_command);
	unlink(file_name);
        putf("\nMail sent.\n");
}
