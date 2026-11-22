#!/usr/bin/env python3
"""Expand '@@N' anchor lines: pad with blank lines so the next line is line N.
Usage: anchor.py in.c.tmpl out.c"""
import sys
out = []
for raw in open(sys.argv[1]).read().split('\n'):
    if raw.startswith('@@'):
        n = int(raw[2:].split()[0])
        if len(out) + 1 > n:
            print('anchor @@%d overflows: already at line %d' % (n, len(out) + 1), file=sys.stderr)
        while len(out) + 1 < n:
            out.append('')
        continue
    out.append(raw)
open(sys.argv[2], 'w').write('\n'.join(out).rstrip('\n') + '\n')
