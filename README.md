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

## FFI integration notes (cr-xxhash)

Use the exported symbol variants directly from your binding and select the call target in consumer dispatch logic.

Example link args:

```sh
-L/path/to/build -lxxh3_wrapper
```

## CPU feature guidance

- `xxh3_*_sse2`: requires SSE2
- `xxh3_*_avx2`: requires AVX2
- `xxh3_*_avx512`: requires AVX512F and OS vector state support
- `xxh3_*_neon`: requires AArch64 NEON
- `xxh3_*_sve`: requires AArch64 SVE

Set `XXH3_FORCE_SCALAR=1` to force scalar path in testing.
