#!/usr/bin/env bash
#
# Build script: sets up Meson and runs benchmark
# Usage: ./build_and_bench.sh
#

set -euo pipefail

BUILD_DIR="buildDir"

echo "=========================================="
echo "xxhash-wrapper Meson Build & Benchmark"
echo "=========================================="
echo ""
echo "Build dir: $BUILD_DIR"
echo ""

# Clean if requested
if [ "${CLEAN:-0}" = "1" ]; then
  echo "Cleaning old build..."
  rm -rf "$BUILD_DIR"
fi

# Setup Meson
echo "Setting up Meson build..."
if [ ! -d "$BUILD_DIR" ]; then
  meson setup "$BUILD_DIR"
else
  echo "Build directory already exists, skipping setup."
fi

# Compile
echo ""
echo "Compiling..."
meson compile -C "$BUILD_DIR"
sync

# Run benchmark
echo ""
echo "=========================================="
echo "Running Benchmark"
echo "=========================================="
echo ""

"$BUILD_DIR"/bench_variants

echo ""
echo "=========================================="
echo "Benchmark complete!"
echo "=========================================="

