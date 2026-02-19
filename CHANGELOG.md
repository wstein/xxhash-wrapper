# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Numeric Quad Versioning](specs/001-xxh3-simd-wrapper/spec.md):
`MAJOR.MINOR.PATCH.WRAPPER_PATCH` where the first three digits track the vendored
xxHash version.

---

## [Unreleased]

### Added
- `XXH3_WRAPPER_VERSION_STRING` compile-time version string macro in `xxh3.h`
- `XXH3_SECRET_SIZE_MIN` constant and `XXH3_ASSERT_SECRET_SIZE()` compile-time guard macro
- Streaming APIs for `xxh32` and `xxh64`: `xxh32_reset`, `xxh32_update`, `xxh32_digest`,
  `xxh64_reset`, `xxh64_update`, `xxh64_digest` (FR-021)
- Conditional libFuzzer fuzz target in `meson.build` via `-Dfuzz=true` option (replaces
  unconditional `if false` stub)
- `meson.options` with `fuzz` boolean option (default: false)

### Changed
- `tests/bench/bench_variants.c`: added `_POSIX_C_SOURCE 199309L` guard so
  `clock_gettime` / `CLOCK_MONOTONIC` compile correctly with `-std=c99` on Linux/Alpine

### Fixed
- Duplicate comment block in `include/xxh3.h` (stale `* - xxh3_*_avx2` block removed)

---

## [0.8.3.0] - 2026-02-19

### Added
- Initial wrapper release tracking xxHash **0.8.3**
- Multi-variant export model: `xxh3_64_<variant>` / `xxh3_128_<variant>` for scalar,
  SSE2, AVX2, AVX512 (x86-64) and NEON, SVE (aarch64)
- Streaming API for XXH3: `xxh3_createState`, `xxh3_freeState`, `xxh3_64_reset/update/digest`,
  `xxh3_128_reset/update/digest`
- Secret-based API for XXH3: `xxh3_64_withSecret`, `xxh3_128_withSecret`,
  `xxh3_64_reset_withSecret`, `xxh3_128_reset_withSecret`, `xxh3_generateSecret`
- Legacy exports: `xxh32` (32-bit scalar, single-shot) and `xxh64` (64-bit scalar, single-shot)
- Platform-specific SIMD variants compiled into separate translation units for optimal performance
- `bench_variants` utility for local performance testing
- Meson build system with shared + static library outputs (`-Ddefault_library=both`)
- Unity test framework integration (`tests/unity/`, `unity.config`)
- ABI regression check script (`scripts/check-abi.sh`)
- Integration smoke test (`tests/integration/check-cr-xxhash.sh`)
- Conventional Commits enforcement script (`scripts/check-commit-msg.sh`)
- GitLab CI pipeline (`.gitlab-ci.yml`) with lint / build / test / verify jobs
- GitLab Flow branch strategy documentation (`docs/ci/gitlab-flow.md`)
- `FR-020` Unity testing framework requirement
- `FR-021` Streaming APIs for xxh32/xxh64
- `FR-022` Clarification: xxh32/xxh64 do not support secret-based variants

### Vendored
- xxHash **0.8.3** (`vendor/xxHash/`) — authoritative implementation

[Unreleased]: https://gitlab.example.com/xxhash-wrapper/xxhash-wrapper/-/compare/v0.8.3.0...HEAD
[0.8.3.0]: https://gitlab.example.com/xxhash-wrapper/xxhash-wrapper/-/tags/v0.8.3.0
