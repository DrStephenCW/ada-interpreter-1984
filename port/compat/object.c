/* object.c -- replaces the 1984 object.c, which loaded compiled VAX
   object files into the running interpreter.  That can't work on a
   modern machine, so loading is reported as unsupported. */
#include <stdio.h>
load_object(file) char *file;
{
	error("loading object files (%s) is not supported in this port", file);
}
