#ifndef XXH3_WRAPPER_INTERNAL_UTILS_H
#define XXH3_WRAPPER_INTERNAL_UTILS_H

#define XXH3_WRAPPER_UNUSED(x) ((void)(x))

/* --------------------------------------------------------------------------
 * Debug-only assertion guard + configurable defensive guards
 *
 * XXH3_DEBUG_ASSERT(expr)
 *   - Behaves like assert(expr) when NDEBUG is not defined; compiles away in
 *     release builds.
 *
 * XXH3_WRAPPER_GUARD(stmt)
 *   - Enable defensive early-return checks when either:
 *       a) the build is a debug build (NDEBUG not defined), OR
 *       b) the compile-time flag `XXH3_WRAPPER_GUARDS` is defined.
 *   - This allows maintainers to force-enable guards in non-debug builds for
 *     testing and CI without depending solely on NDEBUG.
 *
 * Macro semantics:
 *   XXH3_WRAPPER_GUARD( if (ptr == NULL) { return 0; } );
 *
 * In Meson builds, `XXH3_WRAPPER_GUARDS` is defined automatically for
 * `buildtype=debug` via `meson.build` project arguments.
 * -------------------------------------------------------------------------- */
#ifndef NDEBUG
#  include <assert.h>
#  define XXH3_DEBUG_ASSERT(expr) assert(expr)
#else
#  define XXH3_DEBUG_ASSERT(expr) ((void)0)
#endif

/* Guard macro: active when explicitly enabled OR when not building with NDEBUG */
#if defined(XXH3_WRAPPER_GUARDS) || !defined(NDEBUG)
#  define XXH3_WRAPPER_GUARD(stmt) do { stmt; } while (0)
#else
#  define XXH3_WRAPPER_GUARD(stmt) ((void)0)
#endif

#endif
