#!/usr/bin/env sh
set -eu

meson setup build-repro-a --wipe
meson compile -C build-repro-a

meson setup build-repro-b --wipe
meson compile -C build-repro-b

LIB_A="build-repro-a/libxxh3_wrapper.so"
LIB_B="build-repro-b/libxxh3_wrapper.so"

if [ -f "build-repro-a/libxxh3_wrapper.dylib" ]; then
  LIB_A="build-repro-a/libxxh3_wrapper.dylib"
  LIB_B="build-repro-b/libxxh3_wrapper.dylib"
fi

if command -v shasum >/dev/null 2>&1; then
  SH_A=$(shasum -a 256 "$LIB_A" | awk '{print $1}')
  SH_B=$(shasum -a 256 "$LIB_B" | awk '{print $1}')
else
  SH_A=$(sha256sum "$LIB_A" | awk '{print $1}')
  SH_B=$(sha256sum "$LIB_B" | awk '{print $1}')
fi

echo "A=$SH_A"
echo "B=$SH_B"

if [ "$SH_A" != "$SH_B" ]; then
  echo "reproducibility check failed"
  exit 1
fi

echo "reproducibility check passed"
