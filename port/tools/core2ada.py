#!/usr/bin/env python3
"""Print an Ada package from the internal form saved in the 1984 ada.core.
Usage: core2ada.py ada ada.core PACKAGE"""
import sys, re
sys.path.insert(0, __file__.rsplit('/', 1)[0])
from vaxdis import AOut
from coredump import Core
ao = AOut(sys.argv[1]); c = Core(sys.argv[2], ao.text)
names = [l.split(',')[0].strip() for l in open(__file__.rsplit('/', 2)[0] + '/src/nodes.h', encoding='latin-1') if l.startswith('nop_')]
OP = {n: i for i, n in enumerate(names)}
def op(p): return names[c.w(p)] if c.w(p) is not None and c.w(p) < len(names) else '?%s' % c.w(p)
def F(p, k): return c.w(p + 12 + 4 * k)
def lst(p):
    while p:
        yield p; p = c.w(p + 8)
def S(a):
    v = c.s(a)
    return '?' if v is None else v
def ex(p):
    if not p: return ''
    o = op(p)
    if o == 'nop_ident': return S(F(p, 0))
    if o == 'nop_id_decl': return S(F(p, 0))
    if o == 'nop_int': return S(F(p, 1)) if F(p, 1) else str(F(p, 0))
    if o == 'nop_string': return '"%s"' % S(F(p, 0))
    if o == 'nop_quote': return "%s'%s" % (ex(F(p, 1)), S(F(p, 2)))
    if o == 'nop_dot': return "%s.%s" % (ex(F(p, 1)), S(F(p, 2)))
    if o == 'nop_range': return "%s range %s..%s" % (ex(F(p,0)), ex(F(p,2)), ex(F(p,1))) if F(p,0) else "%s..%s" % (ex(F(p,2)), ex(F(p,1)))
    if o == 'nop_call': return '%s(%s)' % (ex(F(p, 0)), ', '.join(ex(F(a, 0)) for a in lst(F(p, 1))))
    if o == 'nop_neg': return '-' + ex(F(p, 0))
    if o in ('nop_plus','nop_minus','nop_star','nop_div'): return '%s %s %s' % (ex(F(p,0)), {'nop_plus':'+','nop_minus':'-','nop_star':'*','nop_div':'/'}[o], ex(F(p,1)))
    return '<%s>' % o
def decl(p, ind):
    o = op(p); t = '\t' * ind
    if o == 'nop_comment': return t + '--' + S(F(p, 0))
    if o in ('nop_type', 'nop_subtype'):
        tx = F(p, 2); kw = 'type' if o == 'nop_type' else 'subtype'
        if not tx: return '%s%s %s;' % (t, kw, S(F(p, 0)))
        if op(tx) == 'nop_enum':
            return '%s%s %s is (%s);' % (t, kw, S(F(p, 0)), ', '.join(ex(i) for i in lst(F(tx, 0))))
        return '%s%s %s is %s;' % (t, kw, S(F(p, 0)), ex(tx))
    if o == 'nop_obj_decl':
        ids = ', '.join(ex(i) for i in lst(F(p, 0)))
        return '%s%s : %s%s%s;' % (t, ids, 'constant ' if F(p, 3) else '', ex(F(p, 1)), (' := ' + ex(F(p, 2))) if F(p, 2) else '')
    if o == 'nop_except_decl':
        return '%s%s : exception;' % (t, ', '.join(ex(i) for i in lst(F(p, 0))))
    if o in ('nop_function', 'nop_procedure'):
        ps = []
        for q in lst(F(p, 4)):
            if op(q) == 'nop_comment': continue
            mode = ('in ' if F(q, 3) and F(q, 4) else '') + ('out ' if F(q, 4) else '')
            ps.append('%s : %s%s%s' % (', '.join(ex(i) for i in lst(F(q, 0))), mode, ex(F(q, 1)), (' := ' + ex(F(q, 2))) if F(q, 2) else ''))
        sig = '%s%s %s' % (t, 'function' if o == 'nop_function' else 'procedure', S(F(p, 0)))
        if ps: sig += '(' + (';\n' + t + '\t').join(ps) + ')'
        if o == 'nop_function': sig += ' return ' + ex(F(p, 5))
        return sig + (' is ...' if F(p, 7) else '') + ';'
    if o == 'nop_use': return '%suse %s;' % (t, ', '.join(ex(F(i, 0)) for i in lst(F(p, 0))))
    if o == 'nop_with': return '%swith %s;' % (t, ', '.join(ex(F(i, 0)) for i in lst(F(p, 0))))
    return '%s<%s>' % (t, o)
def package(p):
    out = []
    for q in lst(F(p, 1)): out.append(decl(q, 0))
    out.append('package %s is' % S(F(p, 0)))
    for q in lst(F(p, 2)): out.append(decl(q, 1))
    out.append('end %s;' % S(F(p, 0)))
    return '\n'.join(out)
want = sys.argv[3]
for sa in c.find_string(want):
    for r in c.refs(sa):
        fix = r - 4
        if c.w(fix) == 10:                 # pack_id
            spec = c.w(fix + 28); body = c.w(fix + 32)
            print(package(spec))
            if body: print('\n-- body: %s' % op(body))
            sys.exit(0)
