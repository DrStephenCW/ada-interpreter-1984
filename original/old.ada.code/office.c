static char rcsid[] =
"$Header:   ../RCS/office.v  Revision 1.2  83/01/12  09:07:27  ada  Exp$";

/*******************************************************************
 *
 * Copyright 1983      Thomas A. Standish
 *		       Programming Environment Project
 *                     Department of Information and Computer Science
 *                     University of California
 *                     Irvine, California  92717
 *
 * All rights reserved. No part of this software may be sold or distributed
 * in any form or by any means without the prior written permission of the 
 * author.
 *
 * Report problems and direct all inquiries to ada%uci@rand-relay (ARPA net).
 */


/* $Log:	../RCS/office.v $
 * Revision 1.2  83/01/12  09:07:27  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:38:17  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <pwd.h>
#include "arr.h"
#include "macros.h"

extern char *getenv();

struct passwd *getpwnam();

static remove_blanks(s)
char *s;
{
        register int i = strlen(s) - 1;
        while ((s[i] eq ' ') and (i)) {
                s[i] _ 0;
                i--;
        };
}

realname(username,fullname)
arrayp username,fullname;
{
        struct passwd *pass;
        char tempname[256];
        str_ada_to_c(tempname,username);
        remove_blanks(tempname);
        pass _ getpwnam(tempname);        
        if (not pass)  error("Username not in password file");
                                /* should return true or false or something */
        str_c_to_ada(fullname,pass->pw_gecos);
}

username(astr)          /* string(1..8) */      /* temp string */
arrayp astr;
{
        register int len, i;
        char *user;

        user _ getenv("USER");  
        if (not user)  error("User name not available in csh environment");
        if (astr->arr_size neq 8)  error("Username expects string(1..8)");

        str_c_to_ada(astr,user);
} 

str_c_to_ada(astr,cstr)         /* copy c string into ada string array */
arrayp astr;
char *cstr;
{
        register int i;
        register int len _ strlen(cstr);        /* length of C string */
        register int size _ astr->arr_size;     /* size of ada array */

        if (len > size)  error("String overflow (copying c str to ada)");

        for (i _ 0 ; i < len; i++)              /* copy non-blank part */
                astr->arr_base[i] _ cstr[i];

        for (i _ len; i < size; i++)            /* fill in rest with blanks */
                astr->arr_base[i] _ ' ';

}

rightnow()              /* return seconds since 00:00:00 GMT Jan 1. 1971 */
{
        return time(0);
}

