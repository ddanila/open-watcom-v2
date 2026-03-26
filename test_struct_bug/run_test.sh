#!/bin/bash
# test_struct_bug/run_test.sh — Test for WASM struct field offset bug
#
# Uses preprocessed MS-DOS MSGSERV files that trigger the bug.
# The $M_MAKE macro generates MOV [MYRT.F2C], DI which should resolve
# to offset 0x2C but WASM produces 0x2D (off by 1).
#
# Usage: ./run_test.sh [path-to-wasm]

set -euo pipefail

WASM="${1:-}"
if [[ -z "$WASM" ]]; then
    for candidate in \
        "../build/binbuild/armo64/wasm" \
        "/Users/danila.sukharev/fun/d-msdos.2/watcom/bin/macos-arm64/wasm" \
        "../rel/armo64/wasm"; do
        if [[ -x "$candidate" ]]; then WASM="$candidate"; break; fi
    done
fi
if [[ -z "$WASM" || ! -x "$WASM" ]]; then
    echo "ERROR: wasm not found. Usage: $0 <path-to-wasm>"; exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PPDIR="$SCRIPT_DIR/pp_test"
PPINC=$(ls -d "$PPDIR"/inc_* 2>/dev/null | head -1)

if [[ ! -d "$PPDIR" || -z "$PPINC" ]]; then
    echo "ERROR: pp_test directory not set up. See README."
    exit 1
fi

echo "=== WASM struct field offset bug test ==="
echo "Using: $WASM"
echo ""

cd "$PPDIR"
"$WASM" -zcm=masm "-I$PPINC" TREESYSM.ASM -fo=treesysm_test.obj
rc=$?
if [[ $rc -ne 0 ]]; then echo "FAIL: assembly error ($rc)"; exit 1; fi

python3 - << 'PYEOF'
import sys

data = open('treesysm_test.obj', 'rb').read()
full_code = bytearray()
i = 0
while i < len(data):
    rt = data[i]; rl = data[i+1] | (data[i+2] << 8)
    if rt == 0xA0:
        seg = data[i+3]; off = data[i+4] | (data[i+5] << 8)
        code = data[i+6:i+3+rl-1]
        if seg == 1:
            while off > len(full_code): full_code.append(0)
            full_code[off:off+len(code)] = code
    i += 3 + rl

# Find $M_RT data (pattern: 12 zeros + FFFFFFFF)
rt_off = None
for i in range(len(full_code) - 16):
    if full_code[i:i+12] == b'\x00'*12 and full_code[i+12:i+16] == b'\xff'*4:
        rt_off = i
        break

if rt_off is None:
    print("FAIL: could not find $M_RT data pattern")
    sys.exit(1)

# Find MOV [addr], DI at the $M_CLASS_ADDRS offset (expected RT+0x2C)
FAIL = 0
for i in range(len(full_code) - 4):
    if full_code[i] == 0x89 and full_code[i+1] == 0x3E:
        addr = full_code[i+2] | (full_code[i+3] << 8)
        off = addr - rt_off
        if 0x2A <= off <= 0x2E:
            expected = 0x2C
            if off == expected:
                print(f"  PASS: $M_CLASS_ADDRS at RT+0x{off:02X} (correct)")
            else:
                print(f"  FAIL: $M_CLASS_ADDRS at RT+0x{off:02X} (expected 0x{expected:02X}, delta={off-expected:+d})")
                FAIL += 1

if FAIL > 0:
    print(f"\n*** BUG: struct field offset wrong by {FAIL} reference(s) ***")
    sys.exit(1)
else:
    print("\nAll struct field offsets correct.")
    sys.exit(0)
PYEOF

rc=$?
rm -f treesysm_test.obj
exit $rc
