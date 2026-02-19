#!/usr/bin/env sh
set -eu

if [ ! -f "build/libxxh3_wrapper.dylib" ] && [ ! -f "build/libxxh3_wrapper.so" ]; then
  echo "build artifact missing; run meson compile -C build"
  exit 1
fi

LIB="build/libxxh3_wrapper.so"
if [ -f "build/libxxh3_wrapper.dylib" ]; then
  LIB="build/libxxh3_wrapper.dylib"
fi

if command -v nm >/dev/null 2>&1; then
  nm -g "$LIB" > build/abi-symbols.txt
else
  echo "nm not found"
  exit 1
fi

grep -q "xxh3_64_scalar" build/abi-symbols.txt
grep -q "xxh3_128_scalar" build/abi-symbols.txt
grep -q "xxh32" build/abi-symbols.txt
grep -q "xxh64" build/abi-symbols.txt
grep -q "xxh3_64_sse2" build/abi-symbols.txt
grep -q "xxh3_64_avx2" build/abi-symbols.txt
grep -q "xxh3_64_avx512" build/abi-symbols.txt

echo "ABI symbol check passed"
