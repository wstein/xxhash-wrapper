#!/usr/bin/env bash
# Ensure test sources that rely on POSIX/XSI APIs define a feature-test macro
# (e.g. _XOPEN_SOURCE, _POSIX_C_SOURCE or _DEFAULT_SOURCE) BEFORE any system
# headers are included.

set -euo pipefail
err=0

# gather test C files
found_files=$(find tests -type f -name '*.c')
for f in $found_files; do
  if grep -qE "sigjmp_buf|sigsetjmp|siglongjmp|#\s*include\s*<signal.h>|#\s*include\s*<setjmp.h>" "$f"; then
    first_include_line=$(grep -nE '^#\s*include' "$f" | head -n1 | cut -d: -f1 || true)
    if [ -z "$first_include_line" ]; then
      echo "WARN: $f: contains POSIX setjmp/signal usage but no #include lines; skipping file"
      continue
    fi
    # Check for a feature-test macro defined *before* the first include
    if ! sed -n "1,$((first_include_line-1))p" "$f" | grep -qE '#\s*define\s+(_XOPEN_SOURCE|_POSIX_C_SOURCE|_DEFAULT_SOURCE)'; then
      echo "ERROR: $f: uses POSIX/XSI APIs but does not define a feature-test macro before includes"
      echo "  Add e.g. '#define _XOPEN_SOURCE 700' at the top of the file (before system headers)"
      err=1
    else
      echo "OK: $f has feature-test macro before includes"
    fi
  fi
done

if [ "$err" -ne 0 ]; then
  echo "\nOne or more test files are missing required feature-test macros. Please fix and re-run." >&2
  exit 1
fi

echo "All tested files include a POSIX/XSI feature-test macro where required."