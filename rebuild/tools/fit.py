#!/usr/bin/env python3
"""Fit one function of a template onto the binary's line table.
Usage: fit.py ada FILE.c in.tmpl FUNC startline out.tmpl
The template must already put FUNC's header so that its first marker
line (the '{') is at the right place; lines from that '{' to the end of
the function carry no anchors.  Lines that end in '@' (stripped) are
forced to be code; other lines are code unless they obviously are not.
Prints the pairing of source code lines and binary group ends and
writes out.tmpl with @@ anchors placed in front of every code line."""
import sys, re, collections
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from vaxdis import AOut, fileinfo, disasm
ao = AOut(sys.argv[1]); fname = sys.argv[2]; tmpl = sys.argv[3]; func = sys.argv[4]
start = int(sys.argv[5]); outp = sys.argv[6]
f = [x for x in fileinfo(ao, fname) if x['name'] == func][0]
groups = collections.OrderedDict()
for a, l in f['lines']: groups.setdefault(a, []).append(l)
ends = sorted(g[-1] for g in groups.values())
# binary calls/strings per group end
listing = disasm(ao, f['start'] + 2, f['end'])
marks = sorted(groups)
endof = {a: groups[a][-1] for a in marks}
bc = collections.defaultdict(list)
for a, txt, refs in listing:
    m = max([x for x in marks if x <= a] or [marks[0]])
    mm = re.match(r'calls\s+\$\d+,(\S+)', txt)
    if mm and '(' not in mm.group(1): bc[endof[m]].append(mm.group(1).lstrip('_'))
    for k, t in refs:
        s = ao.cstring(t, 60)
        if s is not None and not ao.name(t) and k in ('imm','abs','pcrel'): bc[endof[m]].append(repr(s)[:30])
NOCODE = re.compile(r'^(for \(;;\) \{|\{|\}|case\s[^:]*:|default:|case [^:]*:\s*\{?|\w+:|)$')
DECL = re.compile(r'^(register\s+)?(enum\s+\w+|struct\s+\w+|int|char|nodep|boolean|fixp|symp|typep|typedval|retsp|symtabp)\b[^=()]*;$')
lines = open(tmpl).read().split('\n')
# find the template index of the function's '{': the line whose expanded number is start
# (expand anchors up to that point)
out = []; idx = None
for i, raw in enumerate(lines):
    if raw.startswith('@@'):
        n = int(raw[2:]);
        while len(out) + 1 < n: out.append('')
        continue
    out.append(raw)
    if len(out) == start: idx = i; break
head = lines[:idx]
body = lines[idx:]
def strip(s):
    return re.sub(r'/\*.*?\*/', '', s).strip()
incom = False; code = [0]
for j, raw in enumerate(body):
    if j == 0: continue
    s = raw.rstrip()
    if s.endswith('~'): body[j] = s[:-1]; continue
    forced = s.endswith('@')
    if forced: s = s[:-1]; body[j] = s
    t = s
    if incom:
        if '*/' in t: incom = False; t = t.split('*/', 1)[1]
        else: continue
    if '/*' in t and '*/' not in t.split('/*', 1)[1]: incom = True; t = t.split('/*', 1)[0]
    t = strip(t)
    if t.startswith('#'): continue
    t = re.sub(r'^(case [^:]*:|default:)\s*', '', t)
    t = re.sub(r'^\{\s*', '', t).strip()
    if forced or (t and not NOCODE.match(t) and not DECL.match(t)
                  and not (t.endswith(',') or t.endswith('(') or t.endswith('||') or t.endswith('&&') or t.endswith('=')) ):
        code.append(j)
# srccalls for display
def sc(j):
    k = j
    txt = body[j]
    while k > 0 and (strip(body[k-1]).endswith(',') or strip(body[k-1]).endswith('&&') or strip(body[k-1]).endswith('||') or strip(body[k-1]).endswith('=')):
        k -= 1; txt = body[k] + txt
    c = [x for x in re.findall(r'\b([A-Za-z_]\w*)\s*\(', re.sub(r'"[^"]*"', '', txt)) if x not in ('if','while','for','switch','return','sizeof')]
    return c + [repr(x)[:30] for x in re.findall(r'"((?:[^"\\]|\\.)*)"', txt)]
print('source code lines', len(code), 'binary group ends', len(ends))
n = min(len(code), len(ends))
for i in range(max(len(code), len(ends))):
    e = ends[i] if i < len(ends) else None
    j = code[i] if i < len(code) else None
    print('%5s  %-60s | %s' % (e, (body[j].strip()[:60] if j is not None else '-'),
          ' '.join(bc.get(e, [])) if e else ''))
if len(code) != len(ends): sys.exit('count mismatch')
# place: line code[i] at ends[i]; fillers between go right before the code line,
# except leading '}' lines which stay right after the previous code line.
res = head[:]; cur = start  # body[0] is at line 'start'
res.append(body[0]); line = start
prev = 0
for i in range(1, len(code)):
    j = code[i]; e = ends[i]
    fill = body[prev + 1:j]
    lead = []
    while fill and strip(fill[0]) == '}': lead.append(fill.pop(0))
    while fill and fill[0].strip() == '' : fill.pop(0)
    need = len(lead) + len(fill)
    room = e - line - 1
    if need > room:
        # drop blank lines from fill
        while need > room and '' in [x.strip() for x in fill]:
            k = [x.strip() for x in fill].index(''); fill.pop(k); need -= 1
    if need > room:
        print('OVERFLOW at line %d: %d filler lines, room %d: %s' % (e, need, room, body[j].strip()))
        sys.exit(1)
    res += lead
    res.append('@@%d' % (e - len(fill)))
    res += fill
    res.append(body[j]); line = e; prev = j
res += body[prev + 1:]
open(outp, 'w').write('\n'.join(res))
