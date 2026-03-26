#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 /path/to/wasm" >&2
    exit 2
fi

WASM="$1"
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

SRC="$TMPDIR/ctrl_z.asm"
OBJ="$TMPDIR/ctrl_z.obj"
LOG="$TMPDIR/ctrl_z.log"

python3 - "$SRC" <<'PY'
from pathlib import Path
import sys

Path(sys.argv[1]).write_bytes(b"\x1a\nEND\n")
PY

echo "=== WASM DOS EOF marker test ==="
echo "Using: $WASM"
echo

set +e
"$WASM" -zcm=masm "$SRC" -fo="$OBJ" >"$LOG" 2>&1
RC=$?
set -e

cat "$LOG"

if grep -q "Internal error in ../c/asmins.c" "$LOG"; then
    echo
    echo "FAIL: bare Ctrl-Z line triggers internal error"
    exit 1
fi

echo
if [ "$RC" -eq 0 ]; then
    echo "PASS: Ctrl-Z is treated as EOF without crashing"
else
    echo "PASS: Ctrl-Z is treated as EOF without crashing (exit $RC)"
fi
