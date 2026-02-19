#ifndef XXH3_CONVERTERS_H
#define XXH3_CONVERTERS_H

/* Small, internal conversion helpers between the wrapper's `xxh3_128_t`
 * and the vendor `XXH128_hash_t`.
 *
 * These are `static inline` functions (header-only) to avoid repeating the
 * same field assignments in every variant translation unit.
 *
 * IMPORTANT (include-order dependency):
 *   This header depends on the vendor `xxhash.h` type definitions and the
 *   way the vendored header behaves under different compile-time modes
 *   (notably `XXH_INLINE_ALL`). To avoid typedef redefinitions or
 *   incompatible inline/type configurations, always include `xxhash.h`
 *   (or any header that includes it) *before* including `xxh3_converters.h`.
 *
 *   Correct include order (example):
 *     #include "xxh3.h"
 *     #include "xxhash.h"           // vendor types/macros must be visible first
 *     #include "xxh3_converters.h"  // conversion helpers rely on vendor types
 *
 * Rationale:
 *   - `xxhash.h` may expose different inline/type definitions when
 *     `XXH_INLINE_ALL` is defined; mixing different include orders or
 *     compilation modes across translation units can produce warnings
 *     (typedef redefinition) or real type/inlining mismatches.
 *   - Keeping the include order consistent prevents those issues and
 *     makes it safe to build the project both with and without
 *     `XXH_INLINE_ALL`.
 *
 * Note: It is safe to include `xxh3_converters.h` from a TU that already
 * includes `xxhash.h`. Do NOT include the converters header before
 * `xxhash.h`.
 */

#include "xxh3.h"
#include "xxhash.h"

static inline XXH128_hash_t xxh3_to_xxh128(xxh3_128_t src)
{
    XXH128_hash_t out;
    out.high64 = src.high;
    out.low64  = src.low;
    return out;
}

static inline xxh3_128_t xxh128_to_xxh3(XXH128_hash_t src)
{
    xxh3_128_t out;
    out.high = src.high64;
    out.low  = src.low64;
    return out;
}

#endif /* XXH3_CONVERTERS_H */