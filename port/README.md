# Arcturus 1.6-A, buildable

This is the UC Irvine Arcturus Ada environment and interpreter, as linked on
January 5, 1984. It has been ported so that it builds and runs on a modern
x86 Linux machine, including WSL2 on Windows. See [CREDITS.md](../CREDITS.md)
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

The prebuilt binary `bin/ada-i386-static` is a static 32-bit x86 Linux
program. It needs no libraries, so it runs on any x86-64 Linux, including
WSL2 on Windows. WSL1 cannot run 32-bit programs.

```
./arcturus                      # interactive; end with control-D or quit;
./arcturus < examples/tour.ada  # run a script
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

You need gcc with 32-bit support. On Debian, Ubuntu or WSL, install it with
`sudo apt install gcc-multilib make`. Then:

```
make                     # builds ./ada, which ./arcturus prefers
make LDFLAGS=-static     # a static binary like the one in bin/
```

The code is built 32-bit because the 1984 sources keep pointers in `int`s
throughout. On a 32-bit build that is harmless; on a 64-bit build it would
crash.

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
  - `<varargs.h>` and `<sgtty.h>` get small shims, and `call.s` (VAX
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
- **Copyright notices.** The 1983 notice in each file is replaced by a
  pointer to CREDITS.md, with the same number of lines.

Everything else is as it was in 1984, including its limits. For example,
record aggregates are unimplemented, and tasks and generics print "You've
used an unimplemented construct".
