#!/usr/bin/env sh
set -eu

cat > /tmp/xxh3_wrapper_integration.c <<'EOF'
#include <stdio.h>
#include <string.h>
#include "xxh3.h"

int main(void) {
  const char* text = "cr-xxhash integration";
  uint64_t h = xxh3_64_scalar(text, strlen(text), 0);
  printf("%llu\n", (unsigned long long)h);
  return h == 0 ? 1 : 0;
}
EOF

cc -std=c99 -Iinclude /tmp/xxh3_wrapper_integration.c build/libxxh3_wrapper_static.a -o /tmp/xxh3_wrapper_integration
/tmp/xxh3_wrapper_integration
