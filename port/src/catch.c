static char rcsid[] =
"$Header:   ../RCS/catch.v  Revision 1.2  83/01/12  09:12:09  ada  Exp$";

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


/* $Log:	../RCS/catch.v $
 * Revision 1.2  83/01/12  09:12:09  ada
 * Added copyright notice.
 * 
 * Revision 1.1  82/12/12  21:18:29  tadman
 * Initial revision (Arcturus release 1.3E)
 *  */

#include <setjmp.h>

long catch(lab, fn, arg1, arg2, arg3, arg4, arg5)
long **lab;      /* global pointer, initialized by this routine */
long (*fn)();    /* function to call */
long arg1, arg2, arg3, arg4, arg5; {
        volatile long flag;       /* tells if normal return from setjmp */	/* port: volatile, as C requires */
        long res;        /* result value for catch */
        jmp_buf env;    /* place where setjmp stores environment */

        flag = 1;       /* initialize flag to 1 */
        *lab = env;
        res = setjmp(env);
        /* we can reach this point in the code 2 ways
         *   1) we may have returned from the initializing call to
         *      setjmp(). In this case, flag = 1 and res = 0;
         *   2) we may have done an longjmp() from the function fn
         *      or from some function called by fn.
         *      In this case, flag = 0 and 
         *      res is the value given in longjmp()
         */
        if(flag == 1) {         /* if this was the init step */
                flag = 0;       /* toggle the flag */
                res = (*fn)(arg1, arg2, arg3, arg4, arg5);        
                                /* call the function */
               /* if we get to here, fn returned normally */
        }
        return(res);
}







                                                        
