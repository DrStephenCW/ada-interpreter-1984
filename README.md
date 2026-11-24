# Arcturus: an Ada interpreter from 1984

_This project, aside from [`original/`](original/), was done with Claude Opus 5.5 Medium Effort guided by Stephen Clarke-Willson_

Arcturus was an interactive Ada programming environment, written in C at the
University of California, Irvine, in 1982-84. It was built around an
interpreter that runs Ada as you type it. The team is listed in
[CREDITS.md](CREDITS.md).

This repository holds release 1.6-A, as linked on January 5, 1984, recovered
from a 1980s backup tape in 2026.

| Folder | What it is |
|---|---|
| [`original/`](original/) | The files from the tape, byte-exact, including the VAX 4.2BSD binary `ada` and its saved environment `ada.core` |
| [`rebuild/`](rebuild/) | `int.c` (the evaluator) and `toplev.c` (the main loop) are missing from the tape. Both were rebuilt line-exact from the binary's machine code and debugging symbols, and checked against it |
| [`port/`](port/) | A version you can build and run today on 64-bit or 32-bit Linux, x86 or ARM (see its README) |
| [`web/`](web/) | The port compiled to WebAssembly: [`web/arcturus.html`](web/arcturus.html) is one self-contained page that runs Arcturus in a browser (see its README) |

Quick start, on Linux (x86 or ARM, including Graviton) or WSL2:

```
cd port
./arcturus -e < examples/tour.ada   # prebuilt binary in bin/; -e echoes the input
make && ./arcturus               # or build it yourself with gcc
```

The files in `original/` keep their 1983 copyright notices exactly as found.
In `rebuild/` and `port/`, each of those notices has been replaced by a
pointer to CREDITS.md.

## The whole story

This is the third of three repositories that, read together, tell the
story from a weekend parser in 1981 to the interpreter here:

1. [DrStephenCW/ada-parser-1981](https://github.com/DrStephenCW/ada-parser-1981):
   the Ada parser Stephen wrote in MLISP on the UC Irvine PDP-10 in January
   1981, and what came of it.
2. [Above-the-Garage/ada-parser-1981-go](https://github.com/Above-the-Garage/ada-parser-1981-go):
   that parser ported to Go in 2026, with
   [From a Weekend Parser to ADARPE](https://github.com/Above-the-Garage/ada-parser-1981-go/blob/main/docs/adarpe-story-1981.md),
   the 1981 Lisp environment that came between the parser and Arcturus.
3. This repository: Arcturus, the C interpreter of 1982-84, recovered and
   running again.
