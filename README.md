# xxhash-wrapper

SIMD-exporting XXH3 wrapper library for C-compatible consumers.

## Build

```sh
meson setup build
meson compile -C build
```

## Run unit tests

```sh
meson test -C build --print-errorlogs
```

## Public API

- XXH3 single-shot variants: `xxh3_64_<variant>()`, `xxh3_128_<variant>()`
- Streaming API: `xxh3_createState()`, `xxh3_64_reset/update/digest()`, `xxh3_128_reset/update/digest()`
- Secret API: `xxh3_64_withSecret()`, `xxh3_128_withSecret()`, `XXH3_generateSecret()`
- Legacy/traditional scalar exports: `xxh32()`, `xxh64()`

## Platform-Specific Variant Availability (FR-005)

**x86-64 builds** export all SIMD variants:
- `xxh3_64_sse2`, `xxh3_128_sse2` — baseline for 64-bit x86
- `xxh3_64_avx2`, `xxh3_128_avx2` — modern x86
- `xxh3_64_avx512`, `xxh3_128_avx512` — Intel AVX-512
- `xxh3_64_scalar`, `xxh3_128_scalar` — portable C fallback

**aarch64 builds** export only ARM variants:
- `xxh3_64_neon`, `xxh3_128_neon` — AArch64 NEON SIMD
- `xxh3_64_sve`, `xxh3_128_sve` — AArch64 Scalable Vector Extensions
- `xxh3_64_scalar`, `xxh3_128_scalar` — portable C fallback

**Important:** Consumer code that calls platform-unavailable variants will fail at link time. Use compile-time feature detection macros (see below) to guard calls.

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

Set `XXH3_FORCE_SCALAR=1` to force scalar path in testing.
