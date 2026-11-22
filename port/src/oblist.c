static char rcsid[] =
"$Header:   /usr/ics/darpa/ada/RCS/oblist.v  Revision 1.4  83/05/25  15:29:30  ada  Exp$";

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


/* $Log:	/usr/ics/darpa/ada/RCS/oblist.v $
 * Revision 1.4  83/05/25  15:29:30  ada
 * same as 1.3.6.1
 * 
 * Revision 1.3.6.1  83/05/25  15:19:11  klefstad
 * "strapnd" was "strcat"ing into a string that was possibly relocated
 * by 'realloc,' so I changed the "s" to a "p" in the call to 'strcat.'
 * 
 * Revision 1.3  83/05/06  14:39:17  ada
 * same as 1.2.4.1
 * 
 * Revision 1.2.4.1  83/04/12  12:04:49  tadman
 * Added a new function strapnd which appends a string to an already
 * strsav'ed string.  It does this by reallocing the string.
 * 
 * Revision 1.2  83/01/12  09:07:23  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:37:57  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include "oblist.h"

/*
 * strsav(s) char *s    saves string in a unique place
 *                      and returns pointer to it.
 *                      can be deallocated with free()
 * strapnd(s, t) char *s, *t  appends string "t" to string "s"
 *			which must have been saved with strsav().
 *			returns a pointer to a possibly new string
 *			and s is no longer a valid string (i.e. it
 *			reallocs s.  Don't use any pointers left
 *			pointing to the old s and don't use this
 *			on interned strings.
 * intern(s) char *s    if this string has already been
 *                      interned return old pointer
 *                      otherwise return strsave(s)
 *                      don't use free() on these.
 * hashval(s) char *s   returns the hash value of
 *                      of interned identifier s
 *                      (in range 0..hashsize-1)
 */
#define hashsize 256L   /* can't be larger cause stored in byte */

struct ob_form {
        char    *ob_string;
        struct ob_form *ob_link;
};

typedef struct ob_form ob_type;
                               
static ob_type *hash_table[hashsize];

char *strsav(s)
char *s;
{
        char *p;
        long size;

        size = strlen(s) + 1;
        p = (char *)zalloc(size, 0L);
        strcpy(p,s);
        return(p);
}
                               
char *strapnd(s, t)
    char *s, *t;
{
    register long size = strlen(s) + strlen(t) + 1;
    register char *p;

    p = (char *)realloc(s, size);	/* doesn`t use zalloc so
					   not counted */
    strcat(p, t);
    return p;
}

char *intern(s)
char *s;
{
        long     h;
        ob_type *p;
        long     len;
        char    *q;

        h = hash(s);
        p = hash_table[h];
        while(p) {
                if(strcmp(s, p->ob_string)==0)
                        return(p->ob_string);
                p = p->ob_link;
        }
        p = (ob_type *)zalloc(sizeof(ob_type), 0L);
        p->ob_link = hash_table[h];
        hash_table[h] = p;
        len = strlen(s) + 2;   /* byte for null and hash val */
        q = (char *)zalloc(len, 0L);
        *q++ = h;
        strcpy(q, s);
        return(p->ob_string = q);
}

long hashval(s)
char *s;
{
        return(s[-1]&0377);
}

       long hash(s)
char *s;
{
        long r = 0;
        char *t = s;

        while(*t)
                r = r + *t++;
        return(r & (hashsize - 1));
}
