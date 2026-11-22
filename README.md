# Arcturus: an Ada interpreter from 1984

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

Quick start, on Linux (x86 or ARM, including Graviton) or WSL2:

```
cd port
./arcturus -e < examples/tour.ada   # prebuilt binary in bin/; -e echoes the input
make && ./arcturus               # or build it yourself with gcc
```

The files in `original/` keep their 1983 copyright notices exactly as found.
In `rebuild/` and `port/`, each of those notices has been replaced by a
pointer to CREDITS.md.
