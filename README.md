# xxhash-wrapper

SIMD-exporting XXH3 wrapper library for C-compatible consumers.

## Build

### Standard (native platform)

```sh
meson setup build
meson compile -C build
meson test -C build --print-errorlogs
```

### macOS (arm64/Apple Silicon)

```sh
# Native build (no cross-compilation)
meson setup build
meson compile -C build
# Verify ARM variants exported (NEON only on Apple Silicon; SVE not available):
nm -gp build/libxxh3_wrapper.dylib | grep xxh3_64
```

### Linux x86-64 (Alpine - Docker recommended)

```sh
docker run --platform linux/amd64 --rm -v $(pwd):/src:ro alpine:latest sh -c '
  apk add -q meson ninja gcc musl-dev
  mkdir -p /tmp/build && cd /tmp/build
  meson setup /src && meson compile
  # Verify x86 variants exported (SSE2, AVX2, AVX512):
  nm -gp libxxh3_wrapper.so | grep xxh3_64
'
```

### Linux aarch64 (Alpine - Docker)

```sh
docker run --platform linux/arm64 --rm -v $(pwd):/src:ro alpine:latest sh -c '
  apk add -q meson ninja gcc musl-dev
  mkdir -p /tmp/build && cd /tmp/build
  meson setup /src && meson compile
  # Verify ARM variants exported (NEON; SVE if toolchain+CPU support):
  nm -gp libxxh3_wrapper.so | grep xxh3_64
'
```

The build no longer accepts a global `simd_backend` option. Each SIMD variant is compiled in its own translation unit with its own `XXH_VECTOR` and CPU flags, and all variants are linked into the wrapper. See [Platform-Specific Verification Results](docs/PLATFORM_VERIFICATION.md) for detailed build and export examples across platforms.

## Public API

- XXH3 single-shot variants: `xxh3_64_<variant>()`, `xxh3_128_<variant>()` — seeded (explicit seed parameter)
- XXH3 unseeded single-shot variants: `xxh3_64_unseeded()`, `xxh3_128_unseeded()` — dispatches to platform's best variant with seed=0
- XXH3 unseeded architecture-specific variants: `xxh3_64_<variant>_unseeded()`, `xxh3_128_<variant>_unseeded()` (sse2, avx2, avx512, neon, sve, scalar)
- Streaming API: `xxh3_64_reset/update/digest()`, `xxh3_128_reset/update/digest()` — seeded
- Streaming unseeded API: `xxh3_64_reset_unseeded()`, `xxh3_128_reset_unseeded()` — unseeded streaming reset
- State management: `xxh3_createState()`, `xxh3_copyState()` — deep copy for branching workflows (FR-023)
- Secret API: `xxh3_64_withSecret()`, `xxh3_128_withSecret()`, `XXH3_generateSecret()`
- Legacy/traditional scalar exports: `xxh32()`, `xxh64()`

## Platform-Specific Variant Availability (FR-005)

**x86-64 builds** export all SIMD variants:
- `xxh3_64_sse2`, `xxh3_128_sse2` — baseline for 64-bit x86
- `xxh3_64_avx2`, `xxh3_128_avx2` — modern x86
- `xxh3_64_avx512`, `xxh3_128_avx512` — Intel AVX-512
- `xxh3_64_scalar`, `xxh3_128_scalar` — portable C fallback

**aarch64 builds** export only ARM variants:

- `xxh3_64_neon`, `xxh3_128_neon` — AArch64 NEON SIMD (baseline for ARM; always available on aarch64)
- `xxh3_64_sve`, `xxh3_128_sve` — AArch64 Scalable Vector Extensions (unconditionally exported on aarch64; consumer must verify CPU support at runtime if targeting heterogeneous ARM platforms like Apple Silicon vs. broader ARM64 CPUs)
- `xxh3_64_scalar`, `xxh3_128_scalar` — portable C fallback

**Important:** Consumer code that calls platform-unavailable variants will fail at link time. Use compile-time feature detection macros (see below) to guard calls.

### Performance expectations (Apple M2, 1 MiB buffers, release build)

- `xxh3_64_neon`: ~35–50 GB/s (observed ~35.5 GB/s on M2)
- `xxh3_64_scalar`: ~25–36 GB/s (observed ~14–18 GB/s in this simple benchmark; expect higher with tuned clocks and larger buffers)
- `xxh3_64_sve`: not available on M2 (not exported; calling would be a link error)
- `xxh64`: ~25 GB/s target
- `xxh32`: ~11 GB/s target

Note: Throughput varies with compiler, clocks, buffer sizes, and measurement method.

## Compile-Time Feature Detection

Include macros in `xxh3.h` for conditional compilation:

```c
#if XXH3_HAVE_SSE2
    result = xxh3_64_sse2(data, size, seed);
#endif

#if XXH3_HAVE_NEON
    result = xxh3_64_neon(data, size, seed);
#endif
```

Available macros:
- `XXH3_HAVE_X86_SIMD` — x86-64 build
- `XXH3_HAVE_SSE2`, `XXH3_HAVE_AVX2`, `XXH3_HAVE_AVX512` — x86 variants
- `XXH3_HAVE_AARCH64_SIMD` — aarch64 build
- `XXH3_HAVE_NEON`, `XXH3_HAVE_SVE` — ARM variants

## FFI integration notes (cr-xxhash)

Use the exported symbol variants directly from your binding and select the call target in consumer dispatch logic.

Example link args:

```sh
-L/path/to/build -lxxh3_wrapper
```

## CPU feature guidance

**Important:** The library implements **no internal CPU dispatch**. Consumer code is responsible for:

1. **Link-time safety:** Avoid calling unavailable variants (use the compile-time macros above)
2. **Runtime safety (optional):** For variants that exist on the platform, verify runtime CPU support if uncertain:
   - `xxh3_*_sse2`: requires SSE2 (baseline for 64-bit x86, safe to assume present)
   - `xxh3_*_avx2`: requires AVX2 (check at runtime via CPUID if targeting mixed CPU generations)
   - `xxh3_*_avx512`: requires AVX512F and OS vector state support (very restrictive; check CPUID + OS support)
   - `xxh3_*_neon`: requires AArch64 NEON (baseline for ARM; safe to assume present on aarch64 builds)
   - `xxh3_*_sve`: requires AArch64 SVE (check at runtime; not all ARM CPUs support SVE)

This library provides exported symbols per variant but does not do runtime CPU dispatch. Select the appropriate symbol in your consumer. For quick local comparisons, use the provided benchmark:

```sh
./build/bench_variants
```

## What's NOT Exported

This wrapper focuses on core XXH3 hashing and intentionally omits the following vendor functions, per the "simple wrapper" design philosophy:

- **XXH3 with Secret AND Seed:** `XXH3_64bits_withSecretandSeed()`, `XXH3_128bits_withSecretandSeed()` — can be achieved by deriving a secret from seed and using `xxh3_64_withSecret()`
- **Direct Seed-to-Secret:** `XXH3_generateSecret_fromSeed()` — use the general `xxh3_generateSecret()` instead
- **XXH128 Comparison:** `XXH128_isEqual()`, `XXH128_cmp()` — use struct field comparison or `memcmp()` instead
- **Canonical Representation:** `XXH32_canonicalFromHash()`, `XXH64_canonicalFromHash()`, etc. — implement in consumer code if needed

Consumers needing any of these functions can implement them locally by calling the exported wrapper functions.
