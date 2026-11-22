static char rcsid[] =
"$Header:   ../RCS/ppaux.v  Revision 1.2  83/01/12  09:08:29  ada  Exp$";

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


/* $Log:	../RCS/ppaux.v $
 * Revision 1.2  83/01/12  09:08:29  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:41:12  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include "macros.h"
# include <ctype.h>
# include <stdio.h>
# include "nodes.h"
# include "int.h"
# include "color.h"
# include "pp.h"

extern char *prattach ();

FILE * ppfile;
static boolean vbars, number;
static boolean print_attach, print_comment;

static  long indent = 0;
static long  linenum;
static long  pplevel;
static long  lastcut;
static long  blinesbefore;
static long  blinesafter;

static long  foldwidth,		/* wider lines will be folded */
            attachcol,		/* column in which attachments will start 
				*/
            maxattcol,		/* largest column in which attachments
				   will start */
            maxwidth;		/* maximum width of a line (including
				   attachments).  Note: the prettyprinter
				   doesn't split tokens, and there is no
				   warning if a line is wider than
				   maxwidth */
boolean pp_first;

# define Qsize 350L
# define Ssize 30L

enum qtype {
    qch, 			/* text character */
    qcut,			/* cut point (place where line can end) */
    qcol,			/* color change */
};

typedef struct {
    enum qtype ty: 4;
    unsigned    wid:4;
    unsigned    setindent:1;
    unsigned    indent:8;
    char        ch:8;
} Q;

struct S {
    long     indent;
    boolean setindent;
    boolean pack;
    boolean vbar;
    long     blines;
    boolean firstcut;
    nodep attach;
};

static struct S     liststk[Ssize];
static  Q queue[Qsize];
static  Q * get,
       *put,
       *last = &queue[Qsize-1];
static long  width,
            size;

long pp_init (mxwidth, attcol, pr_attach, pr_comment, vrt_bar, num)
    long     mxwidth,
            attcol;
    boolean pr_attach, pr_comment, vrt_bar, num;
{
    maxwidth = mxwidth;
    attachcol = attcol;
    foldwidth = maxwidth - 20;	/* try these values for now */
    maxattcol = foldwidth;

    print_attach = pr_attach;
    print_comment = pr_comment;
    vbars = vrt_bar;
    number = num;

    size = 0;
    width = 0;
    get = put = queue;
    pplevel = 0;
    lastcut = -1;
    indent = 0;
    linenum = 1;
    blinesbefore = blinesafter = 0;
    liststk[0].indent = 0;
    pp_first = true;
}


long ps () {				/* print stack -- for debugging only */
    long     sp,
            ind;
    char    f1,
            f2,
            f3;

    putchar ('\n');
    for (sp = pplevel; sp > 0; sp--) {
	ind = liststk[sp].indent;
	f1 = liststk[sp].setindent ? 's' : ' ';
	f2 = liststk[sp].pack ? 'p' : ' ';
	f3 = liststk[sp].vbar ? 'v' : ' ';
	putf("%3d. %4d %c%c%c\n", sp, ind, f1, f2, f3);
    }
}

long pq () {				/* print queue -- for debugging only */
    static  Q * x;
    long     s,
            w;
    enum qtype ty;
    long     wid,
            ind;
    char    ch;
    boolean si;
    boolean pfound, gfound;

    putchar ('\n');		/* force out any output */
    pfound = gfound = false;
    for (x = queue; x <= last; x++) {
	if (x == get)
	    gfound = true;
	if (x == put)
	    pfound = true;
    }
    if (!pfound)
	putf ("put bad\n");
    if (!gfound)
	putf ("get bad\n");
    if (!(pfound && gfound))
	return;
    if (get == put)
	putf ("queue is empty\n");
    else {
	x = get;
	s = size;
	w = width;
	while (size > 0) {
	    deq (&ty, &wid, &ch, &ind, &si);
	    switch (ty) {
		case qch:
		    putf ("   %4d   %c\n", wid, ch);
		    break;
		case qcut:
		    putf ("*  %4d %3d %3d %c\n", 
			wid, ch, ind, (si ? 's' : ' '));
		    break;
		case qcol:
		    putf ("!  %4d   %d\n", wid, ch);
		    break;
	    }
	}
	get = x;
	size = s;
	width = w;
    }
}

static  long enq (ty, wid, ch, ind, si)
    enum qtype ty;
    long     wid,
            ind;
    char    ch;
    boolean si;
{
    put -> ty = ty;
    put -> wid = wid;
    put -> ch = ch;
    put -> indent = ind;
    put -> setindent = si;
    if (size >= Qsize)
	sysputf ("** pp: queue overflow\n");
    else {
	if (put == last)
	    put = queue;
	else
	    put++;

	width += wid;
	++size;
    }
}

        long deq (ty, wid, ch, ind, si)
    enum qtype * ty;
    long    *wid,
           *ind;
    char   *ch;
    boolean * si;
{
    if (size == 0)
	sysputf ("** pp: queue underflow\n");
    else {
	*ty = get -> ty;
	*wid = get -> wid;
	*ch = get -> ch;
	*ind = get -> indent;
	*si = get -> setindent;
	if (get == last)
	    get = queue;
	else
	    get++;
	width -= *wid;
	size--;
    }
}

static  long lowcut (pos, cutlev)
    long    *cutlev,
           *pos;
{
    Q * p;
    long     i;

    p = put;
    *cutlev = 999999;
    *pos = size;
    for (i = 0; i < size; i++) {
	if (p == queue)
	    p = last;
	else
	    p--;
	if (p -> ty == qcut && p -> ch < *cutlev) {
	    *cutlev = p -> ch;
	    *pos = i;
	}
    }
}

static  long outattach (att, col)
    char   *att;
    long     col;
{
    char   *last;
    boolean first = true;

    do {
	if (strlen (att) > maxwidth - col - (first ? 2 : 3)) {
	    last = att + maxwidth - col - (first ? 2 : 3);
	    while (last > att && *last != ' ' && *last != '\t')
		last--;
	    if (last == att)
		last = att + maxwidth - col;
	}
	else
	    last = att + strlen (att);
	if (first)
	    first = false;
	else {
	    fputchr (ppfile, '\n');
	    doindent (col - 1);
	    fputchr (ppfile, '-');
	}
	fputchr (ppfile, '-');
	fputchr (ppfile, '-');
	while (att < last)
	    fputchr (ppfile, *att++);
    } while (*att != '\0');
}

static  long printattach (a, col, attachcol)
    nodep a;
    long     col,
            attachcol;
{
    long     i;
    long     comcol;
    char   *as;

    if (a != null) {
	if ((print_comment && a -> ndf.op == nop_comment) ||
		(print_attach && a -> ndf.op != nop_comment)) {
	    as = prattach (a);
	    if (as[0] == '.') {
		fputchr (ppfile, '\n');
		doindent (indent);
		outattach (as, indent);
	    }
	    else
		if (col > maxattcol) {
		    fputchr (ppfile, '\n');
		    doindent (attachcol - 1);
		    outattach (as, attachcol - 1);
		}
		else {
		    comcol = col < attachcol - 1 ? attachcol : col + 2;
		    for (i = col; i < comcol; i++)
			fputchr (ppfile, ' ');
		    outattach (as, comcol);
		}
	}
	for (a = a -> ndf.next; a != null; a = a -> ndf.next) {
	    if ((print_comment && a -> ndf.op == nop_comment) ||
		    (print_attach && a -> ndf.op != nop_comment)) {
		fputchr (ppfile, '\n');
		as = prattach (a);
		if (as[0] == '.') {
		    doindent (indent);
		    outattach (as, indent);
		}
		else {
		    doindent (attachcol - 1);
		    outattach (as, attachcol - 1);
		}
	    }
	}
    }
}

        long doindent (indent)
    long     indent;
{
    long     l,
            i;

    if (vbars) {
	if (number) {
	    if (linenum % 5 == 0)
	    	fputf (ppfile, "%5d  ", linenum);
	    else
	    	fputf (ppfile, "       ");
	    linenum++;
	}
	l = 0;
	for (i = 0; i < indent; i++) {
	    while (l <= lastcut - 1 && liststk[l].indent < i)
		l++;
	    while (l <= lastcut - 1 && liststk[l].indent == i &&
		    !liststk[l].vbar)
		l++;
	    if (liststk[l].indent == i && liststk[l].vbar)
		fputchr (ppfile, '|');
	    else
		fputchr (ppfile, ' ');
	}
    }
    else {
        if (number) {
	    if (linenum % 5 == 0)
	    	fputf (ppfile, "%5d  ", linenum);
	    else
	    	fputf (ppfile, "       ");
	    linenum++;
	}
	for (i = 7; i < indent; i += 8)
	    fputchr (ppfile, '\t');
	for (i = 0; i < indent % 8; i++)
	    fputchr (ppfile, ' ');
    }
}

static  long checkblanklines (lev)
    long     lev;
{
    long     l;

    for (l = 1; l <= lev; l++) {
	if (liststk[l].firstcut) {
	    if (liststk[l].blines > blinesbefore)
		blinesbefore = liststk[l].blines;
	    liststk[l].firstcut = false;
	}
    }
}


long doblanklines () {
    long     i;

    if (pp_first)
	pp_first = false;
    else {
	for (i = 0; i < blinesbefore; i++) {
	    fputchr (ppfile, '\n');
	    if (vbars)
		doindent (indent + 1);/* get vertical bars */
	}
	fputchr (ppfile, '\n');
    }
    blinesbefore = blinesafter;
    blinesafter = 0;
}

static  long lineout (pos, lev)
    long     pos;
    long     lev;
{
    enum qtype ty;
    long     wid,
            ind;
    char    ch;
    long     i;
    long     col;
    long     linecut;
    boolean si;

    if (lev < 99999)
	checkblanklines (lev);

    while (size > pos) {

	doblanklines ();
	doindent (indent);
	col = indent + 1;
	linecut = lev;

	while (size > pos) {
	    deq (&ty, &wid, &ch, &ind, &si);
	    if (ty == qch) {
		fputchr (ppfile, ch);
		col += wid;
	    }
	    else if (ty == qcol)
		set_fg(ch);
	    else {
		if (ch <= lev) {
		    if (si && size > pos) {
			long oldindent = liststk[ch].indent;
			long i;

			liststk[ch].indent = col - 1;
			for (i = ch+1; i <= pplevel; i++)
			    liststk[i].indent += col-1-oldindent;
		    }
		    if (!liststk[ch].pack) {
			linecut = ch;
			break;
		    }
		}
		for (i = 0; i < wid; i++)
		    fputchr (ppfile, ' ');
		col += wid;
	    }
	}
	lastcut = lev;
	indent = liststk[lastcut].indent;
	printattach (liststk[linecut].attach, col, attachcol);
	liststk[linecut].attach = null;
	while (size > 0 && (get -> ty == qcut || get->ty == qcol)) {
	    deq (&ty, &wid, &ch, &ind, &si);
	    if (ty == qcol)
		set_fg(ch);
	    else {
		if (lastcut > ch)
		    lastcut = ch;
   		if (ind > 0)
		    indent = ind;
	    }
	}
    }
}

long pp_stok(s, pre, post)		/* outputs a token with special effects.
				   pre and post are strings which are 
				   printed before and after the token,
				   respectively.  They should not take any
				   space (e.g. commands to start and stop
				   underlining).   */
    char   *s,
           *pre,
           *post;
{
    long     lev;
    long     pos,
            cutlev;

    if (strlen (s) + width + indent > foldwidth) {
	lowcut (&pos, &cutlev);
	lineout (pos, cutlev);
    }

    while (*pre)
	enq (qch, 0L, *pre++, 0L, false);
    while (*s)
	enq (qch, 1L, *s++, 0L, false);
    while (*post)
	enq (qch, 0L, *post++, 0L, false);
}

long pp_tok (t)
    char   *t;
{
    pp_stok (t, "", "");
}

long pp_set_color(col)
    color_name col;
{
    if (col != samecolor) {		/* don't put colors in queue */
        if (size == 0)
	    set_fg(col);
        else
	    enq (qcol, 0L, col, 0L, false);
    }
}

long pp_cut (sp) long sp; {
    cut (sp, 0L, false);
}
        long cut (sp, ind, si)
    long     sp,
            ind;
    boolean si;
{
    long     i;

    if (size == 0) {
	if (lastcut > pplevel)
	    lastcut = pplevel;
	if (!liststk[pplevel].pack)
	    indent = liststk[pplevel].indent;
    }
    else {
	enq (qcut, sp, pplevel, ind, si);
	if (pplevel <= lastcut && !liststk[pplevel].pack)
	    lineout (0L, pplevel);
    }
}

long pp_cleanup (endnl)
boolean endnl;
{
    if (size > 0)
	lineout (0L, 0L);
    if (endnl)
	fputchr (ppfile, '\n');
}

long pp_begin (p, ind, bl, si, vb, pk)
    nodep p;
    long     ind,
            bl;
    boolean si, vb, pk;
{
    pplevel++;
    liststk[pplevel].indent = liststk[pplevel - 1].indent + ind;
    liststk[pplevel].setindent = si;
    liststk[pplevel].vbar = vb;
    liststk[pplevel].pack = pk;
    liststk[pplevel].attach = p ? p -> ndf.ag : null;
    liststk[pplevel].blines = bl;
    liststk[pplevel].firstcut = true;
    cut (0L, pk ? 0 : liststk[pplevel].indent, si);
}

static boolean test_attach(p)
    nodep p;
{
    while (p) {
	if ( (p->ndf.op == nop_comment) ? print_comment : print_attach)
	    return true;
	p = p->ndf.next;
    }
    return false;
}

long pp_end ()
{
    long     lev;
    long     pos,
            cutlev;

    if (test_attach(liststk[pplevel].attach))
	while (size > 0) {
	    pp_cut (0L);
	    lowcut (&pos, &cutlev);
	    lineout (pos, cutlev);
	}
    if (liststk[pplevel].blines > blinesafter)
	blinesafter = liststk[pplevel].blines;
    pplevel--;
}

long pp_space () {
    enq (qch, 1L, ' ', 0L, false);
}

  /* capitalizes s according to the requested style.  s must be all lower
     case. */

char *pp_capitalize(s, cap)
    register char *s;
    enum pp_capitalize cap;
{
    static char str[100];
    register char *ns = str;

    switch (cap) {
	case ppcap_lower:		/* all lower case */
	    return s;
    	case ppcap_upper:
	    while (*s)
		*ns++ = islower(*s) ? *s++ + 'A' - 'a' : *s++;
	    *ns = '\0';
	    return str;
	case ppcap_initial:
	    *ns++ = islower(*s) ? *s++ + 'A' - 'a' : *s++;
	    while (*s) *ns++ = *s++;
	    *ns = '\0';
	    return str;
	case ppcap_mixed: {
	    boolean cap_next = true;
	
	    while (*s) {
		*ns++ = (islower(*s) && cap_next) ? *s + 'Q' - 'q' : *s;
		cap_next = (*s++ == '_');
	    }
	    *ns = '\0';
	    return str;
	}
    }
}
