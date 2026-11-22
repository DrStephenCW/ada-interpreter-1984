#!/usr/bin/env python3
"""Read the 1984 ada.core (saved Arcturus environment) and decode symbols.
core layout (toplev.c write_core_file): time_stamp, data_addr, then the
data segment from idata (first page boundary after etext) to data_addr."""
import struct, sys
class Core:
    def __init__(self, path, text_size, pagesize=1024):
        d = open(path, 'rb').read()
        self.stamp, self.end = struct.unpack('<2I', d[:8])
        self.base = text_size if text_size % pagesize == 0 else text_size + pagesize - text_size % pagesize
        self.d = d[8:]
    def ok(self, a): return self.base <= a < self.base + len(self.d) - 4
    def w(self, a): return struct.unpack('<I', self.d[a - self.base:a - self.base + 4])[0] if self.ok(a) else None
    def s(self, a, n=200):
        if not a or not self.ok(a): return None
        o = a - self.base; e = self.d.find(b'\0', o, o + n)
        return self.d[o:e].decode('latin1') if e >= 0 else None
    def find_string(self, txt):
        b = txt.encode() + b'\0'; res = []; i = 0
        while True:
            i = self.d.find(b, i)
            if i < 0: return res
            if i == 0 or self.d[i-1] == 0 or True: res.append(self.base + i)
            i += 1
    def refs(self, addr):
        b = struct.pack('<I', addr); res = []; i = 0
        while True:
            i = self.d.find(b, i)
            if i < 0: return res
            if i % 4 == 0: res.append(self.base + i)
            i += 1
