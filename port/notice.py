"""Replace the 1983 copyright block with a pointer to CREDITS.md,
keeping the line count.  Usage: notice.py FILE... (rewrites in place)"""
import sys

NOTE = ["Arcturus was written by the UC Irvine Arcturus team (Programming",
        "Environment Project, 1982-84).  See CREDITS.md for who they were.",
        "[2026: the original copyright notice was replaced by this pointer.]"]

def replace_notice(text):
    """Swap the 1983 copyright block for a pointer to CREDITS.md, keeping
    the same number of lines so that line numbers do not move."""
    lines = text.split('\n')
    for i, l in enumerate(lines):
        if 'Copyright 1983' in l and 'Standish' in l: break
    else: return text
    j = next(k for k in range(i, len(lines)) if '(ARPA net)' in lines[k])
    s = i - 1
    while s > 0 and lines[s].strip().strip('*#-.') == '' and not lines[s].lstrip().startswith('/*'): s -= 1
    if not (lines[s].lstrip().startswith('/*') or '****' in lines[s]): s = i - 1
    e = j + 1 if j + 1 < len(lines) and lines[j + 1].strip() in ('*/', '--.', '*', '#*', '#') else j
    n = e - s + 1
    if lines[i].lstrip().startswith('--'):
        new = ['--. ' + x for x in NOTE]
    elif lines[i].lstrip().startswith('#'):
        new = ['# ' + x for x in NOTE]
    else:
        opener = lines[s].lstrip().startswith('/*')
        closer = lines[e].rstrip().endswith('*/')
        new = (['/*'] if opener else []) + [' * ' + x for x in NOTE]
        pad = n - len(new) - (1 if closer else 0)
        new += [' *'] * max(pad, 0) + ([' */'] if closer else [])
        n2 = len(new)
        assert n2 == n, (n, n2)
        lines[s:e + 1] = new
        return '\n'.join(lines)
    new += [new[0][:3]] * (n - len(new))
    lines[s:e + 1] = new[:n] if len(new) >= n else new
    return '\n'.join(lines)

if __name__ == '__main__':
    for f in sys.argv[1:]:
        t = open(f, encoding='latin-1').read()
        open(f, 'w', encoding='latin-1').write(replace_notice(t))
