#!/bin/sh
# Build arcturus.wasm from ../port with clang and a wasi-libc sysroot.
#   SYSROOT  wasi-libc sysroot (built with -DSETJMP=ON)
#   BUILTINS libclang_rt.builtins-wasm32.a
# clang 18's wasm object writer crashes on common symbols, so every file
# is compiled and optimized on its own to bitcode with -fcommon, then the
# files are linked into one module (which merges the commons) and the
# merged commons become ordinary globals.  The merged module is not
# optimized again: K&R calls with the wrong number of arguments are
# undefined behaviour to LLVM, and cross-file inlining would act on that.
set -e
STRIP=${STRIP---strip-all}
cd "$(dirname "$0")"
SYSROOT=${SYSROOT:-/home/claude/sysroot}
BUILTINS=${BUILTINS:-/home/claude/rt/libclang_rt.builtins-wasm32.a}
P=../port
B=build
rm -rf $B; mkdir -p $B $B/obj
CFLAGS="--target=wasm32-wasip1 --sysroot=$SYSROOT -std=gnu89 -O2 -g0
  -fno-strict-aliasing -fcommon -fsigned-char -w
  -Wno-int-conversion -Wno-return-type -Wno-incompatible-function-pointer-types
  -D_WASI_EMULATED_SIGNAL -D_WASI_EMULATED_GETPID -mexception-handling -mllvm -wasm-enable-sjlj -Dmain=arcturus_main
  -I$P/compat -I$P/src -include src/wasm_pre.h -include $P/src/decls64.h -DARCTURUS_LIB=\"/lib/\""
for f in $P/src/*.c $P/compat/*.c src/*.c; do
	clang $CFLAGS -emit-llvm -c $f -o $B/obj/$(basename $f .c).bc
done
llvm-link $B/obj/*.bc -S -o $B/all.ll
sed -i -E 's/ = common ((hidden |local_unnamed_addr |dso_local )*)global / = \1global /' $B/all.ll
llvm-as $B/all.ll -o $B/all.opt.bc
llc -O2 -filetype=obj -mattr=+exception-handling -exception-model=wasm \
    -wasm-enable-sjlj $B/all.opt.bc -o $B/all.o
LIB=$SYSROOT/lib/wasm32-wasip1
# the setjmp run time is compiled on its own: the sjlj lowering must not see it
clang --target=wasm32-wasip1 --sysroot=$SYSROOT -O2 -mexception-handling -mllvm -wasm-enable-sjlj \
    -c rt/wasm_sjlj.c -o $B/wasm_sjlj.o
wasm-ld $LIB/crt1-command.o $B/all.o $B/wasm_sjlj.o \
    -L$LIB -lc -lwasi-emulated-getpid \
    $BUILTINS -z stack-size=8388608 --initial-memory=8519680 --max-memory=536870912 \
    $STRIP -o $B/arcturus.raw.wasm
# K&R code calls functions through pointers with whatever arguments it has
# (catch(), call(), the dispatch tables).  WebAssembly traps on a call
# whose signature differs from the function's, so binaryen's fpcast-emu
# gives every indirect call one signature of 20 arguments, the most compat/call.c passes.
WASM_OPT=${WASM_OPT:-/home/claude/binaryen-build/bin/wasm-opt}
$WASM_OPT $B/arcturus.raw.wasm --enable-exception-handling --enable-sign-ext \
    --enable-mutable-globals --enable-bulk-memory --enable-nontrapping-float-to-int \
    --fpcast-emu --pass-arg=max-func-params@20 -O2 -o arcturus.wasm
ls -l arcturus.wasm
