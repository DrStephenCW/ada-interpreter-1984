# Arcturus credits

Arcturus is an interactive Ada programming environment and interpreter. It
was built at the Programming Environment Project, Department of Information
and Computer Science, University of California, Irvine, in 1982-84. The work
was sponsored in part by DARPA under contract MDA-903-82-C-0039. This edition
is release 1.6-A, as linked on January 5, 1984.

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
- **Gene** is named in comments about the Castor macro facility. His full
  name does not appear.
- **ada**, the project's shared account, made most check-ins (266). Each of
  them merged a personal branch.

`config.c` says Arcturus was produced "under the Direction of Professor
Thomas A. Standish". The 1983 files carried a "Copyright 1983 Thomas A.
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
- The sources were ported to 32-bit Linux.
