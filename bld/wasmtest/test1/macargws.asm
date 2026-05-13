; Test: in MASM/TASM mode, a macro parameter substituted into another macro
; call line is re-tokenised; whitespace between the resulting tokens splits
; them into separate positional arguments. WATCOM-mode behavior (whitespace
; does not split args) is unchanged.
;
; This is the documented MASM 5/MASM 6/TASM behavior. Before this fix, wasm
; -zcm=masm kept the whitespace-bearing substitution as a single positional
; arg, breaking real MASM sources that pass `< X OP Y >`-style expressions
; through wrapper macros -- most notably Microsoft's STRUC.INC `.IF`/
; `.WHILE` chain used across the MS-DOS 4.0 sources.
;
; Setup:
;   outer < AL eq 1 >  -> outer arg = "AL eq 1"
;   inner args         -> substitutes into a fresh line: `inner AL eq 1`
;
; MASM/TASM tokenise that to (AL, eq, 1) and pass them as separate args, so
; `inner`'s `b` parameter is `eq` (non-blank) and the ifb-else branch runs.
; The pre-fix wasm kept "AL eq 1" as a single positional arg, making `b`
; blank and the ifb-then branch run.
;
; Cross-checked against:
;   * Microsoft Macro Assembler 5.10 - emits 0EEh.
;   * Turbo Assembler 4.1            - emits 0EEh.
;
; Wired into makefile with `asm_flags_macargws = -zcm=masm`.
;
.8086
.model small
.code

inner   macro a, b
        ifb     <b>
                DB 0DDh
        else
                DB 0EEh
        endif
endm

outer   macro args
        inner   args
endm

        outer   < AL eq 1 >
        end
