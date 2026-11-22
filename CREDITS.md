# Arcturus credits

Arcturus is an interactive Ada programming environment and interpreter. It
was built at the Irvine Programming Environment Project, Department of
Information and Computer Science, University of California, Irvine, in
1982-84. This edition is release 1.6-A, as linked on January 5, 1984.

## Project and funding

**Professor Thomas A. Standish** created the Irvine Programming Environment
Project, found its funding and steered it at a high level. In the words of
Stephen Willson's 1984 dissertation draft, he "sought out the necessary
funding for the Irvine Programming Environment Project and provided the
initial inspiration for the present work." `config.c` says Arcturus was
produced "under the Direction of Professor Thomas A. Standish".

**DARPA.** The 1983 sources and the dissertation draft both carry this
acknowledgment:

> This work was sponsored in part by the Defense Advanced Research Projects
> Agency of the United States Department of Defense under contract
> MDA-903-82-C-0039 to the Irvine Programming Environment Project.

## Special thanks

**Professor Richard N. Taylor**, of the dissertation committee, "must also be
thanked for the many hours he spent understanding the Arcturus system and
the focus of my work" (dissertation draft, 1984). He was Stephen Willson's
advisor.

## The team

`config.c` (1983) lists the authors:

| Name | RCS login | Check-ins on tape |
|---|---|---|
| Scott C. Auchmoody | sca | 1 (Mar 1983) |
| Raymond Klefstad III | klefstad | 3 (Mar-Jun 1983) |
| Craig Snider | snider | 23 (May 1983 - Mar 1984) |
| Frank Tadman | tadman | 105 (Dec 1982 - Jan 1984) |
| Steve Whitehill | - | - |
| Stephen H. Willson | willson | 46 (Dec 1982 - Jan 1984) |

The tape also shows these contributors:

- **ritter** ("PAR" in comments) made 7 check-ins in Aug-Sep 1983, covering
  exception propagation and the `program_error` exception.
- **Gene Fisher** worked on the Castor macro facility. The comments call him
  "Gene", and the dissertation draft's acknowledgments give his full name.
- **ada**, the project's shared account, made most check-ins (266). Each of
  them merged a personal branch.

The dissertation draft also names Stephen Whitehill (the "Steve Whitehill"
above) as Stephen Willson's close collaborator on the second version of
Arcturus, and Frank Tadman as his partner in moving from the Lisp model to
one suited to type information.

## The copyright notice

The 1983 files carried a "Copyright 1983 Thomas A.
Standish" notice. In this edition, at Stephen Willson's direction (2026), that
notice was replaced in every file by a pointer to this page. The line count
was kept, so line numbers do not move. Byte-exact copies of the tape files
keep the original notices.

## 2026 recovery

Stephen Clarke-Willson (Stephen H. Willson above) did the recovery in 2026,
working with Claude (Anthropic).

- `int.c` and `toplev.c` were rebuilt from the debugging symbols and VAX
  machine code in the 1984 binary.
- `pp_functions` was decoded from the environment saved in the 1984
  `ada.core`.
- A few routines missing from the tape sources were rebuilt from the binary.
- The sources were ported to modern Linux, 64-bit and 32-bit, x86 and ARM.
