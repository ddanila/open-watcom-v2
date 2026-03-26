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

## Debug Output Meaning

There are two separate debug concepts for `wasm`:

1. Assembler output debug information:
   `-d1` enables line number debugging support in assembled output.
2. Assembler internal trace/debug output:
   `-d6` is intended for internal debug output, but it only works if the
   assembler binary was compiled with `DEBUG_OUT`.

The `DEBUG_OUT` define is enabled by building `wasm` with `debug=2`, as wired
in `bld/wasm/master.mif`.

## Next Steps

To build the Linux x64 host `wasm` target with internal debug output enabled:

```bash
cd /home/ddanila/fun/open-watcom-v2
. ./setvars.sh
cd bld/wasm/linuxx64
"$OWROOT/build/$OWOBJDIR/wmake" -f makefile clean debug=2
"$OWROOT/build/$OWOBJDIR/wmake" -f makefile debug=2
```

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

## Goal

The immediate goal is to move from the bootstrap assembler (`bwasm`) to a
host-built `wasm` binary for Linux x64 that has internal debug output enabled,
so `-d6` can be used when debugging assembler behavior locally.

The longer-term goal, if needed, is a full non-bootstrap release-style build
that produces final host artifacts under `rel/`.
