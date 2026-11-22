# Arcturus 1.6-A, buildable

This is the UC Irvine Arcturus Ada environment and interpreter, as linked on
January 5, 1984. It has been ported so that it builds and runs on modern
Linux, 64-bit or 32-bit, on x86 or ARM. That includes WSL2 on Windows and
Amazon Linux 2023 on Graviton. See [CREDITS.md](../CREDITS.md)
for who wrote it.

```
$ ./arcturus
Arcturus  willson 1.6-A  January 5, 1984

> function fact(n : integer) return integer is
+ begin
+    if n <= 1 then return 1; else return n * fact(n - 1); end if;
+ end fact;
> fact(10)
3628800
> pp(fact'sym);
function fact(n: in integer) return integer is
...
```

## Running it

`bin/` has prebuilt static binaries for 64-bit ARM (`ada-aarch64-static`,
for Graviton and other ARM64 Linux), 64-bit x86 (`ada-x86_64-static`) and
32-bit x86 (`ada-i386-static`). They need no libraries. The `arcturus`
script picks the one for your machine, or `./ada` if you built one.

```
./arcturus                         # interactive; end with control-D or quit;
./arcturus < examples/tour.ada     # run a script
./arcturus -e < examples/tour.ada  # run a script, echoing each line it reads
```

Each line you type is read on its own:

- A line that ends in `;` is a declaration or statement.
- A line with no `;` is an expression, and its value is printed.
- At top level, assigning to an undeclared name declares it, using the type
  of the value assigned.
- Some useful commands:
  - `pp(name'sym);` prettyprints a declaration.
  - `dir;` lists what you have declared.
  - `where;` shows the current scope.
  - `history("file");` writes out everything you have typed.
  - `printall;` and `symbol_table;` show the symbol tables.
  - `quit;` leaves.

Arcturus keeps your session in the current directory. Declarations go in
`ADA.SAVE` and subprograms in `user/`, so a later session in the same
directory starts where you left off. Delete those files to start fresh.

## Building it

You need gcc and make:

- Amazon Linux 2023, Fedora: `sudo dnf install gcc make`
- Debian, Ubuntu, WSL: `sudo apt install gcc make`

Then:

```
make                     # builds ./ada, which ./arcturus prefers
make LDFLAGS=-static     # a static binary like the ones in bin/
make CC="gcc -m32"       # a 32-bit x86 build (needs gcc-multilib)
```

The 1984 sources keep pointers in `int`s throughout, which only works when
an int and a pointer are the same size. `port.py` makes every `int` a
`long` (see "64-bit" below), so the same sources build 64-bit and 32-bit.

## What is here

| Path | Contents |
|---|---|
| `src/` | The 1984 C sources with the port's edits applied, ready to compile |
| `compat/` | Small replacements for what 4.2BSD and the VAX provided |
| `lib/` | The run-time Ada files the interpreter reads when it starts |
| `recovered/` | The two run-time files that had to be recovered (see below) |
| `examples/` | `hello.ada` and `tour.ada` |
| `bin/` | The prebuilt static binary |
| `port.py` | Regenerates `src/` and `lib/` from the tape files |
| `wide.py` | The 64-bit changes, used by `port.py` |
| `notice.py` | The copyright-notice replacement, also used by `port.py` |
| `tools/` | The VAX a.out reader, and the decoder for the 1984 saved core image |

`port.py` records every change made to the 1984 sources. Each change is also
marked `port:` in the code. To regenerate from the tape:

```
./rebuild.sh        # python3 port.py ../original ../rebuild src, then make
```

## Where the sources came from

- **Matching the binary.** For each file, the RCS revision named in the
  binary's `$Header` strings was used. That is the top-level copy on tape
  for most files, or `old/` for `init.c`, `intaux.c`, `symtab.c` and
  `misc.c`, and `old.ada.code/` for `dump.c`.
- **Rebuilt from the binary.** `int.c` and `toplev.c` were missing from the
  tape. They were rebuilt, line-exact, from the binary's machine code and
  debugging symbols.
- **Small functions rebuilt.** The binary was linked from working copies
  that were a little different from the checked-in revisions. The binary
  calls a few small functions that the tape lacks. They were rebuilt from
  the machine code into `compat/missing.c`: `docref`, `enter_list`,
  `dump_history`, `reorder_and_dump`, `delete_sym` and `touch`.
- **pp_functions recovered.** The run-time file `pp_functions` (rev 1.4,
  1983) was lost. It holds the prettyprinter's options and the declarations
  of `pp`, `ifp` and `dir`. It was decoded from the internal form saved in
  the 1984 `ada.core` by `tools/core2ada.py`. Its names, types, ranges and
  default values are the originals.
- **standard taken from the core image.** The tape's `standard` (March
  1984) spells out type `character`, which this January 1984 interpreter
  can't read. The package saved in `ada.core` is used instead.
- **Newer revisions on tape.** Eight files on tape are newer revisions than
  the ones linked: `casfns`, `casint`, `cassym`, `command`, `count`, `dir`,
  `tyint` and the run-time `standard.str`. They are used as they are.

## What the port changed

- **Compiler and library differences.**
  - `extern` declarations between a K&R parameter list and the body are
    moved, and block-scope `extern`s are repeated at file scope.
  - `static` is dropped from a function that was used before it was
    defined.
  - `perror` is renamed `ada_perror`.
  - `<sgtty.h>` gets a small shim, `<varargs.h>` becomes `<stdarg.h>`, and `call.s` (VAX
    assembly) becomes `compat/call.c`.
  - Old lex's `yyin = stdin` initializer is set at startup, and `yywrap`
    and `yyreject` are provided.
- **VAX habits.**
  - The VAX let a program read address 0 and got 0. One startup read relied
    on this (`countingp`).
  - A VAX 32-bit float is the top half of a 64-bit double, so code that
    passed one where the other was expected printed nearly the right value.
    Three places that print reals now reinterpret the bits.
- **Things that can't work here.**
  - Loading compiled VAX object files reports "not supported".
  - Saving and restoring a memory image (`save;`, `restore;` and
    `ada.core`) reports "not supported". Your session still persists
    through `ADA.SAVE` and `user/` as described above.
- **Paths and conveniences.**
  - The library directory and default editor now point at this folder and
    `vi`. The environment variable `ARCTURUS_LIB` overrides the library
    directory.
  - At end of piped input inside an error break, the program exits instead
    of looping.
- **64-bit.** `wide.py` makes these changes, all mechanical, each on the
  line it changes, so line numbers still match the 1984 files:
  - Every `int` becomes `long`. A `long` is a pointer's size on Linux,
    64-bit or 32-bit.
  - Every implicit `int` gets an explicit `long`: a function with no return
    type, `static x;`, `extern f();`, `register i;`, and a K&R parameter
    that is never declared (it gets `long name;` before the `{`).
  - A constant passed as a whole argument, like the `-1` in `f(x, -1)`,
    gets an `L`. So does `#define null 0`. Otherwise `-1` would arrive in a
    64-bit parameter as 4294967295.
  - `src/decls64.h` is generated and included in every file. It declares
    each function with its real return type, so a call made before the
    definition doesn't cut a pointer down to 32 bits.

  A few hand edits go with them, all in `port.py`. `cm` in `parser.c`
  uses `<stdarg.h>`, since arguments now come in registers. A real takes
  a word, as an integer does, because array indexing relies on it. `dump.c`
  had a variable that clashed with a function name. The memory report
  counts heap growth from startup. ARM chars are unsigned, so the build
  uses `-fsigned-char`, as on the VAX.

  Ada's `integer` is still 32 bits (`integer'last` is 2147483647), as in
  1984. Integer division by zero stops the program on x86, as it did in
  1984, but gives 0 on ARM, which doesn't trap it.
- **Echo.** `-e` (or `--echo`, or `ARCTURUS_ECHO=1`) echoes each input
  line after its prompt, so a script's output reads like a session. It is
  in `compat/port.c`; the 1984 program ignores its arguments.
- **Copyright notices.** The 1983 notice in each file is replaced by a
  pointer to CREDITS.md, with the same number of lines.

Everything else is as it was in 1984, including its limits. For example,
record aggregates are unimplemented, and tasks and generics print "You've
used an unimplemented construct".
