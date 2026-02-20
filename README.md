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

## CI/CD

The project uses **GitHub Actions** for continuous integration. The workflow (`.github/workflows/ci.yml`) runs on every push and pull request and includes:

- **Linting:** C99 compliance check, commit message validation
- **Build:** Multi-platform builds (Linux x86-64, Linux aarch64, macOS arm64) plus variant builds:
  - Standard release build
  - XXH_INLINE_ALL build (detects include-order regressions)
  - Debug build (validates guard branch coverage)
- **Test:** Unit tests, variant tests (inline, debug), ABI checks, integration tests
- **Verify:** Benchmark runs, reproducible build checks

View the workflow status and logs on the [repository's Actions page](https://github.com/wstein/xxhash-wrapper/actions).

## Public API

- XXH3 single-shot variants: `xxh3_64_<variant>()`, `xxh3_128_<variant>()` — seeded (explicit seed parameter)
- XXH3 unseeded single-shot variants: per-variant unseeded functions only (e.g. `xxh3_64_scalar_unseeded()`, `xxh3_64_avx2_unseeded()`). The wrapper does **not** provide generic dispatcher functions; consumers must call the desired variant directly.
- XXH3 unseeded architecture-specific variants: `xxh3_64_<variant>_unseeded()`, `xxh3_128_<variant>_unseeded()` (sse2, avx2, avx512, neon, sve, scalar)
- XXH3 advanced variants: `xxh3_64_withSecretandSeed()`, `xxh3_128_withSecretandSeed()` — delegate to vendor implementations (custom secret + seed)
- Streaming API: `xxh3_64_reset/update/digest()`, `xxh3_128_reset/update/digest()` — seeded
- Streaming unseeded API: `xxh3_64_reset_unseeded()`, `xxh3_128_reset_unseeded()` — unseeded streaming reset
- State management: `xxh3_createState()`, `xxh3_copyState()` — deep copy for branching workflows (FR-023)
- Secret API: `xxh3_64_withSecret()`, `xxh3_128_withSecret()`, `xxh3_generateSecret()`, `xxh3_generateSecret_fromSeed()`
- XXH128 Comparison: `xxh3_128_isEqual()`, `xxh3_128_cmp()` — compare 128-bit hash values
- XXH32 Canonical Representation: `xxh32_canonicalFromHash()`, `xxh32_hashFromCanonical()` — big-endian serialization
- XXH64 Canonical Representation: `xxh64_canonicalFromHash()`, `xxh64_hashFromCanonical()` — big-endian serialization
- XXH128 Canonical Representation: `xxh128_canonicalFromHash()`, `xxh128_hashFromCanonical()` — big-endian serialization (high64 first, then low64)
- Legacy/traditional scalar exports: `xxh32()`, `xxh64()`

Example: serialize XXH128 to a 16-byte canonical buffer

```c
xxh3_128_t h = xxh3_128_scalar(data, len, seed);
xxh128_canonical_t c;
xxh128_canonicalFromHash(&c, h);
/* c.digest now contains 16 bytes in big-endian order: high64 then low64 */

/* Restore back to numeric form */
xxh3_128_t restored = xxh128_hashFromCanonical(&c);
```

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

## Consumer Dispatch Patterns

**Why no internal dispatcher?** This wrapper intentionally provides no generic `xxh3_64()` dispatcher. This design choice enforces consumer responsibility for variant selection, avoiding hidden performance overhead from implicit CPU detection and making dispatch behavior explicit and observable.

**Your responsibility:** Call the variant function directly. Here are recommended patterns:

**x86-64 (compile-time selection with runtime safety):**

```c
/* Simple approach: select the fastest available at compile time */
#if XXH3_HAVE_AVX2
    #define HASH_FUNC xxh3_64_avx2
#elif XXH3_HAVE_SSE2
    #define HASH_FUNC xxh3_64_sse2
#else
    #define HASH_FUNC xxh3_64_scalar
#endif

uint64_t hash = HASH_FUNC(data, size, seed);
```

**aarch64 (platform baseline with optional runtime check):**

```c
/* NEON is always available on aarch64; SVE requires runtime check */
#if XXH3_HAVE_SVE
    /* Check CPU support if targeting heterogeneous ARM platforms */
    int supports_sve = check_sve_support();  /* Your implementation */
    uint64_t hash = supports_sve ? xxh3_64_sve(data, size, seed)
                                 : xxh3_64_neon(data, size, seed);
#else
    uint64_t hash = xxh3_64_neon(data, size, seed);
#endif
```

**For runtime dispatch (advanced):** Implement a CPU detection function (CPUID on x86, /proc/cpuinfo or syscalls on ARM) and select `xxh3_64_sse2`, `xxh3_64_avx2`, `xxh3_64_avx512`, `xxh3_64_neon`, or `xxh3_64_sve` accordingly.

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

**Debug vs release guard behaviour:** All NULL-pointer and invalid-input guards in the wrapper and variant functions are compiled with `#ifndef NDEBUG` and are **active only in debug builds** (`meson setup -Dbuildtype=debug`). Release builds (the default; `buildtype=release`) have `-DNDEBUG` set by meson, which strips all guards completely. Callers must therefore guarantee valid inputs in release builds — passing a NULL pointer or mismatched state produces undefined behaviour.

This library provides exported symbols per variant but does not do runtime CPU dispatch. Select the appropriate symbol in your consumer. For quick local comparisons, use the provided benchmark:

```sh
./build/bench_variants
```
