# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Numeric Quad Versioning](specs/001-xxh3-simd-wrapper/spec.md):
`MAJOR.MINOR.PATCH.WRAPPER_PATCH` where the first three digits track the vendored
xxHash version.

---

## [Unreleased]

---

## [0.8.3.0] - 2026-02-20

### Added
- Initial wrapper release tracking xxHash **0.8.3**
- Multi-variant export model: `xxh3_64_<variant>` / `xxh3_128_<variant>` for scalar,
  SSE2, AVX2, AVX512 (x86-64) and NEON, SVE (aarch64)
- Streaming API for XXH3: `xxh3_createState`, `xxh3_freeState`, `xxh3_64_reset/update/digest`,
  `xxh3_128_reset/update/digest`
- Streaming APIs for `xxh32` and `xxh64`: `xxh32_reset`, `xxh32_update`, `xxh32_digest`,
  `xxh64_reset`, `xxh64_update`, `xxh64_digest` (FR-021)
- Secret-based API for XXH3: `xxh3_64_withSecret`, `xxh3_128_withSecret`,
  `xxh3_64_reset_withSecret`, `xxh3_128_reset_withSecret`, `xxh3_generateSecret`
- Unseeded single-shot and streaming variants for all XXH3 per-variant symbols (e.g. `xxh3_64_scalar_unseeded()`, `xxh3_64_avx2_unseeded()`, `xxh3_128_neon_unseeded()`), `xxh3_64_reset_unseeded()`, `xxh3_128_reset_unseeded()`, plus all architecture-specific variants (sse2, avx2, avx512, neon, sve, scalar)
- Legacy exports: `xxh32` (32-bit scalar, single-shot) and `xxh64` (64-bit scalar, single-shot)
- Platform-specific SIMD variants compiled into separate translation units for optimal performance
- `bench_variants` utility for local performance testing
- Meson build system with shared + static library outputs (`-Ddefault_library=both`)
- Unity test framework integration (`tests/unity/`, `unity.config`)
- ABI regression check script (`scripts/check-abi.sh`)
- Integration smoke test (`tests/integration/check-cr-xxhash.sh`)
- Conventional Commits enforcement script (`scripts/check-commit-msg.sh`)
- GitHub Actions CI workflow (`.github/workflows/ci.yml`): linting (C99 compliance, test feature macros, commits), multi-platform builds (Linux x86-64, Linux aarch64, macOS arm64), variant testing (inline, debug, ABI, integration), verification (benchmarks, reproducibility)
- `XXH3_WRAPPER_VERSION_STRING` compile-time version string macro in `xxh3.h`
- `XXH3_SECRET_SIZE_MIN` constant and `XXH3_ASSERT_SECRET_SIZE()` compile-time guard macro
- Conditional libFuzzer fuzz target in `meson.build` via `-Dfuzz=true` option
- `meson.options` with `fuzz` boolean option (default: false)
- Debug-only parameter validation guards (`#ifndef NDEBUG` blocks) on all wrapper delegate functions in `src/xxh3_wrapper.c` and per-variant functions in `src/variants/*.c`; `XXH3_DEBUG_ASSERT(expr)` macro in `src/common/internal_utils.h`

### Changed
- Refactor: use C99 compound literals for `xxh3_128_t` zero returns in variant sources; make `XXH3_WRAPPER_GUARD` variadic to safely support compound literals inside guard bodies
- `tests/bench/bench_variants.c`: added `_POSIX_C_SOURCE 199309L` guard for `-std=c99` compatibility with `clock_gettime`/`CLOCK_MONOTONIC` on Linux/Alpine
- GitLab CI pipeline (`.gitlab-ci.yml`) with lint / build / test / verify jobs → **replaced by GitHub Actions**
- GitLab Flow branch strategy documentation (`docs/ci/gitlab-flow.md`)

### Fixed
- Duplicate comment block in `include/xxh3.h` (stale `* - xxh3_*_avx2` block removed)
- Guard macro usage: prevent preprocessor argument-splitting in `-std=c99`/`NDEBUG` builds by wrapping multi-statement `XXH3_WRAPPER_GUARD` arguments in `{ ... }`
- CI artifact execute permissions: `actions/upload-artifact@v4` strips executable bits. Added `chmod +x` to test and benchmark jobs to restore permissions before execution
- Test source POSIX/XSI feature macros: added `#define _XOPEN_SOURCE 700` to `tests/unit/test_variants.c` for signal/setjmp compatibility with `-std=c99`; created `scripts/check-test-feature-macros.sh` to enforce this project-wide
- Signal guard wrappers: wrapped AVX2 and AVX512 test calls with `TEST_TRY_VARIANT()` to gracefully skip unsupported CPU instructions and avoid SIGILL crashes on heterogeneous CI runners

### Vendored
- xxHash **0.8.3** (`vendor/xxHash/`) — authoritative implementation

[Unreleased]: https://github.com/wstein/xxhash-wrapper/compare/0.8.3.0...HEAD
[0.8.3.0]: https://github.com/wstein/xxhash-wrapper/releases/tag/0.8.3.0
