#!/usr/bin/env python3
"""Make a buildable copy of Arcturus 1.6-A for modern Linux, 64-bit or 32-bit.

Usage: port.py TAPE_ADA_DIR REBUILT_DIR OUT_DIR

TAPE_ADA_DIR is the ada folder from tape6, REBUILT_DIR holds the rebuilt
int.c and toplev.c.  Every change made to the 1984 sources is listed in
this file; nothing else is touched.  The originals are only read."""
import os, re, sys, shutil
tape, rebuilt, out = sys.argv[1:4]
os.makedirs(out, exist_ok=True)

# where each object in the 1984 Makefile comes from
OBJECTS = '''attach case casfns casint cassym catch config conv count crefaux
 creftab dir dump error except goto ifp init int intaux intform loop maptree misc
 ntabs oblist paraux parerr parser pp ppattach ppaux ppcolor ppcref pragint
 rsw scanner scope stack symtab tokens toplev tyint typ typeonly version command'''.split()
SRC = {o: os.path.join(tape, o + '.c') for o in OBJECTS}
for o in ('init', 'intaux', 'symtab', 'misc'):        # revisions linked in the binary
    SRC[o] = os.path.join(tape, 'old', o + '.c')
SRC['dump'] = os.path.join(tape, 'old.ada.code', 'dump.c')
SRC['int'] = os.path.join(rebuilt, 'int.c')
SRC['toplev'] = os.path.join(rebuilt, 'toplev.c')

# function definitions that pcc let be "static" after a non-static use
UNSTATIC = {
}

# plain text substitutions: file -> [(old, new)]
SUBST = {
    # On the VAX, reading address 0 gave 0 (the text segment started there),
    # and the 1984 code relied on it.  Each such read is fixed here.
    'init.c':    [('    intdebugp = &dummy;', '    intdebugp = &dummy; countingp = &dummy;	/* port: was a read of address 0 */'),
                  # the working copy of init.c linked in January 1984 differs from
                  # rev 1.13 here; the binary hooks up these names (see uci_ada_fns)
                  ('funcinit ("del", symdel);', '{ extern delete_sym(); funcinit ("rm", delete_sym); }	/* port: as in the 1984 binary */'),
                  ('funcinit ("psyms", sympsyms);', '{ extern printall(); funcinit ("printall", printall); }	/* port: as in the 1984 binary */'),
                  ('    funcinit ("putsym", putsym);\n', '    funcinit ("putsym", putsym);\n    { extern touch(); funcinit ("touch", touch); }	/* port: as in the 1984 binary */\n'),
                  ('static\tcheck_top_level() {', 'check_top_level() {'),
                  # a real is kept in a word, like an integer; on the VAX both
                  # were 4 bytes, and array indexing (intaux.c) relies on it
                  ('realtype -> tyf.size = sizeof (float);', 'realtype -> tyf.size = sizeof (int);	/* port: was sizeof (float) */'),
                  # these return a FILE_TYPE (a pointer), not an int
                  ('extern standard_input(), standard_output(), standard_msg();',
                   'extern FILE_TYPE standard_input(), standard_output(), standard_msg();	/* port */')],
    # memory used is the growth of the heap; on a modern system the heap
    # doesn't start at "end", so count from where it was at startup
    'misc.c':    [('((sbrk(0) - (int)end) / 1024));', '(((int)sbrk(0) - port_brk0) / 1024));	/* port: was sbrk(0) - (int)end */'),
                  ('sbrk(0) - (int)end);', '(int)sbrk(0) - port_brk0);	/* port: was sbrk(0) - (int)end */'),
                  ('char *malloc();', 'char *malloc();\nextern int port_brk0;	/* port: in compat/port.c */')],
    # VAX F_float is the top half of a D_float, so 1984 code that handed a
    # 32-bit float to a double slot printed nearly the right value.  IEEE
    # floats don't work that way; these places reinterpret the bits.
    'dump.c.2':  [('sprintf (res, "%g", val);/* can print integer */',
                   'sprintf (res, "%g", *(float *)&val);/* can print integer */	/* port */')],
    'init.c.2':  [('static fputreal(f, x, width, prec)\nFILE *f;\nfloat x;',
                   'static fputreal(f, xbits, width, prec)\nFILE *f;\nint xbits;	/* port: float bits, from call() */'),
                  ('{\n        char buf[128];\n        if(width == 0 && prec == 0) ',
                   '{\n        char buf[128]; float x = *(float *)&xbits;	/* port */\n        if(width == 0 && prec == 0) '),
                  ('static putreal(x, width, prec)        /* put a real on the terminal */\nfloat x;',
                   'static putreal(x, width, prec)        /* put a real on the terminal */\nint x;	/* port: float bits, passed on as they are */')],
    'config.c':  [('/*	Copyright (c) 1982, 1983 Thomas A. Standish			*/',
                   '/*	See CREDITS.md.  [2026: copyright line replaced]		*/'),
                  ('char	*Library = "/ub/darpa/ada/cur/";',
                   '#ifndef ARCTURUS_LIB\n#define ARCTURUS_LIB "lib/"\n#endif\nchar	*Library = ARCTURUS_LIB;	/* port: was "/ub/darpa/ada/cur/" */'),
                  ('char	*Default_Editor = "/ub/darpa/bin/nf";',
                   'char	*Default_Editor = "vi";	/* port: was "/ub/darpa/bin/nf" */')],
    'toplev.c':  [('\tfd _ open(s, 0);\n',
                   '\tfd _ open(s, 0);\n\tif (fd != -1) { close(fd); sysputf("[port: %s is a memory image; it can\'t be restored on this machine]\\n", s); }\n\treturn false;	/* port */\n'),
                  ('\tdata_addr _ (int)sbrk(0);\n',
                   '\tsysputf("[port: saving a memory image is not supported on this machine]\\n");\n\treturn false;	/* port */\n\tdata_addr _ (int)sbrk(0);\n'),
('\t\t\t\tsysputf("Can\'t continue from this type of error");',
                   '\t\t\t\tsysputf("Can\'t continue from this type of error");\n\t\t\tif (feof(stdin)) exit(1);	/* port: stop at end of piped input */')],
    # the old <varargs.h> walked the stack; on 64-bit machines arguments
    # come in registers, so cm takes them through <stdarg.h>.
    'parser.c':  [('# include <varargs.h>', '# include <stdarg.h>	/* port: was <varargs.h> */'),
                  ('static cm(va_alist)\n    va_dcl\n{\n    va_list pvar;\n    nodep p, m;\n\n    va_start(pvar);\n    m = (nodep)va_arg(pvar,nodep);',
                   'static cm(nodep m, ...)	/* port: was cm(va_alist) va_dcl */\n{\n    va_list pvar;\n    nodep p;\n\n    va_start(pvar, m);\n    /* port: m is the first argument, the syntax node */')],
    # give the struct behind FILE_TYPE a tag, so decls64.h can name it
    'adaio.h':   [('typedef struct {\n\tint ibufp;', 'typedef struct adafile {	/* port: tag added */\n\tint ibufp;')],
    'command.c': [('return (int)fgets(s,f);', 'return (int)fgets(s,1024,f);')],
    'dump.c':    [(re.compile(r'\bconst\b'), 'cnst'),
                  (re.compile(r'\bmaptab\b'), 'dump_maptab'),
                  # its static variable newline would clash with the declared
                  # function newline() (see wide.py)
                  ('static boolean newline;', 'static boolean dump_newline;	/* port: was newline */'),
                  ('    newline = false;', '    dump_newline = false;')],   # port: misc.c has the real maptab
    'scanner.c': [('FILE *yyin ={stdin}, *yyout ={stdout};', 'FILE *yyin, *yyout;')],
}
RENAME_PERROR = ('paraux.c', 'parser.c', 'pragint.c', 'scanner.c')


sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from notice import replace_notice
import wide

def move_knr_externs(text):
    """pcc accepted 'extern' declarations between a K&R parameter list
    and the function body; move them to just inside the '{'."""
    lines = text.split('\n'); res = []; i = 0
    while i < len(lines):
        if re.match(r'^\s*extern\b.*;\s*$', lines[i]):
            j = i
            while j < len(lines) and re.match(r'^\s*extern\b.*;\s*$', lines[j]): j += 1
            if j < len(lines) and lines[j].strip().startswith('{') and i > 0 and \
               (re.search(r'\)\s*$', lines[i-1]) or re.match(r'^\s+\S.*;\s*$', lines[i-1])):
                res.append(lines[j]); res.extend(lines[i:j]); i = j + 1; continue
        res.append(lines[i]); i += 1
    return '\n'.join(res)

def hoist_block_externs(text):
    """pcc kept a block-scope 'extern' declaration in force for the rest of
    the file; repeat each one at file scope after the last #include."""
    decls = []
    for l in text.split('\n'):
        m = re.match(r'^\s+(extern\b[^=]*;)\s*(/\*.*\*/)?\s*$', l)
        if m and m.group(1) not in decls: decls.append(m.group(1))
    if not decls: return text
    lines = text.split('\n')
    last = max([i for i, l in enumerate(lines) if l.startswith('#include') or l.startswith('# include')] or [0])
    lines[last+1:last+1] = ['/* port: block-scope externs, repeated at file scope */'] + decls
    return '\n'.join(lines)

def unstatic_used_before(text):
    """pcc allowed 'static f() {...}' after f had been used (and so
    implicitly declared extern); gcc does not.  Drop 'static' there."""
    for m in list(re.finditer(r'^static\s+((?:(?:unsigned|struct|enum)\s+)?\w+(?:\s+|\s*\*+\s*))?(\w+)\s*\([^()]*\)(?!\s*[;,])', text, re.M)):
        name = m.group(2)
        if re.search(r'\b%s\b' % name, text[:m.start()]):
            text = text[:m.start()] + ' ' * 7 + text[m.start() + 7:]
    return text

ported = {}
for o, path in sorted(SRC.items()):
    t = open(path, encoding='latin-1').read()
    name = o + '.c'
    t = replace_notice(t)
    t = move_knr_externs(t)
    t = hoist_block_externs(t)
    t = unstatic_used_before(t)
    for fn in UNSTATIC.get(name, ()):
        t, n = re.subn(r'^static\s+((?:\w+\s*\**\s*)*?\b%s\s*\()' % fn, r'\1', t, flags=re.M)
        assert n, (name, fn)
    for a, b in list(SUBST.get(name, [])) + list(SUBST.get(name + '.2', [])):
        if hasattr(a, 'sub'): t = a.sub(b, t)
        else:
            assert a in t, (name, a); t = t.replace(a, b)
    if name in RENAME_PERROR:
        t = re.sub(r'\bperror\s*\(', 'ada_perror(', t)
    ported[name] = wide.widen(t)
for h in os.listdir(tape):
    if h.endswith('.h'):
        t = replace_notice(open(os.path.join(tape, h), encoding='latin-1').read())
        for a, b in SUBST.get(h, []):
            assert a in t, (h, a); t = t.replace(a, b)
        ported[h] = wide.widen(t)
for name, t in ported.items():
    open(os.path.join(out, name), 'w', encoding='latin-1').write(t)
compat = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'compat')
texts = {n: t for n, t in ported.items() if n.endswith('.c')}
for n in os.listdir(compat):
    if n.endswith('.c'): texts['compat/' + n] = open(os.path.join(compat, n)).read()
open(os.path.join(out, 'decls64.h'), 'w').write(wide.decls_header(texts))

# run-time files, read by the interpreter when it starts
lib = os.path.join(out, '..', 'lib'); os.makedirs(lib, exist_ok=True)
here = os.path.dirname(os.path.abspath(__file__))
for f in ('standard', 'standard.str', 'text_io', 'uci_ada_fns', 'user_package', 'internal_fns'):
    t = replace_notice(open(os.path.join(tape, f), encoding='latin-1').read())
    if f == 'standard':
        # the 84/03/16 revision spells out type character as an enumeration,
        # which the January 1984 interpreter can't read: it sets character
        # up itself in init.c.  Use the package saved in the 1984 ada.core.
        i = t.index('package standard is')
        t = t[:i] + open(os.path.join(here, 'recovered', 'standard.pkg')).read()
    open(os.path.join(lib, f), 'w', encoding='latin-1').write(t)
shutil.copy(os.path.join(here, 'recovered', 'pp_functions'), lib)
