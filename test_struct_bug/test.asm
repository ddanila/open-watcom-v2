; test_struct_bug/test.asm — Minimal reproducer for WASM struct field offset bug
;
; Pattern: macro → INCLUDE file → struct definition + code generation
; This is exactly how MS-DOS MSGSERV works:
;   MSG_SERVICES macro → INCLUDE MSGSERV.ASM → $M_RES_ADDRS struct + SYSLOADMSG proc
;
; Build: wasm -zcm=masm test.asm -fo=test.obj

CSEG SEGMENT PARA PUBLIC 'CODE'
ASSUME CS:CSEG, DS:CSEG

NUM EQU 1

; === The macro that INCLUDEs the struct+code file ===
; (Equivalent to MSG_SERVICES → INCLUDE MSGSERV.ASM)
MSG_SERVICES_MINI MACRO
    INCLUDE msgserv_mini.inc
ENDM

; === Invoke the macro (this triggers the INCLUDE from macro context) ===
    MSG_SERVICES_MINI

CSEG ENDS
END
