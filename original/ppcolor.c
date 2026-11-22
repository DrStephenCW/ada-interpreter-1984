static char rcsid[] =
"$Header:   ../RCS/ppcolor.v  Revision 1.2  83/01/12  09:08:41  ada  Exp$";

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


/* $Log:	../RCS/ppcolor.v $
 * Revision 1.2  83/01/12  09:08:41  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:41:29  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include "macros.h"
# include "nodes.h"
# include "int.h"
# include "count.h"
# include "color.h"
# include "pp.h"

extern pp_count_init();
extern color_name pp_count_color();
extern int pp_depth;

color_name pp_color(p,c_use)
    nodep p;
    enum pp_color_use c_use;
{
    switch (c_use) {
        case ppc_none:
            return samecolor;
        case ppc_count:
                return pp_count_color(max_node_counts > 0 ?
                           (get_count(p) *100) / max_node_counts : 0);
        case ppc_random:
            {
                color_name color;

                do {
                    color = (color_name)(rand() % 8);
                } while (color == (color_name)cur_color.bg);
                return color;
            }
        case ppc_circular:
            {
                color_name color;
                static int color_num = 0;

                do {
                    color = (color_name)(color_num++ % 8);
                } while (color == (color_name)cur_color.bg);
                return color;
            }
        case ppc_depth:
            {
                int color_num;

                color_num = pp_depth % 7;
                if (color_num >= (int)cur_color.bg) color_num++;
                return (color_name)color_num;           
            }
        default:
            error("pp: invalid color usage (#%d)", (int)c_use);
    }
};

pp_color_init(color_use)
    enum pp_color_use color_use;
{
    switch (color_use) {
        case ppc_count:
            pp_count_init();
            break;
        default:
            break;
    }
}

