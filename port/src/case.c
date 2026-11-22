static char rcsid[] =
"$Header:   ../RCS/case.v  Revision 1.2  83/01/12  09:11:28  ada  Exp$";

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


/* $Log:	../RCS/case.v $
 * Revision 1.2  83/01/12  09:11:28  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:17:35  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <stdio.h>
#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"
#include "tab.h"
#include "eval.h"
#include "case.h"

extern nodep intstk[], *intsp, *intstk_end;

                                        

struct choice_item *case_choice_stack = null;   
                                        /* store list of used choice items */
                                        /* so we don't have to use zalloc  */
                                        /* and free, which are expensive.  */
struct choice_item *dupli_check(); 


/* Keep used choice_items in a list for reuse when we're done with them. */

long free_valid_choice_list(ptr)
struct choice_item *ptr;
{       if (ptr) {
                free_valid_choice_list(ptr->next_choice_item);
                ptr->next_choice_item = case_choice_stack;
                case_choice_stack = ptr;
        }
}


long runcase(p)
    register nodep p;
                        /* interpret a case statement */
{   switch(p->ndf.op) {
        case nop_case: {
                typedval r;
                register nodep q,m,x;
                struct choice_item *valid_choice_list,*new_choice;
                long choice_count;
                boolean all_choices_accounted_for;

                x = null;
                choice_count = 0;
                valid_choice_list = null;
                all_choices_accounted_for = false;

                eval(p->nd.casex.expr, eval_mode, &r, null);
                if (r.typ->tyf.class != srangeform
                    && r.typ->tyf.class != enumform
                    && r.typ->tyf.class != intform)   
                        error("Illegal expression type in case stmt\n");
                for (q = p->nd.casex.when_list; q != null; q = q->ndf.next)
                {
                  for (m = q->nd.whenx.choice_list; m != null;
                              m = m->ndf.next)
                  {
                    if (m->nd.choice.choice == null) 
                    {
                      if (q->ndf.next == null)
                      {
                        if (x == null) x = q->nd.whenx.stmt;
                        all_choices_accounted_for = true;
                      }
                      else error("illegal OTHERS choice in case stmt");
                    }
                    else
                    {
                       if (case_choice_stack)
                       {  new_choice = case_choice_stack;
                          case_choice_stack = case_choice_stack->next_choice_item;
                          new_choice->next_choice_item = null;

                       }
                       else
                         new_choice = (struct choice_item *)zalloc(sizeof(struct choice_item),true);
                       eval(m->nd.choice.choice,eval_mode,&new_choice->choice_sel, null);
                       if (new_choice->choice_sel.typ == typetype)
                       {
                         typecheck(r.typ,new_choice->choice_sel.val.tptr);
                         if (x == null && (r.val.integer >= 
                             new_choice->choice_sel.val.tptr->ty.srange.lbound &&
                             r.val.integer <= new_choice->choice_sel.val.tptr->ty.srange.ubound))
                           x = q->nd.whenx.stmt;
                       }
                       else
                       {
                         typecheck(r.typ,new_choice->choice_sel.typ);
                         if (x == null && r.val.integer == new_choice->choice_sel.val.integer)
                            x = q->nd.whenx.stmt;
                       }
                       valid_choice_list = dupli_check(new_choice, valid_choice_list); 
                       if (new_choice->choice_sel.typ == typetype)
                         choice_count += (new_choice->choice_sel.val.tptr->ty.srange.ubound
                          - new_choice->choice_sel.val.tptr->ty.srange.lbound +1);
                        else choice_count++;
                    }
                  }
                }
                if (all_choices_accounted_for ||
                     (choice_count == rangelength(r.typ)))
                                eval(x, eval_mode, &dummy, null);
                else error("Non-exhaustive choices in case stmt\n");
              free_valid_choice_list(valid_choice_list);
              break;
            }
        default: error("whoops in docase");
    } 
} /* end runcase */

long docase(p)
    nodep p;
{
    struct retstkform   rets;

    do_comp_statement(&rets, p, runcase);
}


struct choice_item *dupli_check(n_choice,v_c_list)
    struct choice_item *n_choice, *v_c_list;
{
   struct choice_item *n, *nt;


   for (n = v_c_list; n != null; n = n->next_choice_item)
   {
      if (n_choice->choice_sel.typ == typetype && n->choice_sel.typ == typetype)
      {
         if ((n_choice->choice_sel.val.tptr->ty.srange.lbound
                         >= n->choice_sel.val.tptr->ty.srange.lbound
              && n_choice->choice_sel.val.tptr->ty.srange.lbound
                         <= n->choice_sel.val.tptr->ty.srange.ubound)
             ||
              (n_choice->choice_sel.val.tptr->ty.srange.ubound
                         >= n->choice_sel.val.tptr->ty.srange.lbound
               && n_choice->choice_sel.val.tptr->ty.srange.ubound
                         <= n->choice_sel.val.tptr->ty.srange.ubound))  case_error1();
      }
      else
      if (n_choice->choice_sel.typ == typetype)
      { 
         if (n->choice_sel.val.integer >= n_choice->choice_sel.val.tptr->ty.srange.lbound
             && n->choice_sel.val.integer <= n_choice->choice_sel.val.tptr->ty.srange.ubound)
           case_error1();
      }
      else
      if (n->choice_sel.typ == typetype)
      {
         if (n_choice->choice_sel.val.integer >= n->choice_sel.val.tptr->ty.srange.lbound
             && n_choice->choice_sel.val.integer <= n->choice_sel.val.tptr->ty.srange.ubound)
           case_error1();
      }
      else
      if (n_choice->choice_sel.val.integer == n->choice_sel.val.integer)
        case_error1();
      nt = n;
   }
   if (v_c_list == null) v_c_list = n_choice;
   else nt->next_choice_item = n_choice;
   return(v_c_list);
}



long case_error1()  {
   error("choice duplication in case stmt\n");
}
