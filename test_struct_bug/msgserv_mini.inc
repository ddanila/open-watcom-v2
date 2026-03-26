; msgserv_mini.inc — Minimal reproduction of MSGSERV.ASM pattern
;
; Key ingredients:
; 1. Struct definition inside INCLUDEd file
; 2. LABEL + EQU alias indirection ($M_RT2 LABEL, $M_RT EQU $M_RT2)
; 3. Code generation accessing struct via the EQU alias
; 4. Nested macro invocation ($M_BUILD_PTRS / $M_MAKE)

MYSTRUCT STRUC
    F00 DD 0        ; +0x00
    F04 DD 0        ; +0x04
    F08 DD 0        ; +0x08
    F0C DD -1       ; +0x0C
    F10 DD 0        ; +0x10
    F14 DD 0        ; +0x14
    F18 DD -1       ; +0x18
    F1C DD 0        ; +0x1C
    F20 DD 0        ; +0x20
    F24 DD -1       ; +0x24
    F28 DD -1       ; +0x28
    F2C DD NUM DUP(0)  ; +0x2C  (DUP field)
    F30 DD -1       ; +0x30
    F34 DD 0        ; +0x34
    F38 DW 0        ; +0x38
    F3A DB 0        ; +0x3A
    F3B DB 0,0      ; +0x3B
    F3D DB 0        ; +0x3D
    F3E DW 0        ; +0x3E
    F40 DW 0        ; +0x40
MYSTRUCT ENDS

; Key pattern: LABEL + EQU alias (same as $M_RT2 LABEL + $M_RT EQU $M_RT2)
MYRT2 LABEL BYTE
MYRT EQU MYRT2
    PUBLIC MYRT2
    MYSTRUCT <>

; Inner macro that accesses struct (like $M_MAKE)
MAKE_MINI MACRO
    MOV WORD PTR DS:MYRT.F2C, DI       ; expect 0x2C *** KEY ***
ENDM

; Build procedure (like SYSLOADMSG calling $M_BUILD_PTRS)
LOADMSG_MINI PROC NEAR
    ; Direct field accesses (like SYSLOADMSG code)
    MOV WORD PTR DS:MYRT.F00, DI       ; expect 0x00
    MOV WORD PTR DS:MYRT.F10, DI       ; expect 0x10
    MOV WORD PTR DS:MYRT.F1C, DI       ; expect 0x1C
    MOV WORD PTR DS:MYRT.F28, DI       ; expect 0x28
    MOV WORD PTR DS:MYRT.F34, DI       ; expect 0x34

    ; Macro-generated field access (like $M_BUILD_PTRS → $M_MAKE)
    MAKE_MINI

    MOV WORD PTR DS:MYRT.F40, AX       ; expect 0x40
    RET
LOADMSG_MINI ENDP
