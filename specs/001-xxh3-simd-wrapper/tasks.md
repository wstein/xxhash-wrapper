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

- [ ] T001 Initialize Meson project structure in `meson.build`
- [ ] T002 [P] Configure `vendor/xxHash` integration in `meson.build`
- [ ] T003 [P] Create public header `include/xxh3.h` with basic types and versioning (C99)
- [ ] T004 Create `src/common/internal_utils.h` for shared macros and SIMD helpers
- [ ] T005 Implement `MAJOR.MINOR.PATCH.WRAPPER_PATCH` versioning logic in `meson.build`
- [ ] T028 Set up GitLab Flow branching and protection rules (FR-018) in repository configuration

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [ ] T006 Create `src/xxh3_wrapper.c` and implement scalar fallback functions (`xxh3_64_scalar`, `xxh3_128_scalar`)
- [ ] T007 [P] Implement legacy `xxh32` and traditional `xxh64` exports in `src/xxh3_wrapper.c`
- [ ] T008 Define opaque `xxh3_state_t` in `include/xxh3.h` and implement `xxh3_createState`/`xxh3_freeState` in `src/xxh3_wrapper.c`
- [ ] T009 Configure CI job matrix for SIMD/platform tests (x86_64, aarch64, macOS) and enforce C99 compliance via `-std=c99` in `.gitlab-ci.yml`

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Language Binding Integration (Priority: P1) 🎯 MVP

**Goal**: Export all XXH3 variants as named symbols for consumer dispatch.

**Independent Test**: Verify a C program linked against the library can call `xxh3_64_scalar()` and `xxh3_64_sse2()` and receive correct results.

### Implementation for User Story 1

- [ ] T010 [P] [US1] Implement SSE2 variant wrapper in `src/variants/x86/sse2.c`
- [ ] T011 [P] [US1] Implement AVX2 variant wrapper in `src/variants/x86/avx2.c`
- [ ] T012 [P] [US1] Implement AVX512 variant wrapper in `src/variants/x86/avx512.c`
- [ ] T013 [P] [US1] Implement NEON variant wrapper in `src/variants/arm/neon.c`
- [ ] T014 [P] [US1] Implement SVE variant wrapper in `src/variants/arm/sve.c`
- [ ] T015 [US1] Implement 64-bit and 128-bit streaming `reset`, `update`, and `digest` for all variants in `src/xxh3_wrapper.c`
- [ ] T016 [US1] Implement secret-based variants (`xxh3_*_withSecret`) and `XXH3_generateSecret` in `src/xxh3_wrapper.c`

---

## Phase 4: User Story 3 - Correctness Validation & Testing (Priority: P1)

**Goal**: Ensure all variants produce identical results to canonical xxHash across target platforms.

**Independent Test**: Run unit tests and fuzz tests; all must pass with 100% correctness against reference implementations.

### Tests for User Story 3

- [ ] T017 [P] [US3] Create unit test suite in `tests/unit/test_variants.c` comparing all variants against scalar
- [ ] T018 [P] [US3] Implement fuzz testing harness in `tests/fuzz/fuzz_xxh3.c` using libFuzzer
- [ ] T019 [US3] Add automated ABI/regression checks script in `scripts/check-abi.sh`
- [ ] T020 [US3] Integrate microbenchmark harness in `tests/bench/bench_variants.c` to verify SC-004

---

## Phase 5: User Story 2 - Library Maintenance & Optimization (Priority: P2)

**Goal**: Enable future optimizations and platform coverage without breaking consumers.

**Independent Test**: Verify that the CPU feature override allows forcing scalar mode even on SIMD-capable hardware.

### Implementation for User Story 2

- [ ] T021 [US2] Implement CPU feature override for testing (`XXH3_FORCE_SCALAR=1`) in `src/xxh3_wrapper.c`
- [ ] T022 [US2] Document required CPU features per variant in `include/xxh3.h` for consumer dispatch guidance

---

## Phase 6: Polish & Cross-cutting Concerns

**Purpose**: Final quality checks, documentation, and compliance.

- [ ] T023 Finalize Inclusive Naming across all files in `src/` and `include/`
- [ ] T024 [P] Verify reproducible build checksums across Linux and macOS environments
- [ ] T025 Update `README.md` with usage examples and FFI integration guide
- [ ] T026 [P] Ensure all commits follow Conventional Commits specification
- [ ] T027 Verify integration with cr-xxhash shard (SC-007)

## Implementation Strategy

1. **MVP First**: Complete Phase 1 & 2, then Phase 3 (US1) starting with SSE2 and AVX2 as they are highly parallelizable.
2. **Incremental Delivery**: Each SIMD variant in Phase 3 is a standalone increment.
3. **Continuous Validation**: Use Phase 4 (US3) tests as each variant is implemented.

## Dependencies

- **US1 Completion Order**: T010-T014 are independent but depend on Phase 2.
- **US3 Completion Order**: T017 depends on at least one SIMD variant from US1.
