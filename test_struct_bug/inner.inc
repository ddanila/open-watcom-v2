; inner.inc — Included by outer macro to create nested INCLUDE+macro context
;
; This macro accesses MYRT.fields from inside a macro that's defined in
; an INCLUDEd file, invoked from another macro — creating the nesting
; depth that triggers the WASM offset bug.

INNER_MACRO MACRO
    MOV WORD PTR DS:MYRT.F2C, DI       ; expect 0x2C
    MOV WORD PTR DS:MYRT.F34, DI       ; expect 0x34
    MOV WORD PTR DS:MYRT.F40, AX       ; expect 0x40
ENDM
