#!/usr/bin/env python3
"""Compare a reconstructed C file against the line table in the binary.

For every function in FILE (as recorded in the binary) print each marker
group (lines that share one code address) beside the source text of those
lines, and flag likely mismatches:
  E  a group's last line is blank or a pure comment (code was generated there)
  W  a line outside any group-end that looks like it generates code
Usage: linecheck.py ada FILE.c rebuilt.c [-v]
"""
import sys, re, collections
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from vaxdis import AOut, fileinfo

ao = AOut(sys.argv[1]); fname = sys.argv[2]
src = open(sys.argv[3]).read().split('\n')
verbose = '-v' in sys.argv
def L(n): return src[n-1] if 0 < n <= len(src) else '<EOF>'

def is_code(s):
    t = re.sub(r'/\*.*?\*/', '', s).strip()
    return t != ''

def nocode(s):
    """True if the line by itself can't generate code."""
    t = re.sub(r'/\*.*?\*/', '', s).strip()
    if t in ('', '{', '}', 'else', 'else {', '} else {', 'do', 'do {'): return True
    if t.startswith('/*') or t.startswith('*') : return True
    if re.match(r'^(register\s+)?(static\s+)?(int|char|nodep|boolean|fixp|symp|typep|symtabp|arrayp|typedval|float|double|struct\s+\w+|enum\s+\w+|unsigned|short|long|FILE|retstkp|\w+p)\b[^=()]*;\s*$', t): return True
    if t in ('while (true) {', 'for (;;) {', 'while (1) {'): return True
    return False

errs = 0
total_groups = 0
for f in fileinfo(ao, fname):
    groups = collections.OrderedDict()
    for a, l in f['lines']:
        groups.setdefault(a, []).append(l)
    allines = [l for a, l in f['lines']]
    lo, hi = min(allines), max(allines)
    ends = set(g[-1] for g in groups.values())
    print('== %s  (lines %d-%d, %d groups)' % (f['name'], lo, hi, len(groups)))
    total_groups += len(groups)
    for a, g in groups.items():
        last = g[-1]
        flag = ''
        if not is_code(L(last)): flag = 'E'; errs += 1
        if verbose or flag:
            print('  %s %05x %s' % (flag or ' ', a, ','.join(map(str, g))))
            for l in g:
                print('        %4d| %s' % (l, L(l)))
    # lines inside function, not group ends, that look like code
    for l in range(lo, hi + 1):
        if l in ends: continue
        s = L(l)
        if not nocode(s) and is_code(s):
            # could be the first line of a multi-line statement ending at l+k
            nxt = min([e for e in ends if e > l] or [10**9])
            cont = all(is_code(L(k)) for k in range(l, min(nxt, l + 8) + 1)) and not s.rstrip().endswith(';')
            if not cont:
                print('  W %4d| %s' % (l, s)); errs += 1
print('groups:', total_groups, 'flags:', errs)

# ---- second pass: calls, argument counts and string literals per group ----
from vaxdis import disasm
KEYW = {'if','while','for','switch','return','sizeof','case','_'}
MACROS = set(a for a in sys.argv[4:] if not a.startswith('-'))  # names that are macros, not calls
import glob as _glob
DEFS = {}
for _fn in [sys.argv[3]] + sorted(_glob.glob(sys.argv[1].rsplit('/', 1)[0] + '/*.h')):
    _t = open(_fn, errors='replace').read().replace('\\\n', ' ')
    for _m in re.finditer(r'^#\s*define\s+(\w+)\(([^)]*)\)(.*)$', _t, re.M):
        DEFS[_m.group(1)] = re.sub(r'/\*.*?\*/', '', _m.group(3))
def expand(text):
    for _ in range(4):
        t2 = text
        for n, b in DEFS.items():
            t2 = re.sub(r'\b%s\s*\(([^()]|\([^()]*\))*\)' % n, lambda m, b=b: ' ' + b + ' ', t2)
        if t2 == text: break
        text = t2
    return text
def srccalls(text):
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    text = expand(text)
    strs = re.findall(r'"((?:[^"\\]|\\.)*)"', text)
    t2 = re.sub(r'"((?:[^"\\]|\\.)*)"', '""', text)
    calls = []
    for m in re.finditer(r'\b([A-Za-z_]\w*)\s*\(', t2):
        c = m.group(1)
        if c in KEYW or c in MACROS: continue
        d = 0; n = 0; empty = True
        for ch in t2[m.end():]:
            if ch in '([': d += 1
            elif ch in ')]':
                if d == 0: break
                d -= 1
            elif ch == ',' and d == 0: n += 1
            if not ch.isspace(): empty = False if ch != ')' or d else empty
        calls.append('%s/%d' % (c, 0 if empty else n + 1))
    return collections.Counter(calls), collections.Counter(bytes(s, 'latin1').decode('unicode_escape') for s in strs)
bad = 0
for f in fileinfo(ao, fname):
    listing = disasm(ao, f['start'] + 2, f['end'])
    marks = sorted(set(a for a, l in f['lines']))
    lines_at = collections.defaultdict(list)
    for a, l in f['lines']: lines_at[a].append(l)
    # assign each instruction to the most recent marker address
    bc = collections.defaultdict(collections.Counter); bs = collections.defaultdict(collections.Counter)
    for a, txt, refs in listing:
        m = max([x for x in marks if x <= a] or [marks[0]])
        mm = re.match(r'calls\s+\$(\d+),(\S+)', txt)
        if mm and not txt.startswith('calls   $0,(') and '(' not in mm.group(2):
            n = mm.group(2); bc[m]['%s/%s' % (n.lstrip('_'), mm.group(1))] += 1
        for k, t in refs:
            s = ao.cstring(t, 200)
            if s is not None and not ao.name(t) and k in ('imm','abs','pcrel'): bs[m][s] += 1
        if txt.startswith('jsb') and 'mcount' in txt:
            pass
    for m in marks:
        ls = lines_at[m]
        lo = ls[0]; hi = ls[-1]
        # a statement may start before the group's first line (multi-line): include up to 3 lines back if they are continuation
        text = '\n'.join(L(l) for l in range(lo, hi + 1))
        sc, ss = srccalls(text)
        if m == marks[0]:
            for k in [k for k in sc if k.split('/')[0] == f['name']]: sc.pop(k)
        b = bc[m]; s = bs[m]
        for k in [k for k in b if k.startswith('mcount')]: del b[k]
        if sc != b or ss != s:
            bad += 1
            print('MISMATCH %s lines %d-%d\n   binary calls %s strings %s\n   source calls %s strings %s'
                  % (f['name'], lo, hi, dict(b), list(s), dict(sc), list(ss)))
print('call/string mismatches:', bad)
