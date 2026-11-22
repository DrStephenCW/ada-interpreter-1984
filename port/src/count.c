static char rcsid[] =
"$Header:   /ub/darpa/ada/RCS/count.v  Revision 1.5  84/01/13  01:46:18  ada  Exp$";

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


/* $Log:	/ub/darpa/ada/RCS/count.v $
 * Revision 1.5  84/01/13  01:46:18  ada
 * put reference in for pp_count_color()
 * 
 * Revision 1.4  83/12/13  17:51:04  ada
 * changed order of threshold colors.
 * 
 * Revision 1.2  83/01/12  09:13:03  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:21:32  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

# include <stdio.h>
# include "adaio.h"
# include "macros.h"
# include "nodes.h"
# include "int.h"
# include "sym.h"
# include "tab.h"
# include "eval.h"
# include "count.h"

long max_node_counts = 0;

extern long nill();  /* function that does nothing */
extern nodep find_attach();

static long incr_count(cnt_attach)
    nodep cnt_attach;
{
    (cnt_attach->nd.counter.count)++;
    if (cnt_attach->nd.counter.count > max_node_counts)
        max_node_counts = cnt_attach->nd.counter.count;
}

long increment_node_counter(p) 
register nodep p;
{
    if (apply_attach(p,incr_count,nop_count) == 0) {
        nodep n = newnode(counter_node, nop_count);
        insert_attach(p,n);
        incr_count(n);
    }
}

long zero_counter(ptr)
    nodep ptr;
{
    remove_attach(ptr,nop_count);
}

long clear_counters()
{
    register long i, n;
    register symtabp tab;
    register fixp f;
    max_node_counts = 0;

    for (tab = currtab; tab; tab = tab -> symtab_static_link) {
	n = tab -> symtab_size - 1;
	for (i = 0; i < n; i++)
	    for (f = tab -> symtab_tab[i]; 
		 f; 
		 f = f -> fxf.next) {
		if (f -> fxf.class == func_id || f -> fxf.class == proc_id) {
		    if (f -> fx.func.int_form)
			if (get_count (f -> fx.func.int_form))
			    maptree (f -> fx.func.int_form,
				    zero_counter, nill, nill, nill);
		}
	    }
    }
}
		

long get_count(ptr)
nodep ptr;
{
    ptr = find_attach(ptr, nop_count);
    return (ptr ? ptr->nd.counter.count : 0);
}


color_name pp_count_color();


long write_color_key(fildes)
FILE *fildes;
{
            set_fg(cyan);           /* set forground to cyan       */
            set_bg(black);          /* set background to black     */
            fputf(fildes,"Threshold:   ");
            set_fg(black);
            set_bg(pp_count_color(0L));
            fputf(fildes,"  0   ");
            set_bg(pp_count_color(20L));
            fputf(fildes,"  20  ");
            set_bg(pp_count_color(40L));
            fputf(fildes,"  40  ");
            set_bg(pp_count_color(60L));
            fputf(fildes,"  60  ");
            set_bg(pp_count_color(80L));
            fputf(fildes,"  80  ");
            set_fg(cyan);
            set_bg(black);                                           
            fputf(fildes,"   %% of peak count\n");
}

long pp_count_init()
{
        if (color_able()) {
                set_give_tool_name(false);
                        /* turn off autoprinting of name */
                write_color_key(adamsg);
                write_color_key(adaout);
                set_give_tool_name(true);
                        /* turn it back on */
        }
}

color_name pp_count_color(scaled_count)
    long scaled_count;
{
    if (scaled_count < 20)      return blue;
    else if (scaled_count < 40) return green;
    else if (scaled_count < 60) return yellow;
    else if (scaled_count < 80) return magenta;
    else                        return red;
};
