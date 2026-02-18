<!--
SYNC IMPACT REPORT
- Version change: template → 1.0.0
- Modified principles:
  - template placeholders → I–V (concrete principles focused on correctness, SIMD safety,
    testing discipline, benchmarking/reproducible builds, and documentation/interop)
- Added sections: Non-functional Requirements; Development Workflow & Quality Gates
- Removed sections: none
- Templates updated:
  - .specify/templates/plan-template.md ✅ updated
  - .specify/templates/spec-template.md ✅ updated
  - .specify/templates/tasks-template.md ✅ updated
  - .specify/templates/checklist-template.md ✅ updated
- Follow-up TODOs: none
-->

# xxhash-wrapper Constitution

## Core Principles

### I. Correctness & API/ABI Stability (NON‑NEGOTIABLE)
All public APIs and exported symbols MUST be correct, deterministic, and accompanied
by tests that validate bit‑for‑bit parity with the xxHash reference implementation
for all inputs. Public API and ABI guarantees are contractually binding: any
change to the public API or ABI MUST follow the Governance versioning rules,
include an explicit migration plan, and pass automated ABI‑regression tests.

Rationale: consumers (including `cr-xxhash`) rely on stable behaviour and
stable binary interfaces; correctness and ABI stability are top priority.

### II. SIMD‑safe Performance (PERFORMANCE PREREQUISITE)
SIMD optimizations MUST provide the same observable results as the scalar
reference and MUST include safe scalar fallbacks and platform runtime dispatch.
All SIMD code paths MUST be memory‑safe (no UB), validated with sanitizers,
and exercised by architecture/vector‑width tests (alignment, unaligned input,
partial vectors, small/large buffers). Performance improvements are accepted
only when correctness and determinism are preserved.

Rationale: high throughput is a goal, but never at the cost of correctness or
portability.

### III. Testing Discipline (TEST‑FIRST, MANDATORY)
Development is test‑first: new functionality MUST be accompanied by failing
tests before implementation. Required test classes:
- Unit tests for core logic (high coverage for hashing internals).
- SIMD/platform unit tests across supported CPU targets.
- Fuzz/property tests for input coverage and edge cases (seed corpus required).
- ABI‑regression and symbol/version tests for public interfaces.
- Integration tests that verify interoperability with `cr-xxhash`.

Acceptance gates (CI) MUST run the above suites; critical failures block merges.

### IV. Continuous Benchmarking & Reproducible Builds (MEASURABLE)
Microbenchmarks for XXH3 (64/128) are required and MUST be included alongside
code changes that affect performance. CI MUST record benchmark baselines and
flag regressions above a small, documented tolerance. Builds and release
artifacts MUST be reproducible (pinned toolchain, deterministic flags) so
performance claims and binary compatibility are verifiable.

Rationale: performance must be measurable and repeatable across runners.

### V. Documentation, Interoperability & Packaging (CLEAR CONTRACTS)
Public APIs, ABI guarantees, release notes, migration guides, and usage
examples (including `cr-xxhash` integration examples) MUST be documented and
kept in sync with releases. Packaging MUST include clear versioned artifacts
and changelogs; deprecations MUST include timelines and automated migration
examples where feasible.

Rationale: clear documentation and packaging reduce downstream risk and
support adoption.

## Non‑functional Requirements

- NFR-001: Implementation MUST produce identical outputs to the canonical
  xxHash reference for all inputs (unit tests and property/fuzz checks).
- NFR-002: SIMD implementations MUST be optional, have scalar fallbacks, and
  be validated across vector widths and CPU features.
- NFR-003: CI MUST run unit, SIMD/platform, fuzz, ABI, and benchmark checks
  (failures block merges for relevant changes).
- NFR-004: Performance regressions MUST be detected by CI; regressions above
  the documented tolerance MUST require remediation or maintainers' approval.
- NFR-005: Public API/ABI changes MUST follow semantic versioning and include
  an explicit migration plan and ABI‑compatibility report.
- NFR-006: Builds MUST be reproducible; CI must pin toolchains and preserve
  build artifacts for release verification.
- NFR-007: Supported platforms: Linux (x86_64, aarch64) and macOS (x86_64,
  arm64). Additional platforms MAY be supported but require CI coverage.

## Development Workflow & Quality Gates

- Every PR MUST include an explicit 
