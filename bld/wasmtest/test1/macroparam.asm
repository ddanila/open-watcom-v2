; Test: macro parameter name matching a global symbol must not be expanded
; at macro definition time.  Before the fix, WASM ran ExpandTheConstant()
; on T_MACRO lines, replacing the formal parameter 'num' with the global
; EQU value inside the recorded macro body.
;
.386
.model small
.data

num EQU 1

mystruct struc
    f00 dd 0
    f04 dd 0
    f08 dd num dup(0)
    f0c dd 0
mystruct ends

; Macro takes 'num' as a parameter -- same name as the global EQU.
; Bug: WASM expands 'num' to 1 at MACRO definition time, so
; the DUP count becomes the global value instead of the argument.
build_struct macro num
    mystruct2 struc
        g00 dd 0
        g04 dd 0
        g08 dd num dup(0)
        g0c dd 0
    mystruct2 ends

    myst label byte
    public myst
    mystruct2 <>

    mov word ptr myst.g0c, ax
endm

build_struct 3    ; g08 should be 3 dwords = 12 bytes, g0c at offset 0x14
                  ; Bug: g08 is 1 dword = 4 bytes, g0c at offset 0x0C

.code
end
