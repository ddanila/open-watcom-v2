#!/bin/bash
# test_public_case/run_test.sh -- Test for MASM PUBLIC name canonicalization
#
# In MASM compatibility mode, `PUBLIC ARG_BUF` with a lowercase backing label
# must export `ARG_BUF`, not `arg_buf`.
#
# Usage: ./run_test.sh [path-to-wasm]

set -euo pipefail

WASM="${1:-}"
if [[ -z "$WASM" ]]; then
    for candidate in \
        "bld/wasm/linuxx64/wasm.exe" \
        "build/binbuild/bwasm"; do
        if [[ -x "$candidate" ]]; then WASM="$candidate"; break; fi
    done
fi
if [[ -z "$WASM" || ! -x "$WASM" ]]; then
    echo "ERROR: wasm not found. Usage: $0 <path-to-wasm>"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

OBJXDEF="${OBJXDEF:-$REPO_ROOT/build/binbuild/objxdef}"
if [[ ! -x "$OBJXDEF" ]]; then
    echo "ERROR: objxdef not found at $OBJXDEF"
    exit 1
fi

cd "$SCRIPT_DIR"

echo "=== WASM PUBLIC name case test ==="
echo "Using: $WASM"
echo ""

rm -f repro.obj
"$WASM" -zcm=masm repro.asm -fo=repro.obj

exports="$("$OBJXDEF" repro.obj)"
echo "$exports"

if grep -qx 'ARG_BUF' <<<"$exports" && ! grep -qx 'arg_buf' <<<"$exports"; then
    echo ""
    echo "PASS: exported symbol preserved MASM PUBLIC spelling"
    rm -f repro.obj
    exit 0
fi

echo ""
echo "FAIL: expected exported symbol ARG_BUF, got:"
echo "$exports"
rm -f repro.obj
exit 1
