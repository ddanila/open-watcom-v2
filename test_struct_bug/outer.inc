; outer.inc — Outer macro that INCLUDEs inner.inc and invokes INNER_MACRO
;
; This creates the nested context: outer macro → INCLUDE → inner macro → field access
; which is the pattern that triggers the WASM struct offset bug.

OUTER_MACRO MACRO
    INCLUDE inner.inc
    INNER_MACRO
ENDM
