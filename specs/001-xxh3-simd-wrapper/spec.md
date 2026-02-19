# Feature Specification: SIMD-Optimized XXH3 C Library with Multi-Variant Export

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

A language binding (e.g., Crystal's cr-xxhash) or downstream application needs to use XXH3 hashing functions. It links the C library and calls one of the exported variant functions (`xxh3_64_scalar()`, `xxh3_64_sse2()`, `xxh3_64_avx2()`, etc.). The library does **not** implement internal CPU dispatch; instead it exports each SIMD variant as a distinct named symbol, and the consumer chooses which variant to call — implementing their own runtime dispatch, build-time selection, or configuration logic as needed.

The library additionally exports scalar-only `xxh32()` (legacy, 32-bit) and `xxh64()` (traditional, 64-bit) functions. `xxh64()` is particularly relevant as a high-performance scalar fallback: on CPUs without supported SIMD, `xxh64()` may outperform `xxh3_64_scalar()` due to its simpler algorithm, making it a practical default when SIMD variants are unavailable.

**Why this priority**: This is the primary consumer use case and delivers immediate value — end users get fast, correct hashing without manual configuration.

**Independent Test**: Can be tested by verifying that a C/Crystal program linked against the library can hash data and receive correct results; no runtime crashes or crashes on unsupported CPUs.

**Acceptance Scenarios**:

1. **Given** a C program calls `xxh3_64(data, size, seed)`, **When** the function returns, **Then** the result matches the canonical xxHash reference implementation bit-for-bit.
2. **Given** a program on x86_64 calls `xxh3_64_sse2(data, size, seed)`, **When** the function returns, **Then** the result matches the canonical xxHash reference implementation bit-for-bit and performance is measurably faster than scalar.
3. **Given** a program on aarch64 calls `xxh3_128_neon(data, size, seed)`, **When** the function returns, **Then** the result matches the canonical 128-bit xxHash reference implementation.
4. **Given** a program calls `xxh3_64_scalar(data, size, seed)`, **When** the function returns, **Then** the scalar fallback produces correct results on any supported platform.
5. **Given** a program on a CPU with no SIMD support calls `xxh64(data, size, seed)`, **When** the function returns, **Then** the result matches the canonical xxHash XXH64 reference and throughput meets or exceeds `xxh3_64_scalar()` on that CPU.

---

### User Story 2 - Library Maintenance & Optimization (Priority: P2)

A maintainer needs to add support for a new SIMD variant (e.g., AVX-512, new aarch64 features) or optimize an existing path. They add the implementation, ensure it passes tests (unit, SIMD alignment, fuzz), benchmarks show improvement, and the new code is integrated without breaking existing functionality.

**Why this priority**: Enables future performance improvements and platform coverage; critical for long-term project health but not blocking initial release.

**Independent Test**: Can be tested by verifying that the new SIMD variant is called on matching CPUs, produces identical outputs to reference, and benchmark baselines are recorded without regression.

**Acceptance Scenarios**:

1. **Given** a new SIMD variant is added to the codebase, **When** CI runs, **Then** SIMD/platform tests pass on target CPU and scalar tests pass on all platforms.
2. **Given** the new variant is enabled on a supported CPU, **When** benchmarks are recorded, **Then** performance improves or is documented as a trade-off.
3. **Given** a new variant is exported for a specific CPU, **When** the consumer's dispatch layer selects the correct variant for the running CPU, **Then** the appropriate implementation is called (safe fallback to scalar is the consumer's responsibility; the library guarantees all exported variant functions are correct for any valid input).

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

- What happens when the consumer calls an AVX2 variant on a CPU that does not support AVX2? (Undefined behavior — the consumer MUST query CPU capabilities before calling a SIMD-specific variant. The library documents required CPU features as a precondition per variant. The scalar variant is always safe to call on any platform.)
- How does the library behave when passed NULL pointers or size=0? (Must not crash and must produce valid output per xxHash spec.)
- What if a CPU incorrectly reports AVX-512 support to the consumer's detection logic? (The consumer MUST use CPUID + XGETBV to verify both hardware support and OS register enablement before calling `xxh3_*_avx512()`. Required detection steps are documented in the library header.)
- Can the library be used in signal handlers or async contexts? (Must document constraints, no undefined behavior.)
- How does the library handle extremely large inputs (>4GB)? (Must handle 64-bit size parameters correctly and not overflow size_t.)


## Requirements *(mandatory)*

<!--
  ACTION REQUIRED: The content in this section represents placeholders.
  Fill them out with the right functional requirements.
-->

### Functional Requirements

- **FR-001**: Library MUST export **XXH3 single-shot** functions as separately named per-variant symbols (e.g., `uint64_t xxh3_64_scalar(const void* input, size_t size, uint64_t seed)`, `xxh3_64_sse2()`, `xxh3_64_avx2()`, …) and equivalent `xxh3_128_<variant>()` functions returning `xxh3_128_t` (struct with `uint64_t high, low`). See FR-005/FR-006 for the full variant matrix. See FR-016 for legacy scalar exports.
- **FR-002**: Library MUST also export **streaming APIs**: `xxh3_state_t`, `xxh3_state_t* xxh3_createState()`, `void xxh3_freeState(xxh3_state_t*)`, `void xxh3_64_reset(xxh3_state_t*, uint64_t seed)`, `XXH_errorcode xxh3_64_update(xxh3_state_t*, const void* input, size_t size)`, `uint64_t xxh3_64_digest(xxh3_state_t*)` (and equivalent 128-bit variants).
- **FR-003**: Library MUST export **secret-based variants**: `xxh3_64_withSecret(const void* input, size_t size, const void* secret, size_t secretSize)` and equivalent streaming reset/update for secret-based hashing.
- **FR-004**: Library MUST export `void XXH3_generateSecret(void* secretBuffer, size_t secretSize, uint64_t seed)` to derive custom secrets from seeds.
- **FR-005**: Library MUST export each SIMD variant as a **separately named public function** (e.g., `xxh3_64_scalar()`, `xxh3_64_sse2()`, `xxh3_64_avx2()`, `xxh3_64_avx512()`, `xxh3_128_neon()`). The library does **not** implement internal CPU dispatch; consumers implement their own runtime dispatch, build-time selection, or configuration mechanism as appropriate for their use case.
- **FR-006**: x86/x64 SIMD variant functions (scalar, SSE2, AVX2, AVX512) MUST be compiled into every x86/x64 library build and always exported. ARM aarch64 variant functions (NEON, SVE) are exported only when the library is compiled targeting an aarch64 platform via Meson build flags.
- **FR-007**: Library MUST always export XXH3 scalar fallback functions (`xxh3_64_scalar()`, `xxh3_128_scalar()`, and streaming equivalents) on every supported platform. Scalar variants are safe to call unconditionally without CPU capability checks. `xxh64()` (FR-016) is also always unconditionally safe and may be preferred over `xxh3_64_scalar()` on CPUs without SIMD support due to its simpler algorithm.
- **FR-008**: All SIMD implementations (SSE2, AVX2, AVX512, NEON, SVE) MUST produce identical output to the scalar reference and the canonical xxHash library for identical inputs.
- **FR-009**: Library MUST accept inputs of any size, including empty (size=0) and very large inputs (up to `SIZE_MAX`).
- **FR-010**: Library MUST be distributed as a compiled C-compatible library (static and/or shared) usable from C, C++, and language bindings via FFI.
- **FR-011**: Library MUST include a header file (xxh3.h) that declares the public API, documents function behaviour, parameters, return types, and provides usage examples.
- **FR-012**: Library MUST be buildable with **Meson** and generate reproducible artifacts (same binary given same toolchain and flags).
- **FR-013**: Library MUST support CPU feature overrides or environment-based control (optional, e.g., `XXH3_FORCE_SCALAR=1`) for testing and debugging.
- **FR-014**: Library MUST support both dynamic (runtime) linking and static linking for all platforms.
- **FR-015**: All wrapper source code (`src/`, `include/`) MUST conform to the **C99 language standard** (`-std=c99`). No C11 or compiler-specific extensions (GNU/Clang) are permitted in wrapper code. The vendored xxHash library is compiled as upstream provides it (also C99-compatible).
- **FR-016**: Library MUST export **legacy and traditional scalar-only** hash functions: `uint32_t xxh32(const void* input, size_t size, uint32_t seed)` (legacy, 32-bit output) and `uint64_t xxh64(const void* input, size_t size, uint64_t seed)` (traditional, 64-bit output). These have **no SIMD variants** and **no variant suffix** in their symbol name. Both functions are always exported on all platforms. `xxh64()` is recommended as a high-performance scalar alternative when SIMD is not available, as it may outperform `xxh3_64_scalar()` on CPUs without SIMD acceleration.
- **FR-017**: Library MUST follow a **four-digit (Numeric Quad) versioning scheme**: `MAJOR.MINOR.PATCH.WRAPPER_PATCH`. The first three digits MUST match the version of the vendored xxHash library (e.g., `0.8.3`). The fourth digit is reserved for the wrapper project's own patches or ABI-compatible fixes (e.g., `0.8.3.0`, `0.8.3.1`).

### Key Entities

- **Hash State** (`xxh3_state_t`): Internal state structure (opaque to users) that holds algorithm state during streaming (incremental) hashing; used with `reset()`, `update()`, `digest()`. One state struct exists per active variant; variant choice is locked at `reset()` time.
- **Variant Symbol**: Each exported named function representing one SIMD or scalar implementation of XXH3 (e.g., `xxh3_64_scalar`, `xxh3_64_sse2`, `xxh3_64_avx2`, `xxh3_64_avx512`, `xxh3_64_neon`). All variant symbols for the same operation produce identical output for identical inputs; they differ only in internal instruction set used.
- **Legacy Function** (`xxh32`): Scalar-only 32-bit hash exported for backwards compatibility with older systems or use cases requiring 32-bit output. No SIMD variants; no variant suffix. Always exported on all platforms.
- **Traditional Function** (`xxh64`): Scalar-only 64-bit hash exported as a mature, well-tested alternative. No SIMD variants; no variant suffix. Always exported on all platforms. On CPUs lacking SIMD support, `xxh64()` may deliver higher throughput than `xxh3_64_scalar()` due to its simpler accumulation loop.
- **Consumer Dispatch**: CPU feature detection and variant selection are the **consumer's responsibility**. Consumers query CPU capabilities (e.g., CPUID + XGETBV on x86) and call the appropriate exported variant function. Required CPU prerequisites per variant are documented in the library header.
- **Seed**: A 64-bit input parameter that modifies the hash output; enables domain separation and hash randomization.
- **Secret**: A 136-192 byte blob used for collision resistance; can be derived from seed via `XXH3_generateSecret()` or provided custom.


## Success Criteria *(mandatory)*

<!--
  ACTION REQUIRED: Define measurable success criteria.
  These must be technology-agnostic and measurable.
-->

### Measurable Outcomes

- **SC-001**: All hash outputs (64-bit and 128-bit) MUST match the canonical xxHash reference implementation bit-for-bit for any input (validated by unit tests and property tests with >1M random inputs).
- **SC-002**: All XXH3 SIMD implementations (SSE2, AVX2, AVX512, NEON, SVE) MUST run on their target CPUs and produce identical outputs to `xxh3_64_scalar()` / `xxh3_128_scalar()`. `xxh32()` and `xxh64()` outputs MUST each match their respective canonical xxHash reference implementations.
- **SC-003**: The library MUST run without crashes, undefined behaviour, or data corruption on all supported platforms (Linux x86_64/aarch64, macOS x86_64/arm64); verified by fuzz tests, AddressSanitizer, and MemorySanitizer runs.
- **SC-004**: Performance improvements MUST be measurable: SIMD implementations must demonstrate ≥2x throughput improvement over scalar on supporting CPUs, or trade-off is documented and justified.
- **SC-005**: ABI/binary compatibility MUST be preserved across patch versions; breaking ABI changes MUST be detected by CI and require explicit versioning.
- **SC-006**: Build artifacts MUST be reproducible: rebuilding with identical toolchain and flags MUST produce identical binary checksums.
- **SC-007**: Integration with cr-xxhash MUST succeed: the Crystal shard MUST link the library via FFI and pass all its tests without modification.
- **SC-008**: Documentation MUST include usage examples (C/C++/FFI), supported CPU targets, performance baselines per platform, and API reference; examples MUST compile and run correctly.

## Clarifications

### Session 2026-02-19

- Q: Should MVP include streaming (state-based) hashing? → A: **Yes, both single-shot and streaming APIs**.
- Q: Which platforms should be supported? → A: **x86/x64 + ARM aarch64** (all x86/x64 SIMD variants always exported; ARM aarch64 variants exported when compiled for aarch64). *(Note: original answer mentioned "runtime dispatch"; superseded by the explicit no-dispatch decision below.)*
- Q: Which build system (CMake vs Makefile vs other)? → A: **Meson** (not CMake or Makefile).
- Q: Should the API expose secret management, or just seed? → A: **Expose both seeds and secrets** (`xxh3_*_withSeed()`, `xxh3_*_withSecret()`, `XXH3_generateSecret()`).
- Q: Should AVX512 support be included in MVP? → A: **Yes, include AVX512 from day 1** (requires emulation/hardware for CI).
- Q: Should header-only mode be supported? → A: **No, library-only mode** (dynamic/static linking, no inlining; simpler).
- Q: Should symbol namespacing (`XXH_NAMESPACE` macro) be supported? → A: **No, fixed prefix** (`xxh3_`) for simplicity.

### Session 2026-02-19 (design decisions)

- Q: Does the library implement internal CPU dispatch (vendor's CPUID/function-pointer mechanism)? → A: **No. The library exports each SIMD variant as a separately named function; consumers implement their own dispatch or selection logic.**
- Q: Language standard for all wrapper source code? → A: **C99 (`-std=c99`)** — no C11 or compiler extensions in `src/` or `include/`.
- Q: How is the wrapper versioned? → A: **Numeric Quad (`MAJOR.MINOR.PATCH.WRAPPER_PATCH`)**. Major/Minor/Patch must match the vendored xxHash version. The fourth digit is for the wrapper itself (e.g., `0.8.3.0`).
- Q: Which hash algorithms are exported, and with what variant structure? → A: **XXH3-64 and XXH3-128 with scalar + SIMD variants** (named `xxh3_64_<variant>`, `xxh3_128_<variant>`); plus **xxh32 (legacy)** and **xxh64 (traditional)** as scalar-only flat functions with no variant suffix. `xxh64()` is recommended as the scalar fallback when no SIMD is available.

## Assumptions

- The xxHash reference implementation (https://github.com/Cyan4973/xxHash) is authoritative for correctness and algorithm details.
- **Platforms**: MVP supports Linux and macOS on x86_64 and aarch64 (ARM64).
  - **x86/x64**: All SIMD variants (scalar, SSE2, AVX2, AVX512) are compiled into every x86/x64 build and exported as distinct symbols. The library does **not** use the vendor's internal dispatcher (`XXH_DISPATCH_FUNCS` / `XXH_featureTest`). Consumers implement CPU detection (CPUID + XGETBV) and call the appropriate variant symbol.
  - **ARM aarch64**: NEON and SVE variant symbols are exported only when the library is built targeting aarch64 (via `meson setup -Dcpu_family=aarch64`). Scalar is always exported. No runtime detection in the library.
- **Language Standard**: All wrapper code (`src/`, `include/`) is C99 (`-std=c99`). No C11 or GCC/Clang extensions.
- **Versioning**: Numeric Quad (`MAJOR.MINOR.PATCH.WRAPPER_PATCH`). First 3 digits match vendor xxHash. The fourth digit is for the wrapper project itself.
- **Build System**: Meson is the primary build tool; no CMake or Makefile.
- **API Scope**: Both **single-shot** (`xxh3_64_<variant>()`, `xxh3_128_<variant>()`) and **streaming** (`xxh3_64_<variant>_reset/update/digest`) APIs are public per XXH3 variant; both seed-based and secret-based variants exposed. Additionally, scalar-only `xxh32()` and `xxh64()` are always exported (no streaming, no SIMD variants).
- **Linking**: Library is distributed as compiled artifacts (static + shared); no header-only mode (users must link against library).
- **Symbols**: XXH3 functions use prefix `xxh3_*` / `XXH3_*` with SIMD variant suffix (`_scalar`, `_sse2`, `_avx2`, `_avx512`, `_neon`, `_sve`). Legacy/traditional functions use plain names `xxh32` and `xxh64` with no suffix. No namespace macro support.
- **Error Handling**: Contract-based (preconditions documented); calling a SIMD variant on a CPU that does not support the required instruction set is undefined behavior. The library header documents required CPU features per variant.
- **Threading**: No mutable shared state; library is reentrant and thread-safe for concurrent read-only hashing.
