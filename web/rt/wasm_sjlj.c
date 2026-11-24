/* wasm_sjlj.c -- the setjmp/longjmp run time that clang 18's
   -wasm-enable-sjlj code calls (saveSetjmp, testSetjmp, getTempRet0).
   Current wasi-libc has the newer interface clang 19 uses, so the web
   build carries this one, after the original Emscripten version.

   Each function that calls setjmp keeps a table of (id, label) pairs.
   setjmp gives its jmp_buf a fresh id; longjmp throws a wasm exception
   carrying the jmp_buf, and the catching function looks the id up in
   its own table to find where to resume. */
#include <stdint.h>
#include <stdlib.h>

struct entry { uint32_t id, label; };
static uint32_t next_id, temp_ret0;
static struct { void *env; int val; } thrown;

void *saveSetjmp(void *env, uint32_t label, void *table, uint32_t size)
{
	struct entry *e = table;
	uint32_t i;
	for (i = 0; i < size; i++) {
		if (e[i].id == 0) {
			uint32_t id = ++next_id;
			*(uint32_t *)env = id;
			e[i].id = id;
			e[i].label = label;
			e[i + 1].id = 0;	/* the table always has size+1 entries */
			temp_ret0 = size;
			return table;
		}
	}
	size *= 2;
	table = realloc(table, sizeof(struct entry) * (size + 1));
	if (!table) __builtin_trap();
	return saveSetjmp(env, label, table, size);
}

uint32_t testSetjmp(uint32_t id, void *table, uint32_t size)
{
	struct entry *e = table;
	uint32_t i;
	for (i = 0; i < size && e[i].id; i++)
		if (e[i].id == id) return e[i].label;
	return 0;
}

uint32_t getTempRet0(void) { return temp_ret0; }

void __wasm_longjmp(void *env, int val)
{
	thrown.env = env;
	thrown.val = val ? val : 1;
	__builtin_wasm_throw(1, &thrown);	/* 1 == C_LONGJMP */
}
