/* $Header:   ../RCS/color.h.v  Revision 1.2  83/01/12  09:03:35  ada  Exp$ */

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


/* $Log:	../RCS/color.h.v $
Revision 1.2  83/01/12  09:03:35  ada
Added copyright notice.

Revision 1.1  82/12/12  21:31:24  tadman
Initial revision (Arcturus release 1.3E)
 */

typedef enum {
        black,
        blue,
        red,
        green,
        yellow,
        magenta,
        cyan,
        white,
	samecolor,
} color_name;

typedef struct {
        char chval;
        char fcolor;
        char bcolor;
	char protected;
}       fchar;

/*
#define achar long
#define achar_compat_type long

#define char_of(x) ((char)access_char(x))
#define fgcolor_of(x) ((color_name)access_fcolor(x))
#define bgcolor_of(x) ((color_name)access_bcolor(x))
#define protected_of(x) (access_protected(x))

#define assign_char_of(x,y) assgn_char_of(&(x),y)
#define assign_fcolor_of(x,y) assgn_fcolor_of(&(x),y)
#define assign_bcolor_of(x,y) assgn_bcolor_of(&(x),y)
#define assign_protected_of(x,y) assgn_protected_of(&(x),y)
*/

#define achar fchar
#define achar_compat_type long

#define char_of(x) ((x).chval)
#define fgcolor_of(x) ((color_name)((x).fcolor))
#define bgcolor_of(x) ((color_name)((x).bcolor))
#define protected_of(x) ((x).protected)

#define assign_char_of(x,y) ((x).chval = (y))
#define assign_fcolor_of(x,y) ((x).fcolor = (char)(y))
#define assign_bcolor_of(x,y) ((x).bcolor = (char)(y))
#define assign_protected_of(x,y) ((x).protected = (y))

#define color_to_bg(c) ((int)(c) + 20)
#define color_to_fg(c) ((int)(c) + 10)


#define set_fg(f) set_both(f,cur_color.bg)
#define set_bg(b) set_both(cur_color.fg,b)

typedef struct {
        color_name fg,bg;
}       color_pair;

color_pair  cur_color;
