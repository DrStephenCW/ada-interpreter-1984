static char rcsid[] =
"$Header:   ../RCS/ppcolor.v  Revision 1.2  83/01/12  09:08:41  ada  Exp$";

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

extern long pp_count_init();
extern color_name pp_count_color();
extern long pp_depth;

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
                static long color_num = 0;

                do {
                    color = (color_name)(color_num++ % 8);
                } while (color == (color_name)cur_color.bg);
                return color;
            }
        case ppc_depth:
            {
                long color_num;

                color_num = pp_depth % 7;
                if (color_num >= (long)cur_color.bg) color_num++;
                return (color_name)color_num;           
            }
        default:
            error("pp: invalid color usage (#%d)", (long)c_use);
    }
};

long pp_color_init(color_use)
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

