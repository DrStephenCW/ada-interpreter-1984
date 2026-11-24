/* wasm_pre.h -- included first in every file of the web build.
   WebAssembly checks that a call matches the function's real signature,
   so the C library functions the 1984 code calls without a declaration
   are declared here. */
#include <setjmp.h>
#include <stddef.h>
int open(const char *, int, ...);
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
