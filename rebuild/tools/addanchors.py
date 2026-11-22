import sys
# usage: addanchors.py tmpl  (reads "N<TAB>text-prefix" from stdin; inserts @@N before the
# first line, after the previous anchor point, whose stripped text starts with prefix)
p = sys.argv[1]
lines = open(p).read().split('\n')
pos = 0
for spec in sys.stdin.read().strip().split('\n'):
    n, pre = spec.split('\t', 1)
    for i in range(pos, len(lines)):
        if lines[i].strip().startswith(pre.strip()) and not lines[i].startswith('@@'):
            k = i
            if True:
                while k > 0 and lines[k-1] in ('\t}', ''): k -= 1
                while k < i and lines[k].strip() == '': k += 1
            lines.insert(k, '@@' + n); i += 1
            pos = i + 1; break
    else:
        sys.exit('not found: ' + spec)
open(p, 'w').write('\n'.join(lines))
