/* varargs.h -- the pre-ANSI interface, for 32-bit x86 (arguments on the stack) */
#ifndef _COMPAT_VARARGS_H
#define _COMPAT_VARARGS_H
typedef char *va_list;
#define va_alist	__va_alist
#define va_dcl		int __va_alist;
#define va_start(p)	((p) = (char *)&__va_alist)
#define va_arg(p, t)	(*(t *)(((p) += sizeof(t)) - sizeof(t)))
#define va_end(p)
#endif
