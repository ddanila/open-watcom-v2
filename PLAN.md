# Plan: Split upstream PR #1369 "Update bundled libraries"

Reference: https://github.com/open-watcom/open-watcom-v2/pull/1369

## Context

PR #1369 by @mal359 updates all bundled third-party libraries in one large PR (406 files).
The goal is to address security vulnerabilities (CVEs) in outdated bundled copies.
We will re-create this work as separate, reviewable PRs -- one library at a time.
We will NOT copy-paste from #1369; instead, we'll manually apply each upgrade
using upstream sources and CVE patches as references.

## Current bundled library versions

| Library  | Current version | Location           |
|----------|----------------|--------------------|
| bzip2    | 1.0.3          | `contrib/bzip2/`   |
| zlib     | 1.2.3          | `contrib/zlib/`    |
| TinyXML  | 2.5.3          | `contrib/tinyxml/` |
| libzip   | ~0.7 (2005)    | `contrib/libzip/`  |

## Proposed PRs (in order of priority / independence)

### PR 1: Update bundled TinyXML to 2.6.2 + CVE patches
- **Scope**: `contrib/tinyxml/` (111 files, mostly docs)
- **Target version**: 2.6.2
- **CVEs addressed**: CVE-2021-42260, CVE-2023-34194
- **What to do**:
  1. Replace TinyXML source with 2.6.2 release from upstream
  2. Apply post-discontinuation CVE patches (Debian/SF)
  3. Apply FileZilla and select SourceForge community patches
  4. Ensure C++ headers are correct for OW build
- **References from #1369 commits**:
  - `33585ab1` Update bundled TinyXML to 2.6.2
  - `678df23b` Added FileZilla and select SF patches
  - `fffd7299` Fix bootstrap with newer zlib (C++ headers in TinyXML)
- **Why first**: Smallest scope of code changes, mostly a drop-in replacement + patches.

### PR 2: Update bundled bzip2 to 1.0.8 + distro patches
- **Scope**: `contrib/bzip2/` (44 files)
- **Current version**: 1.0.3 (2005)
- **Target version**: 1.0.8
- **CVEs addressed**: CAN-2005-0953, CAN-2005-0758, CERT-FI 20469,
  CVE-2010-0405, CVE-2016-3189, CVE-2019-12900
- **What to do**:
  1. Merge bzip2 1.0.8 source
  2. Apply upstream hardening patches
  3. Apply Debian/Gentoo/GitLab bzip2 distro patches
  4. Replace sprintf in bzip2recover
- **References from #1369 commits**:
  - `e611fe9a` Merge bzip2-1.0.8
  - `8747064f` Add upstream patches
  - `2cbddad0` Hardened bzip2
  - `416785d1` bzip2 tweaks
- **Why second**: Self-contained library, no downstream build system changes needed.

### PR 3: Update bundled zlib to latest + CVE patches
- **Scope**: `contrib/zlib/` (248 files) + `bld/setupgui/zlib/master.mif` (1 file)
- **Current version**: 1.2.3 (2005)
- **Target version**: zlib-master (as of ~Jan 2025)
- **CVEs addressed**: CVE-2016-9840, CVE-2016-9841, CVE-2016-9842,
  CVE-2016-9843, CVE-2018-25032, CVE-2022-37434, CVE-2023-45853
- **What to do**:
  1. Update zlib source to latest stable release
  2. Apply select SUSE patches
  3. Update `bld/setupgui/zlib/master.mif` for new source files
  4. Verify bootstrap still works
- **References from #1369 commits**:
  - `4c20f4df` Update to zlib-master
  - `fffd7299` Fix bootstrap with newer zlib
  - `6458ef88` Let's try that again
- **Why third**: Largest file count; touches build system; may need bootstrap verification.

### PR 4: Backport CVE patches to bundled libzip
- **Scope**: `contrib/libzip/lib/` (2 files)
- **CVEs addressed**: CVE-2011-0421, CVE-2015-2331
- **What to do**:
  1. Backport CVE-2011-0421 fix (null pointer deref in `zip_name_locate`)
  2. Backport CVE-2015-2331 fix (integer overflow)
  3. Replace deprecated `stricmp` with `strcasecmp`
- **References from #1369 commits**:
  - `d8d80586` Backport CVE patches
- **Why last**: Only 2 files, minimal risk, quick to review.
  Could also be done first if we want a quick win.

## Notes

- Each PR description will reference upstream PR #1369 for context
- We will manually apply changes using upstream sources, not copy from #1369
- Build verification should be done for each PR where feasible
- The order above can be adjusted -- PR 4 (libzip) could be done first as a quick win
