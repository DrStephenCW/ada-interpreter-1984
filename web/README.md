# Arcturus in the browser

`arcturus.html` is one self-contained page that runs the 1984 interpreter
in a browser. The ported C in `../port` is compiled to WebAssembly, and
nothing runs on a server.

| File | What it is |
|---|---|
| `build.sh` | Builds `arcturus.wasm` from `../port` |
| `src/` | What the web build adds: stubs for what WASI lacks (`wasm_compat.c`) and declarations it needs (`wasm_pre.h`) |
| `rt/wasm_sjlj.c` | The setjmp/longjmp run time that clang 18's `-wasm-enable-sjlj` code calls |
| `wasi.js` | A small WASI layer with an in-memory file system and line-at-a-time stdin |
| `page.html` | The page template |
| `make_page.py` | Inlines the wasm, `../port/lib`, `wasi.js` and the examples into `arcturus.html` |
| `examples/` | Extra examples for the page. The first line is the button name |

Building it needs clang and wasm-ld (18), a wasi-libc sysroot built with
`-DSETJMP=ON`, compiler-rt builtins for wasm32, and binaryen's `wasm-opt`:

```
SYSROOT=... BUILTINS=... WASM_OPT=... ./build.sh
```

```
python3 make_page.py
```

How it works, and what it took:

- **One signature for indirect calls.** The K&R code calls functions through
  pointers with whatever arguments it has (`catch()`, `call()`, the dispatch
  tables). WebAssembly traps when a call's signature differs from the
  function's. Binaryen's `--fpcast-emu` gives every indirect call one
  signature of 20 arguments.
- **setjmp/longjmp** become WebAssembly exceptions.
- **Two fixes in `port.py`** that every build now gets:
  - `catch()`'s `flag` is `volatile`, as C requires of a local changed after
    setjmp. Without it, optimized code ran the caught function twice.
  - `doindex` takes a `long`, but one caller passed it a `word` union.
    x86 and ARM pass a one-word union like a long. WebAssembly passes it by
    address, so every array element read the wrong place.
- **No cross-file optimization.** Each file is optimized on its own, then
  the files are linked. K&R calls with the wrong number of arguments are
  undefined behaviour to LLVM, and cross-file inlining would act on that.
- **Replay instead of blocking input.** The page keeps the lines you have
  typed. Each new line reruns the whole session in a fresh instance, in a
  worker, until the interpreter asks for input again. That takes a few
  milliseconds. A line that crashes or runs for more than 8 seconds is
  dropped.

Limits: the browser's own stack allows about 500 nested Ada calls, compared
with thousands natively. `edit`, `save`/`restore` and loading VAX object
files can't work here.
