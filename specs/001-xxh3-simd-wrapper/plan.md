# Implementation Plan: SIMD-Optimized XXH3 C Library with Multi-Variant Export

**Feature**: `001-xxh3-simd-wrapper`
**Status**: Draft
**Created**: 2026-02-19

## Tech Stack & Libraries

- **Language**: C99 (`-std=c99`) for wrapper and exported header
- **Build System**: Meson (v1.0.0+)
- **Underyling Library**: xxHash (v0.8.3, vendored in `vendor/xxHash`)
- **Testing Framework**: Custom C-based test runner (for SIMD variant comparisons and fuzz testing)
- **CI/CD**: Gitlab CI (as per project structure)
- **Supported Platforms**:
  - Linux (x86_64, aarch64)
  - macOS (x86_64, arm64)

## Project Structure

```text
.
├── include/
│   └── xxh3.h                 # Public API header (C99)
├── src/
│   ├── xxh3_wrapper.c         # Main wrapper implementation
│   ├── variants/
│   │   ├── x86/
│   │   │   ├── sse2.c         # SSE2-specific compilation unit
│   │   │   ├── avx2.c         # AVX2-specific compilation unit
│   │   │   └── avx512.c       # AVX512-specific compilation unit
│   │   └── arm/
│   │       ├── neon.c         # NEON-specific compilation unit
│   │       └── sve.c          # SVE-specific compilation unit
│   └── common/
│       └── internal_utils.h   # Shared macros and internal helpers
├── tests/
│   ├── unit/                  # Correctness tests (comparing variants)
│   ├── integration/           # Build and link tests
│   └── fuzz/                  # Fuzz testing harness
├── vendor/
│   └── xxHash/                # Original xxHash source (git submodule or copy)
└── meson.build                # Master build file
```

## Implementation Strategy

### 1. Project Initialization & Build System (Phase 1)
- Set up Meson build structure.
- Define library targets (static and shared).
- Configure compiler flags for different SIMD variants (`-msse2`, `-mavx2`, `-mavx512f`, etc.).
- Ensure `vendor/xxHash` is correctly linked/included.

### 2. Public API Definition (Phase 1)
- Create `include/xxh3.h` with all required function signatures (`xxh3_64_scalar`, `xxh3_64_sse2`, etc.).
- Define the opaque `xxh3_state_t` structure.
- Document preconditions for SIMD variants (CPUID requirements).

### 3. SIMD Variant Implementation (Phase 2 & 3)
- Use a "trampoline" or separate compilation unit approach to ensure SIMD instructions are only emitted where intended.
- **x86/x64**: Implement SSE2, AVX2, and AVX512 wrappers. These must always be compiled into the binary.
- **ARM aarch64**: Implement NEON and SVE wrappers. These are conditionally compiled based on the target architecture.
- **Scalar**: Implement the baseline scalar fallback.
- **Legacy/Traditional**: Export `xxh32` and `xxh64` directly from the vendored source.

### 4. Streaming API Implementation (Phase 3)
- Implement `xxh3_createState()`, `xxh3_freeState()`, `reset()`, `update()`, and `digest()`.
- Ensure the state correctly tracks the selected variant logic (though the user selects the variant at `reset()` time).

### 5. Correctness & Validation (Phase 4)
- Unit tests to compare outputs of all variants against scalar reference.
- Property-based/fuzz tests to ensure stability across large/empty inputs.
- Benchmark suite to verify performance gains (SC-004).

### 6. Polish & Documentation (Phase 5)
- Finalize `xxh3.h` documentation and usage examples.
- Ensure compliance with Inclusive Naming and Conventional Commits.
- Verify reproducible builds.

## Key Decisions

- **No Internal Dispatch**: We will export individual symbols. This simplifies the library significantly and gives full control to the consumer (e.g., Crystal's shard).
- **Separate Compilation Units**: To avoid CPU feature leakage, each SIMD variant will be compiled in its own file with specific compiler flags.
- **C99 Compliance**: Strictly enforce `-std=c99` for all non-vendored code.
- **Versioning**: Follow `MAJOR.MINOR.PATCH.WRAPPER_PATCH` (e.g., `0.8.3.0`).

## Dependencies

- **xxHash**: Vendored code in `vendor/xxHash`.
- **Meson**: Build tool.
- **Compiler**: GCC or Clang with support for AVX512 and ARM SVE.
