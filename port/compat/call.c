/* call.c -- portable replacement for call.s (VAX assembly).
   call(end, n, fn) calls fn with the n words that end just before "end".
   A word is a long: 32 bits on 32-bit Linux, 64 on 64-bit. */
typedef long (*ifn)();
long call(long *end, long n, ifn f)
{
	long *a = end - n;
	switch (n) {
	case 0: return f();
	case 1: return f(a[0]);
	case 2: return f(a[0],a[1]);
	case 3: return f(a[0],a[1],a[2]);
	case 4: return f(a[0],a[1],a[2],a[3]);
	case 5: return f(a[0],a[1],a[2],a[3],a[4]);
	case 6: return f(a[0],a[1],a[2],a[3],a[4],a[5]);
	case 7: return f(a[0],a[1],a[2],a[3],a[4],a[5],a[6]);
	case 8: return f(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]);
	case 9: return f(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8]);
	case 10: return f(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]);
	default: return f(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],
			a[10],a[11],a[12],a[13],a[14],a[15],a[16],a[17],a[18],a[19]);
	}
}
