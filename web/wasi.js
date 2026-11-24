// wasi.js -- a small WASI preview-1 layer for running arcturus.wasm in a
// browser or under node.  The file system is in memory, standard input is
// a list of lines given up front, and everything written to fd 1 and 2 is
// collected in order.  Input lines can be echoed to the output, so a run
// reads like a terminal session.
//
//   const r = runArcturus(wasmModule, files, inputLines, {echo: true});
//   r.output   -- what the program printed (and the echoed input)
//   r.status   -- exit status; "input" if it stopped to wait for a line
//                 past the end of inputLines (opts.stopForInput); or
//                 "trap" with r.error.  A null input line is end of file.
//   r.consumed -- how many input lines it read
"use strict";

(function (root) {
  const ERR = { SUCCESS: 0, BADF: 8, EXIST: 20, INVAL: 28, ISDIR: 31, NOENT: 44, NOSYS: 52, NOTDIR: 54, NOTEMPTY: 55, SPIPE: 70 };
  const FT = { CHAR: 2, DIR: 3, FILE: 4 };

  class Exit { constructor(code) { this.code = code; } }
  class NeedInput {}

  function normalize(path) {
    const out = [];
    for (const part of path.split("/")) {
      if (part === "" || part === ".") continue;
      if (part === "..") out.pop(); else out.push(part);
    }
    return "/" + out.join("/");
  }

  function runArcturus(module, files, inputLines, opts) {
    opts = opts || {};
    const enc = new TextEncoder(), dec = new TextDecoder();
    // file system: path -> {dir:true} or {data:Uint8Array, size}
    const fs = new Map([["/", { dir: true }]]);
    function mkdirs(p) {
      const parts = p.split("/").filter(Boolean);
      let cur = "";
      for (const part of parts) { cur += "/" + part; if (!fs.has(cur)) fs.set(cur, { dir: true }); }
    }
    for (const [name, text] of Object.entries(files)) {
      const p = normalize(name);
      mkdirs(p.slice(0, p.lastIndexOf("/")));
      const data = typeof text === "string" ? enc.encode(text) : text;
      fs.set(p, { data: data.slice(), size: data.length });
    }

    let output = "";
    let lineNo = 0;
    const lines = inputLines.slice();
    let pending = null;          // bytes of the line being read
    const fds = new Map();       // fd -> descriptor
    fds.set(0, { kind: "stdin" });
    fds.set(1, { kind: "out" });
    fds.set(2, { kind: "out" });
    fds.set(3, { kind: "dir", path: "/", preopen: "/" });
    let nextFd = 4;

    let mem;
    const view = () => new DataView(mem.buffer);
    const bytes = () => new Uint8Array(mem.buffer);
    const str = (ptr, len) => dec.decode(bytes().subarray(ptr, ptr + len));

    function readStdin(max) {
      if (!pending) {
        if (lineNo >= lines.length) {
          if (opts.stopForInput) throw new NeedInput();
          return new Uint8Array(0);
        }
        const next = lines[lineNo++];
        if (next === null) {             // end of file (control-D)
          if (opts.echo) output += "^D\n";
          return new Uint8Array(0);
        }
        const line = next + "\n";
        if (opts.echo) output += line;
        pending = enc.encode(line);
      }
      const n = Math.min(max, pending.length);
      const chunk = pending.subarray(0, n);
      pending = n < pending.length ? pending.subarray(n) : null;
      return chunk;
    }

    function iovs(ptr, cnt) {
      const v = view(), list = [];
      for (let i = 0; i < cnt; i++) list.push([v.getUint32(ptr + 8 * i, true), v.getUint32(ptr + 8 * i + 4, true)]);
      return list;
    }

    function resolve(dirfd, ptr, len) {
      const d = fds.get(dirfd);
      if (!d || d.kind !== "dir") return null;
      const p = str(ptr, len);
      return normalize(p.startsWith("/") ? p : d.path + "/" + p);
    }

    function filestat(ptr, node) {
      const v = view();
      v.setBigUint64(ptr, 1n, true);                       // dev
      v.setBigUint64(ptr + 8, 1n, true);                   // ino
      v.setUint8(ptr + 16, node.dir ? FT.DIR : FT.FILE);   // filetype
      v.setBigUint64(ptr + 24, 1n, true);                  // nlink
      v.setBigUint64(ptr + 32, BigInt(node.dir ? 0 : node.size), true);
      v.setBigUint64(ptr + 40, 0n, true);
      v.setBigUint64(ptr + 48, 0n, true);
      v.setBigUint64(ptr + 56, 0n, true);
    }

    function grow(node, need) {
      if (need <= node.data.length) return;
      const n = new Uint8Array(Math.max(need, node.data.length * 2, 256));
      n.set(node.data.subarray(0, node.size));
      node.data = n;
    }

    const env = ["EDITOR=none", "TERM=dumb", "HOME=/", "USER=willson"];
    const args = ["arcturus"];
    function putStrings(list, ptrs, buf) {
      const v = view(), b = bytes();
      for (const s of list) {
        v.setUint32(ptrs, buf, true); ptrs += 4;
        const e = enc.encode(s + "\0"); b.set(e, buf); buf += e.length;
      }
      return ERR.SUCCESS;
    }
    function sizes(list, cntPtr, sizePtr) {
      const v = view();
      v.setUint32(cntPtr, list.length, true);
      v.setUint32(sizePtr, list.reduce((n, s) => n + enc.encode(s).length + 1, 0), true);
      return ERR.SUCCESS;
    }

    const wasi = {
      args_get: (a, b) => putStrings(args, a, b),
      args_sizes_get: (a, b) => sizes(args, a, b),
      environ_get: (a, b) => putStrings(env, a, b),
      environ_sizes_get: (a, b) => sizes(env, a, b),
      clock_res_get: (id, ptr) => { view().setBigUint64(ptr, 1000n, true); return 0; },
      clock_time_get: (id, prec, ptr) => { view().setBigUint64(ptr, BigInt(Date.now()) * 1000000n, true); return 0; },
      random_get: (ptr, len) => { const b = bytes(); for (let i = 0; i < len; i++) b[ptr + i] = (Math.random() * 256) | 0; return 0; },
      proc_exit: (code) => { throw new Exit(code); },
      sched_yield: () => 0,
      poll_oneoff: (inp, out, n, neventsPtr) => { view().setUint32(neventsPtr, 0, true); return 0; },

      fd_write: (fd, iov, cnt, nPtr) => {
        const d = fds.get(fd);
        if (!d) return ERR.BADF;
        let total = 0;
        for (const [p, l] of iovs(iov, cnt)) {
          const chunk = bytes().subarray(p, p + l);
          if (d.kind === "out") output += dec.decode(chunk, { stream: true });
          else if (d.kind === "file") {
            const node = fs.get(d.path);
            const at = d.append ? node.size : d.pos;
            grow(node, at + l);
            node.data.set(chunk, at);
            node.size = Math.max(node.size, at + l);
            d.pos = at + l;
          } else return ERR.BADF;
          total += l;
        }
        view().setUint32(nPtr, total, true);
        return 0;
      },
      fd_read: (fd, iov, cnt, nPtr) => {
        const d = fds.get(fd);
        if (!d) return ERR.BADF;
        let total = 0;
        for (const [p, l] of iovs(iov, cnt)) {
          let chunk;
          if (d.kind === "stdin") chunk = readStdin(l);
          else if (d.kind === "file") {
            const node = fs.get(d.path);
            chunk = node.data.subarray(d.pos, Math.min(node.size, d.pos + l));
            d.pos += chunk.length;
          } else return ERR.BADF;
          bytes().set(chunk, p);
          total += chunk.length;
          if (chunk.length < l || d.kind === "stdin") break;
        }
        view().setUint32(nPtr, total, true);
        return 0;
      },
      fd_pread: () => ERR.NOSYS,
      fd_pwrite: () => ERR.NOSYS,
      fd_seek: (fd, offset, whence, outPtr) => {
        const d = fds.get(fd);
        if (!d) return ERR.BADF;
        if (d.kind !== "file") return ERR.SPIPE;
        const size = fs.get(d.path).size;
        let pos = Number(offset);
        if (whence === 1) pos += d.pos; else if (whence === 2) pos += size;
        if (pos < 0) return ERR.INVAL;
        d.pos = pos;
        view().setBigUint64(outPtr, BigInt(pos), true);
        return 0;
      },
      fd_tell: (fd, outPtr) => {
        const d = fds.get(fd);
        if (!d || d.kind !== "file") return ERR.SPIPE;
        view().setBigUint64(outPtr, BigInt(d.pos), true);
        return 0;
      },
      fd_close: (fd) => { if (!fds.has(fd)) return ERR.BADF; if (fd > 3) fds.delete(fd); return 0; },
      fd_sync: () => 0, fd_datasync: () => 0, fd_advise: () => 0, fd_allocate: () => 0,
      fd_fdstat_get: (fd, ptr) => {
        const d = fds.get(fd);
        if (!d) return ERR.BADF;
        const v = view();
        const type = d.kind === "dir" ? FT.DIR : d.kind === "file" ? FT.FILE : FT.CHAR;
        v.setUint8(ptr, type);
        v.setUint16(ptr + 2, d.append ? 1 : 0, true);
        v.setBigUint64(ptr + 8, 0xFFFFFFFFn, true);
        v.setBigUint64(ptr + 16, 0xFFFFFFFFn, true);
        return 0;
      },
      fd_fdstat_set_flags: () => 0,
      fd_fdstat_set_rights: () => 0,
      fd_filestat_get: (fd, ptr) => {
        const d = fds.get(fd);
        if (!d) return ERR.BADF;
        if (d.kind === "file" || d.kind === "dir") filestat(ptr, fs.get(d.path));
        else { new Uint8Array(mem.buffer, ptr, 64).fill(0); view().setUint8(ptr + 16, FT.CHAR); }
        return 0;
      },
      fd_filestat_set_size: (fd, size) => {
        const d = fds.get(fd);
        if (!d || d.kind !== "file") return ERR.BADF;
        const node = fs.get(d.path); grow(node, Number(size)); node.size = Number(size);
        return 0;
      },
      fd_filestat_set_times: () => 0,
      fd_prestat_get: (fd, ptr) => {
        const d = fds.get(fd);
        if (!d || !d.preopen) return ERR.BADF;
        view().setUint32(ptr, 0, true);
        view().setUint32(ptr + 4, enc.encode(d.preopen).length, true);
        return 0;
      },
      fd_prestat_dir_name: (fd, ptr, len) => {
        const d = fds.get(fd);
        if (!d || !d.preopen) return ERR.BADF;
        bytes().set(enc.encode(d.preopen).subarray(0, len), ptr);
        return 0;
      },
      fd_readdir: (fd, buf, len, cookie, usedPtr) => {
        const d = fds.get(fd);
        if (!d || d.kind !== "dir") return ERR.BADF;
        const prefix = d.path === "/" ? "/" : d.path + "/";
        const names = [...fs.keys()].filter(k => k !== d.path && k.startsWith(prefix) && !k.slice(prefix.length).includes("/"));
        let off = 0;
        const v = view(), b = bytes();
        for (let i = Number(cookie); i < names.length; i++) {
          const name = enc.encode(names[i].slice(prefix.length));
          const ent = new Uint8Array(24 + name.length);
          const ev = new DataView(ent.buffer);
          ev.setBigUint64(0, BigInt(i + 1), true);
          ev.setBigUint64(8, BigInt(i + 2), true);
          ev.setUint32(16, name.length, true);
          ev.setUint8(20, fs.get(names[i]).dir ? FT.DIR : FT.FILE);
          ent.set(name, 24);
          const n = Math.min(ent.length, len - off);
          b.set(ent.subarray(0, n), buf + off);
          off += n;
          if (off >= len) break;
        }
        v.setUint32(usedPtr, off, true);
        return 0;
      },
      fd_renumber: (from, to) => {
        if (!fds.has(from)) return ERR.BADF;
        fds.set(to, fds.get(from)); fds.delete(from);
        return 0;
      },
      path_open: (dirfd, dirflags, ptr, len, oflags, rbase, rinh, fdflags, outPtr) => {
        const p = resolve(dirfd, ptr, len);
        if (p === null) return ERR.BADF;
        let node = fs.get(p);
        const CREAT = 1, DIRECTORY = 2, EXCL = 4, TRUNC = 8;
        if (node && (oflags & EXCL) && (oflags & CREAT)) return ERR.EXIST;
        if (!node) {
          if (!(oflags & CREAT)) return ERR.NOENT;
          const parent = fs.get(p.slice(0, p.lastIndexOf("/")) || "/");
          if (!parent || !parent.dir) return ERR.NOENT;
          node = { data: new Uint8Array(0), size: 0 };
          fs.set(p, node);
        }
        if ((oflags & DIRECTORY) && !node.dir) return ERR.NOTDIR;
        if (node.dir) { fds.set(nextFd, { kind: "dir", path: p }); }
        else {
          if (oflags & TRUNC) node.size = 0;
          fds.set(nextFd, { kind: "file", path: p, pos: 0, append: !!(fdflags & 1) });
        }
        view().setUint32(outPtr, nextFd, true);
        nextFd++;
        return 0;
      },
      path_filestat_get: (dirfd, flags, ptr, len, out) => {
        const p = resolve(dirfd, ptr, len);
        if (p === null) return ERR.BADF;
        const node = fs.get(p);
        if (!node) return ERR.NOENT;
        filestat(out, node);
        return 0;
      },
      path_filestat_set_times: () => 0,
      path_create_directory: (dirfd, ptr, len) => {
        const p = resolve(dirfd, ptr, len);
        if (p === null) return ERR.BADF;
        if (fs.has(p)) return ERR.EXIST;
        fs.set(p, { dir: true });
        return 0;
      },
      path_remove_directory: (dirfd, ptr, len) => {
        const p = resolve(dirfd, ptr, len);
        const node = fs.get(p);
        if (!node) return ERR.NOENT;
        if (!node.dir) return ERR.NOTDIR;
        if ([...fs.keys()].some(k => k.startsWith(p + "/"))) return ERR.NOTEMPTY;
        fs.delete(p);
        return 0;
      },
      path_unlink_file: (dirfd, ptr, len) => {
        const p = resolve(dirfd, ptr, len);
        const node = fs.get(p);
        if (!node) return ERR.NOENT;
        if (node.dir) return ERR.ISDIR;
        fs.delete(p);
        return 0;
      },
      path_rename: (fd1, p1, l1, fd2, p2, l2) => {
        const a = resolve(fd1, p1, l1), b = resolve(fd2, p2, l2);
        if (!fs.has(a)) return ERR.NOENT;
        fs.set(b, fs.get(a)); fs.delete(a);
        return 0;
      },
      path_link: () => ERR.NOSYS,
      path_symlink: () => ERR.NOSYS,
      path_readlink: () => ERR.NOENT,
      sock_accept: () => ERR.NOSYS, sock_recv: () => ERR.NOSYS,
      sock_send: () => ERR.NOSYS, sock_shutdown: () => ERR.NOSYS,
    };

    const arcturus = {
      // a second descriptor reading the same stream (ada_break uses it)
      dup: (fd) => { const d = fds.get(fd); if (!d) return -1; fds.set(nextFd, Object.assign({}, d, { dup: true })); return nextFd++; },
    };

    const instance = new WebAssembly.Instance(module, { wasi_snapshot_preview1: wasi, arcturus });
    mem = instance.exports.memory;
    const result = { status: 0 };
    try {
      instance.exports._start();
    } catch (e) {
      if (e instanceof Exit) result.status = e.code;
      else if (e instanceof NeedInput) result.status = "input";
      else { result.status = "trap"; result.error = String(e && e.message || e); }
    }
    result.output = output + dec.decode();
    result.consumed = lineNo;
    result.files = fs;
    return result;
  }

  root.runArcturus = runArcturus;
  if (typeof module !== "undefined" && module.exports) module.exports = { runArcturus };
})(typeof self !== "undefined" ? self : globalThis);
