#!/usr/bin/env sh
set -eu

MSG=${1:-${CI_COMMIT_MESSAGE:-}}
if [ -z "$MSG" ]; then
  echo "missing commit message input"
  exit 1
fi

echo "$MSG" | grep -Eq '^(feat|fix|docs|style|refactor|perf|test|build|ci|chore|revert)(\([a-zA-Z0-9._-]+\))?!?: .+'
echo "commit message format ok"
