# Feature Specification: SIMD-Optimized XXH3 C Library with Runtime CPU Dispatch

**Feature Branch**: `001-xxh3-simd-wrapper`  
**Created**: 2026-02-18  
**Status**: Draft  
**Input**: User description: "Provides a wrapper that builds and exports multiple SIMD‑optimized implementations of the XXH3 hash algorithm (64‑ and 128‑bit) as a single, portable C-compatible library. It is used as the native backend for the cr-xxhash Crystal shard and references the original xxHash project for implementation details and compatibility. By bundling several SIMD variants, this library makes it easy to build and select the fastest implementation for a given CPU at build- or runtime, simplifying integration for language bindings and projects that need high-performance XXH3 hashing."

## User Scenarios & Testing *(mandatory)*

<!--
  IMPORTANT: User stories should be PRIORITIZED as user journeys ordered by importance.
  Each user story/journey must be INDEPENDENTLY TESTABLE - meaning if you implement just ONE of them,
  you should still have a viable MVP (Minimum Viable Product) that delivers value.
  
  Assign priorities (P1, P2, P3, etc.) to each story, where P1 is the most critical.
  Think of each story as a standalone slice of functionality that can be:
  - Developed independently
  - Tested independently
  - Deployed independently
  - Demonstrated to users independently
-->

### User Story 1 - Language Binding Integration (Priority: P1)

A language binding (e.g., Crystal's cr-xxhash) or downstream application needs to use XXH3 hashing functions. It links the C library and calls `xxh3_64()` or `xxh3_128()` functions; the library automatically detects CPU features and dispatches to the fastest available SIMD implementation at runtime.

**Why this priority**: This is the primary consumer use case and delivers immediate value — end users get fast, correct hashing without manual configuration.

**Independent Test**: Can be tested by verifying that a C/Crystal program linked against the library can hash data and receive correct results; no runtime crashes or crashes on unsupported CPUs.

**Acceptance Scenarios**:

1. **Given** a C program calls `xxh3_64(data, size, seed)`, **When** the function returns, **Then** the result matches the canonical xxHash reference implementation bit-for-bit.
2. **Given** a program runs on x86_64 with SSE2, **When** `xxh3_64()` is called, **Then** the SSE2 implementation is used and performance is measurably faster than scalar.
3. **Given** a program runs on aarch64 with NEON, **When** `xxh3_128()` is called, **Then** the NEON implementation is used and output matches reference.
4. **Given** a program runs on a CPU without supported SIMD, **When** any XXH3 function is called, **Then** the scalar fallback is used and produces correct results.

---

### User Story 2 - Library Maintenance & Optimization (Priority: P2)

A maintainer needs to add support for a new SIMD variant (e.g., AVX-512, new aarch64 features) or optimize an existing path. They add the implementation, ensure it passes tests (unit, SIMD alignment, fuzz), benchmarks show improvement, and the new code is integrated without breaking existing functionality.

**Why this priority**: Enables future performance improvements and platform coverage; critical for long-term project health but not blocking initial release.

**Independent Test**: Can be tested by verifying that the new SIMD variant is called on matching CPUs, produces identical outputs to reference, and benchmark baselines are recorded without regression.

**Acceptance Scenarios**:

1. **Given** a new SIMD variant is added to the codebase, **When** CI runs, **Then** SIMD/platform tests pass on target CPU and scalar tests pass on all platforms.
2. **Given** the new variant is enabled on a supported CPU, **When** benchmarks are recorded, **Then** performance improves or is documented as a trade-off.
3. **Given** the new variant is built for an unsupported CPU, **When** the library runs, **Then** the previous best implementation or scalar is used (safe fallback).

---

### User Story 3 - Correctness Validation & Testing (Priority: P1)

Users and CI verify that the library produces correct hashing output across all supported platforms and SIMD variants. This includes unit tests comparing outputs to xxHash, property/fuzz tests with diverse inputs, and ABI regression checks ensuring no silent binary incompatibilities.

**Why this priority**: Correctness is non-negotiable; this story ensures the library is trustworthy for production use.

**Independent Test**: Can be tested by running the full test suite (unit + SIMD/platform + fuzz) and verifying all outputs match canonical xxHash; ABI checks confirm symbol stability.

**Acceptance Scenarios**:

1. **Given** unit tests compare outputs across SIMD variants, **When** tests run, **Then** all variants produce identical results for the same input.
2. **Given** fuzz harness is run with random inputs, **When** tests complete, **Then** no crashes, undefined behaviour, or output mismatches occur.
3. **Given** an ABI compatibility test, **When** the library is rebuilt at the current version, **Then** all public symbols match previous builds.
4. **Given** the underlying xxHash algorithm is updated, **When** the library is updated, **Then** outputs remain identical to the canonical version for all supported XXH3 variants.

---

### Edge Cases

- What happens when the library is built on a CPU with AVX2 but runs on one without it? (Must gracefully fall back to SSE or scalar.)
- How does the library behave when passed NULL pointers or size=0? (Must not crash and must produce valid output per xxHash spec.)
- What if a CPU claims to support AVX-512 but the CPU-detection logic incorrectly identifies it? (Must fall back safely; test all detection paths.)
- Can the library be used in signal handlers or async contexts? (Must document constraints, no undefined behavior.)
- How does the library handle extremely large inputs (>4GB)? (Must handle 64-bit size parameters correctly and not overflow size_t.)


## Requirements *(mandatory)*

<!--
  ACTION REQUIRED: The content in this section represents placeholders.
  Fill them out with the right functional requirements.
-->

### Functional Requirements

- **FR-001**: Library MUST export **single-shot** hash functions: `uint64_t xxh3_64(const void* input, size_t size, uint64_t seed)` and `xxh3_128_t xxh3_128(const void* input, size_t size, uint64_t seed)`, where `xxh3_128_t` is a struct containing two `uint64_t` fields (high/low).
- **FR-002**: Library MUST also export **streaming APIs**: `xxh3_state_t`, `xxh3_state_t* xxh3_createState()`, `void xxh3_freeState(xxh3_state_t*)`, `void xxh3_64_reset(xxh3_state_t*, uint64_t seed)`, `XXH_errorcode xxh3_64_update(xxh3_state_t*, const void* input, size_t size)`, `uint64_t xxh3_64_digest(xxh3_state_t*)` (and equivalent 128-bit variants).
- **FR-003**: Library MUST export **secret-based variants**: `xxh3_64_withSecret(const void* input, size_t size, const void* secret, size_t secretSize)` and equivalent streaming reset/update for secret-based hashing.
- **FR-004**: Library MUST export `void XXH3_generateSecret(void* secretBuffer, size_t secretSize, uint64_t seed)` to derive custom secrets from seeds.
- **FR-005**: Library MUST detect CPU capabilities at runtime on x86/x64 (SSE2, SSE4.2, AVX2, AVX512) and ARM aarch64 (NEON, SVE) and select the fastest compatible SIMD implementation based on runtime detection.
- **FR-006**: For non-x86/x64 platforms (ARM aarch64), SIMD variant is selected at **compile-time** based on build flags (e.g., `-march=native`, explicit CPU target).
- **FR-007**: Library MUST provide a scalar (non-SIMD) fallback implementation that is compiled and available on all platforms as a safe default.
- **FR-008**: All SIMD implementations (SSE2, AVX2, AVX512, NEON, SVE) MUST produce identical output to the scalar reference and the canonical xxHash library for identical inputs.
- **FR-009**: Library MUST accept inputs of any size, including empty (size=0) and very large inputs (up to `SIZE_MAX`).
- **FR-010**: Library MUST be distributed as a compiled C-compatible library (static and/or shared) usable from C, C++, and language bindings via FFI.
- **FR-011**: Library MUST include a header file (xxh3.h) that declares the public API, documents function behaviour, parameters, return types, and provides usage examples.
- **FR-012**: Library MUST be buildable with **Meson** and generate reproducible artifacts (same binary given same toolchain and flags).
- **FR-013**: Library MUST support CPU feature overrides or environment-based control (optional, e.g., `XXH3_FORCE_SCALAR=1`) for testing and debugging.
- **FR-014**: Library MUST support both dynamic (runtime) linking and static linking for all platforms.

### Key Entities

- **Hash State** (`xxh3_state_t`): Internal state structure (opaque to users) that holds algorithm state during streaming (incremental) hashing; used with `reset()`, `update()`, `digest()`.
- **CPU Feature Bitmap**: Runtime-detected CPU capability flags on x86/x64 (SSE2, SSE4.2, AVX2, AVX512); ARM aarch64 features detected at compile-time.
- **SIMD Variant**: A specific compiled implementation of XXH3 (scalar, SSE2, AVX2, AVX512, NEON, SVE) with identical logic but optimized CPU-specific instructions.
- **Seed**: A 64-bit input parameter that modifies the hash output; enables domain separation and hash randomization.
- **Secret**: A 136-192 byte blob used for collision resistance; can be derived from seed via `XXH3_generateSecret()` or provided custom.


## Success Criteria *(mandatory)*

<!--
  ACTION REQUIRED: Define measurable success criteria.
  These must be technology-agnostic and measurable.
-->

### Measurable Outcomes

- **SC-001**: All hash outputs (64-bit and 128-bit) MUST match the canonical xxHash reference implementation bit-for-bit for any input (validated by unit tests and property tests with >1M random inputs).
- **SC-002**: SIMD implementations (SSE2, AVX2, NEON) MUST run on their target CPUs and produce identical outputs to the scalar implementation.
- **SC-003**: The library MUST run without crashes, undefined behaviour, or data corruption on all supported platforms (Linux x86_64/aarch64, macOS x86_64/arm64); verified by fuzz tests, AddressSanitizer, and MemorySanitizer runs.
- **SC-004**: Performance improvements MUST be measurable: SIMD implementations must demonstrate ≥2x throughput improvement over scalar on supporting CPUs, or trade-off is documented and justified.
- **SC-005**: ABI/binary compatibility MUST be preserved across patch versions; breaking ABI changes MUST be detected by CI and require explicit versioning.
- **SC-006**: Build artifacts MUST be reproducible: rebuilding with identical toolchain and flags MUST produce identical binary checksums.
- **SC-007**: Integration with cr-xxhash MUST succeed: the Crystal shard MUST link the library via FFI and pass all its tests without modification.
- **SC-008**: Documentation MUST include usage examples (C/C++/FFI), supported CPU targets, performance baselines per platform, and API reference; examples MUST compile and run correctly.

## Clarifications

### Session 2026-02-19

- Q: Should MVP include streaming (state-based) hashing? → A: **Yes, both single-shot and streaming APIs**.
- Q: Which platforms should be supported (runtime vs compile-time dispatch)? → A: **x86/x64 (runtime dispatch) + ARM aarch64 (compile-time NEON/scalar)**.
- Q: Which build system (CMake vs Makefile vs other)? → A: **Meson** (not CMake or Makefile).
- Q: Should the API expose secret management, or just seed? → A: **Expose both seeds and secrets** (`xxh3_*_withSeed()`, `xxh3_*_withSecret()`, `XXH3_generateSecret()`).
- Q: Should AVX512 support be included in MVP? → A: **Yes, include AVX512 from day 1** (requires emulation/hardware for CI).
- Q: Should header-only mode be supported? → A: **No, library-only mode** (dynamic/static linking, no inlining; simpler).
- Q: Should symbol namespacing (`XXH_NAMESPACE` macro) be supported? → A: **No, fixed prefix** (`xxh3_`) for simplicity.

## Assumptions

- The xxHash reference implementation (https://github.com/Cyan4973/xxHash) is authoritative for correctness and algorithm details.
- **Platforms**: MVP supports Linux and macOS on x86_64 and aarch64 (ARM64).
  - **x86/x64**: Runtime CPU feature detection at startup (SSE2, SSE4.2, AVX2, AVX512); dispatch via function pointers to fastest available.
  - **ARM aarch64**: Compile-time SIMD selection (NEON, SVE based on build flags `meson setup -Dcpu_family=aarch64`); scalar fallback always included.
- **Build System**: Meson is the primary build tool; no CMake or Makefile.
- **API Scope**: Both **single-shot** (`xxh3_64()`, `xxh3_128()`) and **streaming** (`createState/reset/update/digest`) APIs are public; both seed-based and secret-based variants exposed.
- **Linking**: Library is distributed as compiled artifacts (static + shared); no header-only mode (users must link against library).
- **Symbols**: Fixed public symbol prefix (`xxh3_*`, `XXH3_*`); no namespace macro support.
- **Error Handling**: Contract-based (preconditions documented); invalid inputs (e.g., NULL with non-zero size) result in undefined behavior.
- **Threading**: No mutable shared state; library is reentrant and thread-safe for concurrent read-only hashing.
