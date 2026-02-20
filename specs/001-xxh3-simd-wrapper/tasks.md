# Tasks: SIMD-Optimized XXH3 C Library with Multi-Variant Export

**Input**: Design documents from `specs/001-xxh3-simd-wrapper/`
**Prerequisites**: [plan.md](plan.md) (required), [spec.md](spec.md) (required for user stories)

**Tests**: Test tasks are included as correctness and validation are core requirements (User Story 3).

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [x] T001 Initialize Meson project structure in `meson.build`
- [x] T002 [P] Configure `vendor/xxHash` integration in `meson.build`
- [x] T003 [P] Create public header `include/xxh3.h` with basic types and versioning (C99)
- [x] T004 Create `src/common/internal_utils.h` for shared macros and SIMD helpers
- [x] T005 Implement `MAJOR.MINOR.PATCH.WRAPPER_PATCH` versioning logic in `meson.build`
- [x] T028 Set up GitLab Flow branching and protection rules (FR-017) in repository configuration

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [x] T006 Create `src/xxh3_wrapper.c` and implement scalar fallback functions (`xxh3_64_scalar`, `xxh3_128_scalar`)
- [x] T007 [P] Implement legacy `xxh32` and traditional `xxh64` exports in `src/xxh3_wrapper.c`
- [x] T008 Define opaque `xxh3_state_t` in `include/xxh3.h` and implement `xxh3_createState`/`xxh3_freeState` in `src/xxh3_wrapper.c`
- [x] T009 Configure CI job matrix for SIMD/platform tests (x86_64, aarch64, macOS) and enforce C99 compliance via `-std=c99` in `.gitlab-ci.yml`

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Language Binding Integration (Priority: P1) 🎯 MVP

**Goal**: Export all XXH3 variants as named symbols for consumer dispatch.

**Independent Test**: Verify a C program linked against the library can call `xxh3_64_scalar()` and `xxh3_64_sse2()` and receive correct results.

### Implementation for User Story 1

- [x] T010 [P] [US1] Implement SSE2 variant wrapper in `src/variants/x86/sse2.c`
- [x] T011 [P] [US1] Implement AVX2 variant wrapper in `src/variants/x86/avx2.c`
- [x] T012 [P] [US1] Implement AVX512 variant wrapper in `src/variants/x86/avx512.c`
- [x] T013 [P] [US1] Implement NEON variant wrapper in `src/variants/arm/neon.c`
- [x] T014 [P] [US1] Implement SVE variant wrapper in `src/variants/arm/sve.c`
- [x] T015 [US1] Implement 64-bit and 128-bit streaming `reset`, `update`, and `digest` for all variants in `src/xxh3_wrapper.c`
- [x] T016 [US1] Implement secret-based variants (`xxh3_*_withSecret`) and `XXH3_generateSecret` in `src/xxh3_wrapper.c`

---

## Phase 4: User Story 3 - Correctness Validation & Testing (Priority: P1)

**Goal**: Ensure all variants produce identical results to canonical xxHash across target platforms.

**Independent Test**: Run unit tests and fuzz tests; all must pass with 100% correctness against reference implementations.

### Tests for User Story 3

- [x] T017 [P] [US3] Create unit test suite in `tests/unit/test_variants.c` comparing all variants against scalar
- [x] T018 [P] [US3] Implement fuzz testing harness in `tests/fuzz/fuzz_xxh3.c` using libFuzzer
- [x] T019 [US3] Add automated ABI/regression checks script in `scripts/check-abi.sh`
- [x] T020 [US3] Integrate microbenchmark harness in `tests/bench/bench_variants.c` to verify SC-004
- [x] T029 [P] [US3] Add Unity test framework integration in `meson.build` with `meson.add_test_setup()`
- [x] T030 [P] [US3] Migrate `tests/unit/test_variants.c` to use Unity test macros for better assertions
- [x] T031 [US3] Add Unity test configuration file `unity.config` for test grouping and reporting

---

## Phase 5: User Story 2 - Library Maintenance & Optimization (Priority: P2)

**Goal**: Enable future optimizations and platform coverage without breaking consumers.

**Independent Test**: Verify that the CPU feature override allows forcing scalar mode even on SIMD-capable hardware.

### Implementation for User Story 2

- [x] T021 [US2] ~~Implement CPU feature override for testing (`XXH3_FORCE_SCALAR=1`)~~ **(Replaced)** — Removed in favor of unconditional SIMD export with consumer-side dispatch. All variants are now always exported as named public symbols; CPU feature detection is the consumer's responsibility.
- [x] T022 [US2] Document required CPU features per variant in `include/xxh3.h` for consumer dispatch guidance

---

## Phase 6: Polish & Cross-cutting Concerns

**Purpose**: Final quality checks, documentation, and compliance.

- [x] Ensure unit tests expose POSIX/XSI feature macros (add `#define _XOPEN_SOURCE 700` to `tests/unit/test_variants.c`) so `sigjmp_buf` and related helpers are available when compiling with `-std=c99`.
- [x] Add CI lint check to verify test sources declare required feature-test macros when using POSIX/XSI APIs.
- [x] Fix CI artifact permissions: add `chmod +x` to CI jobs that download and execute build artifacts, as `actions/upload-artifact@v4` strips execute permissions.

- [x] Refactor: replace verbose `xxh3_128_t` zero-initializers in `src/variants/*` with
      concise C99 compound-literal returns (`return ((xxh3_128_t){0,0});`) (style-only).

- [x] T023 Finalize Inclusive Naming across all files in `src/` and `include/`
- [x] T024 [P] Verify reproducible build checksums across Linux and macOS environments
- [x] T025 Update `README.md` with usage examples and FFI integration guide
- [x] T026 [P] Ensure all commits follow Conventional Commits specification
- [x] T027 Verify integration with cr-xxhash shard (SC-007)
- [x] T032 Implement platform-specific SIMD variant compilation per FR-005 in `meson.build` and `include/xxh3.h`
  - ✅ Update `meson.build` to conditionally include x86 variants (SSE2, AVX2, AVX512) only on x86-64 builds
  - ✅ Update `meson.build` to conditionally include ARM variants (NEON, SVE) only on aarch64 builds
  - ✅ Add compile-time feature detection macros (`XXH3_HAVE_*`) to header for safe consumer code
  - ✅ Update test files (`test_variants.c`, `bench_variants.c`, `fuzz_xxh3.c`) to guard variant calls with preprocessor conditionals and runtime `TEST_TRY_VARIANT()` signal guards for potentially-unsupported SIMD variants (SVE, AVX2, AVX512)
  - ✅ Verify exported symbols match platform spec (no cross-platform exports)
  - ✅ Update `README.md` with Platform-Specific Variant Availability section and feature detection guidance
  - ✅ **Cross-Platform Verification (Feb 19, 2026)**:
    - macOS arm64: NEON, SVE only (no x86 variants) — 44/44 tests pass ✅
    - Linux x86-64: SSE2, AVX2, AVX512 only (no ARM variants) — 44/44 tests pass ✅
    - Linux aarch64: NEON, SVE only (no x86 variants) — 44/44 tests pass ✅
  - ✅ Create comprehensive verification document at `docs/PLATFORM_VERIFICATION.md`
  - ✅ Update meeting minutes Section 24 with detailed results and symbol tables

## Implementation Strategy

1. **MVP First**: Complete Phase 1 & 2, then Phase 3 (US1) starting with SSE2 and AVX2 as they are highly parallelizable.
2. **Incremental Delivery**: Each SIMD variant in Phase 3 is a standalone increment.
3. **Continuous Validation**: Use Phase 4 (US3) tests as each variant is implemented.

## Dependencies

- **US1 Completion Order**: T010-T014 are independent but depend on Phase 2.
- **US3 Completion Order**: T017 depends on at least one SIMD variant from US1.

---

## Final Status: PROJECT COMPLETE ✅

**All 32 tasks completed** as of February 19, 2026.

### Deliverables Summary

#### Phase 1: Setup (T001-T005) - ✅ Complete

- Meson project structure with vendored xxHash v0.8.3
- Public header API with versioning
- Shared utilities and SIMD helpers

#### Phase 2: Foundations (T006-T009) - ✅ Complete

- Scalar fallback functions (xxh3_64_scalar, xxh3_128_scalar)
- Legacy xxh32/xxh64 exports
- Streaming API infrastructure (createState/freeState)
- CI/CD job matrix configuration

#### Phase 3: User Story 1 - Language Binding (T010-T016) - ✅ Complete

- All 6 SIMD variants implemented and exported (SSE2, AVX2, AVX512, NEON, SVE, Scalar)
- Streaming APIs (reset/update/digest) for all variants
- Secret-based variants with custom secret generation
- Both 64-bit and 128-bit functions

#### Phase 4: User Story 3 - Testing (T017-T031) - ✅ Complete

- 44 comprehensive unit tests (100% pass on all 3 platforms)
- Fuzz testing harness
- Microbenchmark suite
- Unity test framework integration

#### Phase 5: User Story 2 - Maintenance (T021-T022) - ✅ Complete

- ~~CPU feature override mechanism (XXH3_FORCE_SCALAR)~~ **(Removed)** — Replaced with unconditional SIMD variant export on each architecture; consumer performs CPU detection and variant selection.
- CPU feature documentation in header

#### Phase 6: Polish & Cross-Cutting (T023-T032) - ✅ Complete

- Inclusive naming verification
- Reproducible builds (T024) — verified across Linux & macOS
- GitLab Flow adoption (T028) — branch protection configured
- Platform-specific compilation (T032) — verified on 3 architectures
  - macOS arm64 (Darwin)
  - Linux x86-64 (Alpine)
  - Linux aarch64 (Alpine)

### Quality Metrics

✅ **Code Quality**:

- C99 standard compliant
- Zero compiler warnings with `-Wall -Wextra`
- Conventional Commits throughout history

✅ **Test Coverage**:

- 44 unit tests per platform
- All tests pass on all 3 target architectures
- Comprehensive input coverage (short, medium, long, random)
- Streaming vs single-shot validation

✅ **Platform Compliance**:

- macOS arm64: ARM variants only (NEON, SVE)
- Linux x86-64: x86 variants only (SSE2, AVX2, AVX512)
- Linux aarch64: ARM variants only (NEON, SVE)
- Zero cross-platform symbol leakage
- FR-005 100% compliant

✅ **Documentation**:

- README with platform-specific build instructions
- PLATFORM_VERIFICATION.md with detailed symbol export tables
- Meeting minutes with implementation details and rationale
- Comprehensive AsciiDoc meeting minutes (Section 24)
- Inline code documentation and API comments

### Architecture Decisions

🏗️ **No Internal Dispatch**: Library exports all variants as named symbols; consumers implement selection logic

- Enables independent evaluation and selection by language bindings
- Simplifies library code (no CPUID detection)
- Shifts responsibility to consumer (appropriate for C libraries)

🏗️ **Meson Over CMake**: Platform-aware build configuration

- Host-machine detection for conditional compilation
- Cleaner syntax for conditional sources
- Superior caching behavior

🏗️ **Vendored xxHash**: Full version pinning

- Reproducible builds
- No external dependencies at build time
- Easy to track upstream changes

---

## Deployment Readiness

✅ **Feature Complete**: All functional requirements met
✅ **Testing Complete**: All platforms verified
✅ **Documentation Complete**: README, specs, meeting minutes updated
✅ **CI/CD Ready**: Job matrix configured for 3 platforms
✅ **Production Safe**: No runtime dispatch logic, no undefined behavior

**Ready for release to cr-xxhash and downstream consumers.**
