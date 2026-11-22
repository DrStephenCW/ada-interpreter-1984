#!/usr/bin/env python3
"""Minimal VAX disassembler for 4.2BSD a.out files with dbx stabs.

Usage: vaxdis.py ADA_BINARY SOURCEFILE.c   -> annotated listing of that file's functions
"""
import struct, sys, collections

# operand codes: first char access (r,w,m,a,v,b), second size (b,w,l,q,o,f,d)
T = {}
def op(code, name, *ops): T[code] = (name, ops)

for c, n in [(0,'halt'),(1,'nop'),(2,'rei'),(3,'bpt'),(4,'ret'),(5,'rsb'),(6,'ldpctx'),(7,'svpctx')]: op(c, n)
op(0x08,'cvtps','rw','ab','rw','ab'); op(0x09,'cvtsp','rw','ab','rw','ab')
op(0x0a,'index','rl','rl','rl','rl','rl','wl'); op(0x0b,'crc','ab','rl','rw','ab')
op(0x0c,'prober','rb','rw','ab'); op(0x0d,'probew','rb','rw','ab')
op(0x0e,'insque','ab','ab'); op(0x0f,'remque','ab','wl')
for c, n in zip(range(0x10,0x20), ['bsbb','brb','bneq','beql','bgtr','bleq','jsb','jmp','bgeq','blss','bgtru','blequ','bvc','bvs','bgequ','blssu']):
    op(c, n, 'ab') if n in ('jsb','jmp') else op(c, n, 'bb')
op(0x20,'addp4','rw','ab','rw','ab'); op(0x21,'addp6','rw','ab','rw','ab','rw','ab')
op(0x22,'subp4','rw','ab','rw','ab'); op(0x23,'subp6','rw','ab','rw','ab','rw','ab')
op(0x24,'cvtpt','rw','ab','ab','rw','ab'); op(0x25,'mulp','rw','ab','rw','ab','rw','ab')
op(0x26,'cvttp','rw','ab','ab','rw','ab'); op(0x27,'divp','rw','ab','rw','ab','rw','ab')
op(0x28,'movc3','rw','ab','ab'); op(0x29,'cmpc3','rw','ab','ab')
op(0x2a,'scanc','rw','ab','ab','rb'); op(0x2b,'spanc','rw','ab','ab','rb')
op(0x2c,'movc5','rw','ab','rb','rw','ab'); op(0x2d,'cmpc5','rw','ab','rb','rw','ab')
op(0x2e,'movtc','rw','ab','rb','ab','rw','ab'); op(0x2f,'movtuc','rw','ab','rb','ab','rw','ab')
op(0x30,'bsbw','bw'); op(0x31,'brw','bw'); op(0x32,'cvtwl','rw','wl'); op(0x33,'cvtwb','rw','wb')
op(0x34,'movp','rw','ab','ab'); op(0x35,'cmpp3','rw','ab','ab'); op(0x36,'cvtpl','rw','ab','wl')
op(0x37,'cmpp4','rw','ab','rw','ab'); op(0x38,'editpc','rw','ab','ab','ab'); op(0x39,'matchc','rw','ab','rw','ab')
op(0x3a,'locc','rb','rw','ab'); op(0x3b,'skpc','rb','rw','ab'); op(0x3c,'movzwl','rw','wl')
op(0x3d,'acbw','rw','rw','mw','bw'); op(0x3e,'movaw','aw','wl'); op(0x3f,'pushaw','aw')
def fam(base, s, w):
    # f/d float families
    op(base+0,'add%s2'%s,'r'+w,'m'+w); op(base+1,'add%s3'%s,'r'+w,'r'+w,'w'+w)
    op(base+2,'sub%s2'%s,'r'+w,'m'+w); op(base+3,'sub%s3'%s,'r'+w,'r'+w,'w'+w)
    op(base+4,'mul%s2'%s,'r'+w,'m'+w); op(base+5,'mul%s3'%s,'r'+w,'r'+w,'w'+w)
    op(base+6,'div%s2'%s,'r'+w,'m'+w); op(base+7,'div%s3'%s,'r'+w,'r'+w,'w'+w)
    op(base+8,'cvt%sb'%s,'r'+w,'wb'); op(base+9,'cvt%sw'%s,'r'+w,'ww'); op(base+10,'cvt%sl'%s,'r'+w,'wl')
    op(base+11,'cvtr%sl'%s,'r'+w,'wl'); op(base+12,'cvtb%s'%s,'rb','w'+w); op(base+13,'cvtw%s'%s,'rw','w'+w)
    op(base+14,'cvtl%s'%s,'rl','w'+w); op(base+15,'acb%s'%s,'r'+w,'r'+w,'m'+w,'bw')
    op(base+16,'mov%s'%s,'r'+w,'w'+w); op(base+17,'cmp%s'%s,'r'+w,'r'+w); op(base+18,'mneg%s'%s,'r'+w,'w'+w)
    op(base+19,'tst%s'%s,'r'+w); op(base+20,'emod%s'%s,'r'+w,'rb','r'+w,'wl','w'+w); op(base+21,'poly%s'%s,'r'+w,'rw','ab')
fam(0x40,'f','f'); op(0x56,'cvtfd','rf','wd'); op(0x58,'adawi','rw','mw')
op(0x5c,'insqhi','ab','aq'); op(0x5d,'insqti','ab','aq'); op(0x5e,'remqhi','aq','wl'); op(0x5f,'remqti','aq','wl')
fam(0x60,'d','d'); op(0x76,'cvtdf','rd','wf')
op(0x78,'ashl','rb','rl','wl'); op(0x79,'ashq','rb','rq','wq'); op(0x7a,'emul','rl','rl','rl','wq')
op(0x7b,'ediv','rl','rq','wl','wl'); op(0x7c,'clrq','wq'); op(0x7d,'movq','rq','wq'); op(0x7e,'movaq','aq','wl'); op(0x7f,'pushaq','aq')
def ifam(base, s, w):
    names = ['add%s2','add%s3','sub%s2','sub%s3','mul%s2','mul%s3','div%s2','div%s3','bis%s2','bis%s3','bic%s2','bic%s3','xor%s2','xor%s3']
    for i, n in enumerate(names):
        if n.endswith('2'): op(base+i, n % s, 'r'+w, 'm'+w)
        else: op(base+i, n % s, 'r'+w, 'r'+w, 'w'+w)
    op(base+14,'mneg%s'%s,'r'+w,'w'+w); op(base+15,'case%s'%s,'r'+w,'r'+w,'r'+w)
ifam(0x80,'b','b'); ifam(0xa0,'w','w'); ifam(0xc0,'l','l')
op(0x90,'movb','rb','wb'); op(0x91,'cmpb','rb','rb'); op(0x92,'mcomb','rb','wb'); op(0x93,'bitb','rb','rb')
op(0x94,'clrb','wb'); op(0x95,'tstb','rb'); op(0x96,'incb','mb'); op(0x97,'decb','mb')
op(0x98,'cvtbl','rb','wl'); op(0x99,'cvtbw','rb','ww'); op(0x9a,'movzbl','rb','wl'); op(0x9b,'movzbw','rb','ww')
op(0x9c,'rotl','rb','rl','wl'); op(0x9d,'acbb','rb','rb','mb','bw'); op(0x9e,'movab','ab','wl'); op(0x9f,'pushab','ab')
op(0xb0,'movw','rw','ww'); op(0xb1,'cmpw','rw','rw'); op(0xb2,'mcomw','rw','ww'); op(0xb3,'bitw','rw','rw')
op(0xb4,'clrw','ww'); op(0xb5,'tstw','rw'); op(0xb6,'incw','mw'); op(0xb7,'decw','mw')
op(0xb8,'bispsw','rw'); op(0xb9,'bicpsw','rw'); op(0xba,'popr','rw'); op(0xbb,'pushr','rw')
op(0xbc,'chmk','rw'); op(0xbd,'chme','rw'); op(0xbe,'chms','rw'); op(0xbf,'chmu','rw')
op(0xd0,'movl','rl','wl'); op(0xd1,'cmpl','rl','rl'); op(0xd2,'mcoml','rl','wl'); op(0xd3,'bitl','rl','rl')
op(0xd4,'clrl','wl'); op(0xd5,'tstl','rl'); op(0xd6,'incl','ml'); op(0xd7,'decl','ml')
op(0xd8,'adwc','rl','ml'); op(0xd9,'sbwc','rl','ml'); op(0xda,'mtpr','rl','rl'); op(0xdb,'mfpr','rl','wl')
op(0xdc,'movpsl','wl'); op(0xdd,'pushl','rl'); op(0xde,'moval','al','wl'); op(0xdf,'pushal','al')
for c, n in zip(range(0xe0,0xe8), ['bbs','bbc','bbss','bbcs','bbsc','bbcc','bbssi','bbcci']): op(c, n, 'rl','vb','bb')
op(0xe8,'blbs','rl','bb'); op(0xe9,'blbc','rl','bb')
op(0xea,'ffs','rl','rb','vb','wl'); op(0xeb,'ffc','rl','rb','vb','wl')
op(0xec,'cmpv','rl','rb','vb','rl'); op(0xed,'cmpzv','rl','rb','vb','rl')
op(0xee,'extv','rl','rb','vb','wl'); op(0xef,'extzv','rl','rb','vb','wl'); op(0xf0,'insv','rl','rl','rb','vb')
op(0xf1,'acbl','rl','rl','ml','bw'); op(0xf2,'aoblss','rl','ml','bb'); op(0xf3,'aobleq','rl','ml','bb')
op(0xf4,'sobgeq','ml','bb'); op(0xf5,'sobgtr','ml','bb'); op(0xf6,'cvtlb','rl','wb'); op(0xf7,'cvtlw','rl','ww')
op(0xf8,'ashp','rb','rw','ab','rb','rw','ab'); op(0xf9,'cvtlp','rl','rw','ab')
op(0xfa,'callg','ab','ab'); op(0xfb,'calls','rl','ab'); op(0xfc,'xfc')

SZ = {'b':1,'w':2,'l':4,'q':8,'o':16,'f':4,'d':8}
REG = ['r%d' % i for i in range(12)] + ['ap','fp','sp','pc']


class AOut:
    def __init__(self, path):
        d = self.d = open(path,'rb').read()
        self.magic, self.text, self.data, self.bss, self.syms, self.entry, trs, drs = struct.unpack('<8I', d[:32])
        self.toff = 1024
        self.tbase = 0
        self.dbase = self.text  # ZMAGIC: data follows text (page aligned)
        off = 1024 + self.text + self.data + trs + drs
        stroff = off + self.syms
        def s(i):
            e = d.index(b'\0', stroff + i); return d[stroff + i:e].decode('latin1')
        self.stabs = []
        for k in range(self.syms // 12):
            strx, typ, oth, desc, val = struct.unpack('<IBBhI', d[off+12*k:off+12*k+12])
            self.stabs.append((typ, s(strx) if strx else '', desc & 0xffff, val))
        self.sym = {}
        for typ, name, desc, val in self.stabs:
            if typ & 0xe0: continue
            t = typ & 0x1e
            if t in (4, 6, 8) and name:
                old = self.sym.get(val)
                if old is None or (not old.startswith('_') and name.startswith('_')) or old.endswith('.o'):
                    self.sym[val] = name

    def mem(self, addr, n):
        if addr < self.text:
            return self.d[self.toff + addr:self.toff + addr + n]
        a = addr - self.dbase
        if 0 <= a < self.data:
            return self.d[self.toff + self.text + a:self.toff + self.text + a + n]
        return None

    def cstring(self, addr, maxlen=80):
        b = self.mem(addr, maxlen)
        if not b: return None
        e = b.find(b'\0')
        if e < 0: return None
        s = b[:e]
        if all(32 <= c < 127 or c in (9, 10) for c in s) and len(s) > 0:
            return s.decode('latin1')
        return None

    def name(self, addr):
        if addr in self.sym: return self.sym[addr]
        return None


def disasm(ao, start, end, lines=None, labels=None):
    out = []
    pc = start
    while pc < end:
        here = pc
        b = ao.mem(pc, 1)[0]; pc += 1
        if b not in T:
            out.append((here, '.byte 0x%02x' % b, [])); continue
        name, ops = T[b]
        args = []; refs = []
        caseinfo = None
        for o in ops:
            acc, size = o[0], o[1]
            if acc == 'b':
                if size == 'b':
                    disp = struct.unpack('<b', ao.mem(pc,1))[0]; pc += 1
                else:
                    disp = struct.unpack('<h', ao.mem(pc,2))[0]; pc += 2
                tgt = pc + disp
                args.append('L%x' % tgt); refs.append(('br', tgt)); continue
            spec = ao.mem(pc,1)[0]; pc += 1
            idx = ''
            mode, r = spec >> 4, spec & 15
            if mode == 4:
                idx = '[%s]' % REG[r]
                spec = ao.mem(pc,1)[0]; pc += 1
                mode, r = spec >> 4, spec & 15
            if mode <= 3:
                lit = spec & 63
                if size in 'fd':
                    # short float literal
                    e = (lit >> 3) & 7; f = lit & 7
                    val = (1 + f/8.0) * 2**(e) / 2 if True else 0
                    val = (8 + f) / 16.0 * 2 ** (e + 1)
                    args.append('$%g' % val)
                else:
                    args.append('$%d' % lit)
                if caseinfo is None and name.startswith('case'): pass
                args[-1] = args[-1]
                continue
            R = REG[r]
            if mode == 5: a = R
            elif mode == 6: a = '(%s)' % R
            elif mode == 7: a = '-(%s)' % R
            elif mode == 8:
                if r == 15:
                    n = SZ.get(size, 4) if acc != 'a' else 4
                    raw = ao.mem(pc, n); pc += n
                    if size == 'f':
                        a = '$f:%s' % raw.hex()
                    elif size == 'd':
                        a = '$d:%s' % raw.hex()
                    else:
                        v = int.from_bytes(raw, 'little', signed=True)
                        a = '$%d' % v
                        u = v & 0xffffffff
                        if n == 4 and (ao.name(u) or (ao.text <= u < ao.text + ao.data)):
                            refs.append(('imm', u))
                else: a = '(%s)+' % R
            elif mode == 9:
                if r == 15:
                    v = struct.unpack('<I', ao.mem(pc,4))[0]; pc += 4
                    a = '*$0x%x' % v; refs.append(('abs', v))
                else: a = '@(%s)+' % R
            else:
                dsz = {0xa:1,0xb:1,0xc:2,0xd:2,0xe:4,0xf:4}[mode]
                fmt = {1:'<b',2:'<h',4:'<i'}[dsz]
                disp = struct.unpack(fmt, ao.mem(pc,dsz))[0]; pc += dsz
                defer = '*' if mode in (0xb,0xd,0xf) else ''
                if r == 15:
                    tgt = (pc + disp) & 0xffffffff
                    nm = ao.name(tgt)
                    a = defer + (nm if nm else '0x%x' % tgt)
                    refs.append(('pcrel', tgt))
                else:
                    a = '%s%d(%s)' % (defer, disp, R)
            args.append(a + idx)
        txt = '%-7s %s' % (name, ','.join(args))
        if name.startswith('case'):
            # limit is third operand
            try: limit = int(args[2].lstrip('$'))
            except ValueError: limit = None
            tbl = pc
            if limit is not None:
                tg = []
                for i in range(limit + 1):
                    disp = struct.unpack('<h', ao.mem(pc,2))[0]; pc += 2
                    tg.append(tbl + disp); refs.append(('br', tbl + disp))
                txt += '   ; table: ' + ' '.join('L%x' % t for t in tg)
        out.append((here, txt, refs))
    return out


def fileinfo(ao, fname):
    """Return functions in source file: list of dict(name, type, start, params, locals, lines)."""
    cur = None; funcs = []; f = None
    so = sorted([(v, n) for t, n, dsc, v in ao.stabs if t == 0x64])
    for i, (typ, name, desc, val) in enumerate(ao.stabs):
        if typ == 0x64:
            cur = name; f = None; continue
        if cur != fname: continue
        if typ == 0x24:
            f = dict(name=name.split(':')[0], stab=name, start=val, params=[], locals=[], regs=[], lines=[], blocks=[])
            funcs.append(f)
        elif f is None:
            if typ in (0x80, 0x20, 0x26, 0x28): pass
            continue
        elif typ == 0xa0: f['params'].append((name, val))
        elif typ == 0x80: f['locals'].append((name, val))
        elif typ == 0x40: f['regs'].append((name, val))
        elif typ == 0x26 or typ == 0x28: f['locals'].append(('static ' + name, val))
        elif typ == 0x44: f['lines'].append((val, desc))
        elif typ in (0xc0, 0xe0): f['blocks'].append(('{' if typ == 0xc0 else '}', val, desc))
    # end addresses
    allstarts = sorted(set(v for t, n, d, v in ao.stabs if t == 0x24) | set(v for v, n in so) | {ao.text})
    for f in funcs:
        f['end'] = min(a for a in allstarts if a > f['start'])
    return funcs


if __name__ == '__main__':
    ao = AOut(sys.argv[1])
    funcs = fileinfo(ao, sys.argv[2])
    only = sys.argv[3:]
    cur=None
    for typ, name, desc, val in ao.stabs:
        if typ == 0x64: cur = name; continue
        if cur == sys.argv[2] and typ in (0x20, 0x26, 0x28) :
            print('GLOBAL/STATIC %02x %s 0x%x' % (typ, name, val))
    for f in funcs:
        if only and f['name'] not in only: continue
        print('=' * 70)
        print('FUNCTION', f['stab'], 'start 0x%x end 0x%x' % (f['start'], f['end']))
        for p in f['params']: print('  param', p[0], 'ap+%d' % p[1])
        for p in f['regs']: print('  reg  ', p[0], REG[p[1]] if p[1] < 16 else p[1])
        for p in f['locals']: print('  local', p[0], 'fp%+d' % (p[1] - (1 << 32) if p[1] > 1 << 31 else p[1]))
        ln = collections.defaultdict(list)
        for a, l in f['lines']: ln[a].append(l)
        mask = struct.unpack('<H', ao.mem(f['start'], 2))[0]
        print('  entry mask 0x%04x' % mask)
        listing = disasm(ao, f['start'] + 2, f['end'])
        tgts = set(t for _, _, rs in listing for k, t in rs if k == 'br')
        for a, txt, refs in listing:
            if a in ln: print('  # line', ','.join(map(str, ln[a])))
            lab = ('L%x:' % a) if a in tgts else ''
            note = []
            for k, t in refs:
                if k in ('pcrel', 'abs', 'imm'):
                    nm = ao.name(t)
                    s = ao.cstring(t)
                    if nm and k != 'pcrel': note.append(nm)
                    if s is not None and not nm: note.append(repr(s))
            print('  %-8s %05x  %s%s' % (lab, a, txt, ('   ; ' + ' '.join(note)) if note else ''))
