#ifndef XXH3_WRAPPER_INTERNAL_UTILS_H
#define XXH3_WRAPPER_INTERNAL_UTILS_H

#define XXH3_WRAPPER_UNUSED(x) ((void)(x))

/* --------------------------------------------------------------------------
 * Debug-only assertion guard
 *
 * XXH3_DEBUG_ASSERT(expr) evaluates `expr` as a runtime assertion in debug
 * builds (NDEBUG not defined) and compiles to nothing in release builds
 * (NDEBUG defined by the build system, e.g. meson buildtype=release).
 *
 * Usage:
 *   XXH3_DEBUG_ASSERT(ptr != NULL);
 *
 * For defensive early-return guards, use #ifndef NDEBUG directly:
 *   #ifndef NDEBUG
 *       if (ptr == NULL) { return 0; }
 *   #endif
 *
 * Both approaches are stripped entirely in release builds (-DNDEBUG).
 * -------------------------------------------------------------------------- */
#ifndef NDEBUG
#  include <assert.h>
#  define XXH3_DEBUG_ASSERT(expr) assert(expr)
#else
#  define XXH3_DEBUG_ASSERT(expr) ((void)0)
#endif

#endif
