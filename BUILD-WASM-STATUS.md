# Local `wasm` Build Status

This note records the current local build state for the Open Watcom `wasm`
project in this repository.

## Important Naming Note

In this repository, `wasm` means the Open Watcom assembler project under
`bld/wasm`. It is not a WebAssembly backend.

Relevant sources:

- `bld/wasm`
- `bld/wasm/doc/cmdlopts.txt`
- `bld/wasm/master.mif`
- `setvars.sh`
- `build.sh`

## What Was Built

The local bootstrap build was completed with:

```bash
. ./setvars.sh
./build.sh boot
```

That produced bootstrap host tools in `build/binbuild/`.

Relevant artifacts observed after the build:

- `build/binbuild/bwasm`
- `bld/wasm/binbuild/bwasm.exe`
- `bld/wasm/binbuild/bwasm.sym`
- `build/binbuild/wmake`
- `build/binbuild/builder`

The current host here is Linux x86_64, and the bootstrap `bwasm` artifact is a
Linux x86_64 ELF executable.

## Current State

At this point, the repository has a working bootstrap assembler:

```bash
build/binbuild/bwasm
```

This is a bootstrap tool, not a final `rel/` installation artifact.

At the time this note was written:

- the `boot` build completed
- there was no `rel/` directory yet
- there was no final packaged host `wasm` under a `rel/bin*` directory
- `master` contains this note so the local build state is recorded in-tree

## Debug Output Meaning

There are two separate debug concepts for `wasm`:

1. Assembler output debug information:
   `-d1` enables line number debugging support in assembled output.
2. Assembler internal trace/debug output:
   `-d6` is intended for internal debug output, but it only works if the
   assembler binary was compiled with `DEBUG_OUT`.

The `DEBUG_OUT` define is enabled by building `wasm` with `debug=2`, as wired
in `bld/wasm/master.mif`.

## What Failed Before `boot`

An attempt to build the host `linuxx64` `wasm` target before the full bootstrap
stage failed because required build libraries were not available yet.

The relevant failure was:

```text
/usr/bin/ld: cannot find ../../../bld/watcom/linuxx64/clibext.lib: No such file or directory
```

This is why `./build.sh boot` is a prerequisite before trying to build
`bld/wasm/linuxx64` directly.

## Next Steps

To build the Linux x64 host `wasm` target with internal debug output enabled:

```bash
cd /home/ddanila/fun/open-watcom-v2
. ./setvars.sh
cd bld/wasm/linuxx64
"$OWROOT/build/$OWOBJDIR/wmake" -f makefile clean debug=2
"$OWROOT/build/$OWOBJDIR/wmake" -f makefile debug=2
```

This command should produce a non-bootstrap host build of `wasm` for Linux x64.
The exact final artifact path still needs to be confirmed after running the
commands above.

After that build, verify:

- the output binary location under `bld/wasm/linuxx64` or its generated target
  path
- whether `-d6` now produces internal debug output
- whether `-d1` still emits line number debug information as expected

Suggested smoke tests:

```bash
build/binbuild/bwasm -?
```

and then, after the `debug=2` target build:

```bash
path/to/wasm -d6 sample.asm
path/to/wasm -d1 sample.asm
```

Minimal smoke test source:

```asm
.model small
.code
start:
    mov ax, 4c00h
    int 21h
end start
```

Use it to confirm:

- `-d1` completes successfully and emits object output with line debug support
- `-d6` actually prints internal assembler diagnostics on the debug-enabled host
  build

## Expected Build Shapes

There are three relevant build states for this work:

1. `preboot`
   Produces the minimum build tools needed to continue bootstrapping.
2. `boot`
   Produces bootstrap host tools in `build/binbuild/`, including `bwasm`.
3. host `wasm` with `debug=2`
   Intended to produce a host-built Linux x64 `wasm` binary with `DEBUG_OUT`
   enabled so `-d6` can be exercised.

If a full release-style tree is needed later, expect that to come from a later
top-level builder flow rather than from `boot` alone.

## Known Unknowns

The following items still need direct confirmation:

- the exact output path of the Linux x64 non-bootstrap `debug=2` `wasm` binary
- whether `-d6` emits useful internal trace output on that binary
- whether the final host-built binary is named `wasm`, `wasm.exe`, or copied to
  another generated path
- whether a later `builder build` or `builder rel` step is desired for this
  work, or whether the host `debug=2` binary alone is sufficient

## Goal

The immediate goal is to move from the bootstrap assembler (`bwasm`) to a
host-built `wasm` binary for Linux x64 that has internal debug output enabled,
so `-d6` can be used when debugging assembler behavior locally.

The longer-term goal, if needed, is a full non-bootstrap release-style build
that produces final host artifacts under `rel/`.

## Confirmed MSGSERV Macro Bug

The original `MSGSERV` struct-offset failure in the `~/fun/msdos` tree is now
confirmed and fixed in the compiler.

Observed failure before the fix:

- `test_struct_bug/run_test.sh` reproduced `$M_RT.$M_CLASS_ADDRS` as `RT+0x2D`
  instead of the correct `RT+0x2C`
- the same bad code generation appeared in the real
  `MS-DOS/v4.0/src/CMD/TREE/TREESYSM.ASM` path when assembled through the
  `MSGSERV` macro stack

Root cause:

- the bug was not in struct field lookup itself
- `wasm` was expanding constants on `MACRO` definition lines before recording
  formal parameter names
- in `SYSMSG.INC`, the formal parameter `nummsg` in
  `$M_BUILD_PTRS Macro nummsg` could therefore be rewritten to `0` because the
  global symbol `NUMmsg` already existed
- that corrupted stored macro lines such as `$M_INDEX = 0`, which later
  expanded using the invocation argument and shifted `$M_RT.$M_CLASS_ADDRS`
  references by `+1`

Compiler fix:

- [bld/wasm/c/breakout.c](/home/ddanila/fun/open-watcom-v2/bld/wasm/c/breakout.c)
  now treats `T_MACRO` the same as `T_NAME` in `directive()`
- this preserves formal parameter names and prevents premature constant
  expansion on macro definition lines

Verification:

- the committed `test_struct_bug` reproducer now passes
- `MS-DOS/v4.0/src/CMD/TREE/TREESYSM.ASM` assembles successfully against the
  fixed host `wasm`

Separate note:

- `MS-DOS/v4.0/src/CMD/DEBUG/DEBUG.ASM` currently hits a different `wasm`
  internal assertion in `asmins.c(2626)`; that is distinct from the fixed
  `MSGSERV` offset bug
