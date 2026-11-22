# Arcturus 1.6-A: int.c and toplev.c, rebuilt

Two source files of the Arcturus Ada interpreter (UC Irvine, 1983-84) did not
survive on tape: `int.c` (rev 1.9, 83/11/30, the core evaluator) and
`toplev.c` (rev 1.8, 83/11/30, `main`, the read-eval loop, signals and core
files). Both were rebuilt in November 2026 from the executable `ada` on tape6.
That executable is a VAX 4.2BSD a.out, stamped "Arcturus January 5, 1984
willson" (sha256 5a333e68c94152ed...), and it was compiled with `-g`, so it
carries full debugging symbols.

| File | Lines | Functions | Line markers checked |
|---|---|---|---|
| int.c | 1230 | clearinterp, copy, interp, exprinterp, eval | 1054 |
| toplev.c | 312 | 13 (check_control_c ... main) | 185 |

## How it was done

1. **Disassembler.** No VAX objdump could be downloaded, so `tools/vaxdis.py`
   is a small VAX disassembler and a.out/stabs reader written for this job.
   It decodes the whole instruction set used by pcc, including `casel`
   switch tables.
2. **What the binary records.** For each function, the debugging symbols give:
   - its name and the line its name is on;
   - every parameter, local and register variable, with its name and type,
     and the block it belongs to;
   - a line-number marker for every source line in the body, with the code
     address of each.

   Struct layouts come from the surviving headers. Strings come from the data
   segment.
3. **Line-exact placement.** A group of markers that share one code address
   ends on the line where that code's statement sits. Every statement in the
   rebuilt files sits on the line the 1984 compiler recorded for it.
   `tools/fit.py` pairs the code lines in the source with the binary's 508
   groups in `eval`, and `tools/anchor.py` pads with blank lines to put each
   statement on its line.
4. **Checking.** `tools/linecheck.py` re-reads the binary and checks every
   marker group against the rebuilt source. The source must have a code line
   where every group ends. Each group must also call the same functions, with
   the same number of arguments, and use the same string constants. Operand
   order, structure offsets and branch structure were checked by hand against
   the disassembly for the cases where mistakes were likely. That check found
   and fixed four errors, for example a field that is `subtypex` and not
   `initexp`.
5. **Syntax check.** Both files pass `gcc -fsyntax-only -std=gnu89` against
   the original 1984 headers.

## Results (`checks/`)

- Neither file has a marker group that ends on a blank or comment line.
- `toplev.c` has no call or string mismatches.
- `int.c` has one mismatch, which is expected. `catenate(l, r, result)`
  passes two `typedval` structs by value, so the binary pushes 5 longwords
  for 3 arguments.
- The remaining "W" lines in the reports are a heuristic that flags the
  continuation lines of comments. They are not errors.

## What is exact and what is not

- **Exact:**
  - every executable statement, its line number and its control flow;
  - every function, parameter and local name, and their types;
  - global names;
  - the order of the globals;
  - error message text;
  - the rcsid strings.
- **Close but not certain:**
  - The exact spelling of each expression, for example `!x` vs `x == 0`, or
    `for (;;)` vs `while (true)`.
  - Whether a value was written with the project's `_` assignment macro or
    with `=`.
  - Which of several equivalent if/else layouts was used.

  The compiler produces the same code for each of these choices.
- **Written fresh:**
  - All comments and blank lines.
  - The label `again` in `eval`.
  - The names of the operator macros `arith_op`, `rel_op` and `bool_op`. The
    binary shows that each operator case is one source line, so these were
    macros, but it does not record their names.
  - The order of declarations and `#include`s at the top of each file.
  - The `$Log` history.
- **Lost:** lines 922-927 of `int.c`. They sat inside an `#ifdef` in the
  subprogram-declaration case, and the line table shows a 7-line hole there.
  The compiler never saw them.

## Side finding

Comparing the binary's `$Header` strings with the tape shows which revision
of each file was linked. But for `init.c`, `symtab.c`, `misc.c` and `dump.c`,
the binary was built from working copies that differed from the checked-in
revision with the same number. For example, `old/init.c` runs 3 lines lower
than the binary in `symwrite_core`. The binary also has a few small functions
that are not on tape (see the buildable port).

## Credits and copyright

The team is listed in [CREDITS.md](../CREDITS.md). The 1983 files carried a
"Copyright 1983 Thomas A. Standish" notice. In these files it has been
replaced, at Stephen Willson's direction, by a pointer to CREDITS.md. The
replacement has the same number of lines, so every line number above still
holds. The byte-exact tape files keep the original notice.

## Layout

```
int.c, toplev.c        rebuilt sources (line-exact)
tmpl/                  the templates the sources are generated from
tools/                 vaxdis.py, linecheck.py, fit.py, anchor.py
checks/                linecheck reports for both files
```

Regenerate a file and check it (from this folder; the tools need Python 3,
and read the original `ada` binary and headers in `../original/`):

```
python3 tools/anchor.py tmpl/int.c.tmpl int.c
python3 tools/linecheck.py ../original/ada int.c int.c fileno
```
