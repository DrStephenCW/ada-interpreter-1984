/*  $Header:   /usr/ics/darpa/ada/RCS/call.v  Revision 1.3  83/01/12  10:37:11  ada  Exp$ */

/* $Log:	/usr/ics/darpa/ada/RCS/call.v $
 # Revision 1.3  83/01/12  10:37:11  ada
 # Use correct commenting convention.
 # 
 * Revision 1.2  83/01/12  09:39:40  ada
 * Added coyright notice.
 * 
 */
/*******************************************************************
*
* Copyright 1983      Thomas A. Standish
*		      Programming Environment Project
*                     Department of Information and Computer Science
*                     University of California
*                     Irvine, California  92717
*
* All rights reserved. No part of this software may be sold or distributed
* in any form or by any means without the prior written permission of the 
* author.
*
* Report problems and direct all inquiries to ada%uci@rand-relay (ARPA net).
*
*/

.data
.text
LL0:.align	1
.globl	_call
.set	L12,0x800
.data
.text
_call:.word	L12
clrl	r11
jbr	L18
L2000001:
subl2	$4,4(ap)
movl	4(ap),r0
pushl	(r0)
incl	r11
L18:cmpl	r11,8(ap)
jlss	L2000001
calls	8(ap),*12(ap)
ret

