static char rcsid[] =
"$Header:   /usr/ics/darpa/ada/RCS/pragint.v  Revision 1.3  83/06/14  16:32:09  ada  Exp$";

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


/* $Log:	/usr/ics/darpa/ada/RCS/pragint.v $
 * Revision 1.3  83/06/14  16:32:09  ada
 * Same as 1.2.4.1.
 * 
 * Revision 1.2.4.1  83/06/14  14:14:18  tadman
 * Deleted pragma include and pragma load.
 * 
 * Revision 1.2  83/01/12  09:08:53  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:42:11  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include "macros.h"
#include "nodes.h"
#include "int.h"
#include "sym.h"

long dopragma(p) nodep p;
{
        nodep firstarg;
        nodep secondarg;

        if (strcmp(p->nd.pragmax.id, "package_name") eq 0)  {
            if (p->nd.pragmax.args)  {
              firstarg = p->nd.pragmax.args->nd.arg.exp;
              if (not firstarg)  ada_perror("package_name needs argument");
              if (firstarg->ndf.op neq nop_ident)
                ada_perror("package name needs identifier");
              package_name(firstarg->nd.ident.id);
            }
        }
        else if (strcmp(p->nd.pragmax.id, "function_name") eq 0)  {
            if (p->nd.pragmax.args)  {
              firstarg = p->nd.pragmax.args->nd.arg.exp;
              if (not firstarg)  ada_perror("function_name needs argument");
              if (firstarg->ndf.op neq nop_ident)
                ada_perror("function_name needs identifier");
              subp_name(firstarg->nd.ident.id, func_id);
            }
        }
        else if (strcmp(p->nd.pragmax.id, "procedure_name") eq 0)  {
            if (p->nd.pragmax.args)  {
              firstarg = p->nd.pragmax.args->nd.arg.exp;
              if (not firstarg)  ada_perror("procedure_name needs argument");
              if (firstarg->ndf.op neq nop_ident)
                ada_perror("procedure name needs identifier");
              subp_name(firstarg->nd.ident.id, proc_id);
            }
        }
        else error("illegal pragma: %s", p->nd.pragmax.id);
}

