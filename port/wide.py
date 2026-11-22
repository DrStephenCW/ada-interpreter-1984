"""Make the 1984 sources 64-bit clean, for port.py.

The 1984 code keeps pointers in ints, and leans on C's implicit int: a
function, variable or K&R parameter with no type is an int.  On the VAX an
int and a pointer were both 32 bits.  On a 64-bit machine a pointer is 64
bits, so this module makes every int a long, which is 64 bits on 64-bit
Linux and 32 bits on 32-bit Linux:

  1. the keyword `int` becomes `long` (but `short int` stays);
  2. a declaration with only a storage class (`static x;`, `extern f();`,
     `register i, j;`) gets `long` after the storage class;
  3. a function defined with no return type (`f(a, b)` at the left margin)
     gets `long` in front;
  4. a constant passed as a whole argument (`f(x, -1)`), or defined by
     `#define NAME 0`, gets an L, so it fills a 64-bit argument;
  5. a K&R parameter that is never declared gets `long name;` just before
     the function's `{`, on the same line, so line numbers don't move.

Comments and string literals are left alone.  Every edit is on the line it
replaces, so line numbers stay the same as in the 1984 files.

decls_header() then writes a header, forced into every compile, that
declares each non-static function as long (or its own simple type), so a
call made before the function's definition doesn't cut its result to the
32 bits of an implicit int."""
import re

TOKEN = re.compile(r'''
    (?P<comment>/\*.*?\*/)
  | (?P<string>"(?:\\.|[^"\\\n])*")
  | (?P<char>'(?:\\.|[^'\\\n])*')
  | (?P<pp>^[ \t]*\#(?:[^\n]*\\\n)*[^\n]*)
  | (?P<ident>[A-Za-z_]\w*)
  | (?P<num>\d\w*(?:\.\w*)?)
  | (?P<ws>\s+)
  | (?P<op>.)
''', re.S | re.M | re.X)

STORAGE = {'static', 'extern', 'register', 'auto'}
TYPEWORDS = {'int', 'long', 'short', 'char', 'float', 'double', 'unsigned',
             'signed', 'void', 'struct', 'union', 'enum', 'const', 'volatile',
             'typedef'}
KEYWORDS = TYPEWORDS | STORAGE | {'if', 'else', 'while', 'for', 'do', 'switch',
           'case', 'default', 'return', 'goto', 'break', 'continue', 'sizeof'}


def tokens(text):
    return [(m.lastgroup, m.group(), m.start()) for m in TOKEN.finditer(text)]


def widen_num(v):
    """An integer constant becomes long (0 -> 0L).  Used for a constant
    passed to a function, so that it fills a whole 64-bit argument: -1
    passed as an int arrives as 4294967295 in a long parameter."""
    return v + 'L' if re.fullmatch(r'0[xX][0-9a-fA-F]+|\d+', v) else v


def widen_pp(line):
    """`int` and constants in a #define body (`#define boolean int`,
    `#define null 0`)."""
    m = re.match(r'(\s*#\s*define\s+\w+(?:\([^)]*\))?)(.*)$', line, re.S)
    if not m: return line
    body = re.sub(r'(?<!short )\bint\b', 'long', m.group(2))
    c = re.match(r'(\s+-?)(\d+|0[xX][0-9a-fA-F]+)(\b.*)$', body, re.S)
    if c and not m.group(1).rstrip().endswith(')'):   # #define NAME number
        body = c.group(1) + widen_num(c.group(2)) + c.group(3)
    return m.group(1) + body


def widen(text):
    toks = tokens(text)
    code = [i for i, t in enumerate(toks) if t[0] not in ('comment', 'ws')]
    ins = {}                       # offset -> text to insert
    rep = {}                       # token index -> replacement
    depth = 0                      # brace depth
    paren = 0
    calls = []                     # for each open '(': is it a call's?
    prev = None                    # previous code token value
    for n, i in enumerate(code):
        kind, val, pos = toks[i]
        nxt = toks[code[n + 1]][1] if n + 1 < len(code) else ''
        nxt2 = toks[code[n + 2]][1] if n + 2 < len(code) else ''
        if kind == 'pp':
            rep[i] = widen_pp(val)
            prev = None            # a directive ends whatever came before
            continue
        if val == '{': depth += 1
        elif val == '}': depth -= 1
        elif val == '(':
            paren += 1
            calls.append(prev is not None and re.match(r'[A-Za-z_]', prev) is not None
                         and prev not in KEYWORDS)
        elif val == ')':
            paren -= 1
            if calls: calls.pop()
        if kind == 'ident' and val == 'int' and prev != 'short':
            rep[i] = 'long'
        # a constant that is a whole argument of a call: f(0, -1)
        if kind == 'num' and calls and calls[-1] and nxt in (',', ')') and (
                prev in ('(', ',') or (prev == '-' and toks[code[n - 2]][1] in ('(', ','))):
            rep[i] = widen_num(val)
        # 2. storage class with no type
        if kind == 'ident' and val in STORAGE and prev not in STORAGE:
            j = n + 1
            while j < len(code) and toks[code[j]][1] in STORAGE: j += 1
            if j < len(code):
                k, v, p = toks[code[j]]
                after = toks[code[j + 1]][1] if j + 1 < len(code) else ''
                if v == '(' or v == '*' or (k == 'ident' and v not in KEYWORDS and
                                            after in ('(', ';', ',', '=', '[')):
                    ins[p] = ins.get(p, '') + 'long '
        # 3. function definition with no return type, at the left margin
        if (kind == 'ident' and depth == 0 and paren == 0 and val not in KEYWORDS
                and nxt == '(' and (prev is None or prev in (';', '}'))
                and re.fullmatch(r'[ \t]*', text[text.rfind('\n', 0, pos) + 1:pos])):
            ins[pos] = ins.get(pos, '') + 'long '
        prev = val
    # 4. undeclared K&R parameters
    for start, params, brace in knr_functions(text, toks, code):
        decl = text[start:brace]
        missing = [p for p in params if not declared(p, decl)]
        if missing:
            ins[brace] = ins.get(brace, '') + 'long %s; ' % ', '.join(missing)
    out = []; last = 0
    edits = sorted([(toks[i][2], toks[i][2] + len(toks[i][1]), r) for i, r in rep.items()] +
                   [(p, p, s) for p, s in ins.items()])
    for a, b, s in edits:
        out.append(text[last:a]); out.append(s); last = b
    out.append(text[last:])
    return ''.join(out)


def declared(name, decl):
    """Is K&R parameter `name` declared in the text between ')' and '{'?"""
    d = re.sub(r'/\*.*?\*/', ' ', decl, flags=re.S)
    return re.search(r'[\s,*(]%s\s*(\[|\)|,|;|=)' % re.escape(name), ' ' + d) is not None


def knr_functions(text, toks, code):
    """Yield (offset after ')', [param names], offset of '{') for each
    old-style function definition at file scope."""
    depth = 0
    n = 0
    while n < len(code):
        k, v, p = toks[code[n]]
        if v == '{': depth += 1
        elif v == '}': depth -= 1
        elif depth == 0 and k == 'ident' and n + 1 < len(code) and toks[code[n + 1]][1] == '(':
            # parameter list: identifiers separated by commas, nothing else
            j = n + 2; names = []; ok = True
            while j < len(code) and toks[code[j]][1] != ')':
                kk, vv, _ = toks[code[j]]
                if kk == 'ident' and vv not in KEYWORDS: names.append(vv)
                elif vv != ',': ok = False; break
                j += 1
            if ok and j < len(code):
                close = toks[code[j]][2] + 1
                # skip declarations up to the body's '{'; a ';' or ',' right
                # after ')' means this was a declaration, not a definition
                m = j + 1
                if m < len(code) and toks[code[m]][1] in (';', ','):
                    n = m; continue
                while m < len(code) and toks[code[m]][1] != '{' and toks[code[m]][0] != 'pp':
                    m += 1
                if m < len(code) and toks[code[m]][1] == '{' and names:
                    yield close, names, toks[code[m]][2]
                n = m; continue
        n += 1


# return types a definition can have, and how the header spells them
RETTYPES = {'long': 'long', 'char': 'char', 'float': 'float', 'double': 'double',
            'void': 'void', 'unsigned': 'unsigned', 'unsigned long': 'unsigned long',
            'boolean': 'long', 'nodep': 'struct node *', 'fixp': 'struct fixform *',
            'symp': 'struct symbolform *', 'typep': 'struct typeform *',
            'usep': 'struct useform *', 'symtabp': 'struct symtabform *',
            'retsp': 'struct retstkform *', 'FILE': 'struct _IO_FILE',
            'struct node': 'struct node', 'FILE_TYPE': 'struct adafile *'}
# left out because <signal.h> declares it too (its result is never used)
LIBC = {'raise'}


def definitions(text):
    """Yield (return type words, name) for each function defined at file
    scope, for example (['static', 'long', '*'], 'f')."""
    toks = [t for t in tokens(text) if t[0] not in ('comment', 'ws')]
    depth = 0; start = 0; n = 0
    while n < len(toks):
        k, v, p = toks[n]
        if k == 'pp' or v in (';', '}'):
            if v == '{': depth += 1
            elif v == '}': depth -= 1
            start = n + 1
        elif v == '{': depth += 1
        elif v == '(' and depth == 0 and n > 0 and toks[n - 1][0] == 'ident':
            m = n; level = 0
            while m < len(toks):
                if toks[m][1] == '(': level += 1
                elif toks[m][1] == ')':
                    level -= 1
                    if level == 0: break
                m += 1
            if m + 1 < len(toks) and toks[m + 1][1] not in (';', ',', '=', ')', '('):
                yield [t[1] for t in toks[start:n - 1]], toks[n - 1][1]
            n = m
        n += 1


def decls_header(texts):
    """`long f();` (or `char *f();`...) for every non-static function
    defined at file scope."""
    seen = {}
    for name, t in sorted(texts.items()):
        for words, fn in definitions(t):
            if 'static' in words or fn == 'main' or fn in KEYWORDS or fn in LIBC: continue
            stars = ''.join(w for w in words if w == '*')
            typ = ' '.join(w for w in words if w != '*' and w not in STORAGE)
            if typ not in RETTYPES: continue
            typ = RETTYPES[typ]
            sep = '' if typ.endswith('*') else ' '
            seen.setdefault(fn, '%s%s%s%s();' % (typ, sep, stars, fn))
    return ('/* port: generated by wide.py.  Every non-static 1984 function,\n'
            '   declared, so that a call made before its definition keeps all\n'
            '   64 bits of the result. */\n' +
            '\n'.join(seen[f] for f in sorted(seen)) + '\n'
            '/* and from the C library, used undeclared in misc.c */\n'
            'char *sbrk();\n')
