comment $
cmacros - assembly macros for interfacing to HHLs
(C)Copyright 1984, 1988 Microsoft Corporation
$
if1
outif MACRO name,defval,onmsg,offmsg
    ifndef name
	ifb <defval>
	    name=0
	else
	    name=defval
	endif
    endif
    if name
	ifnb <onmsg>
	    ;%out ! onmsg
	endif
    else
	ifnb <offmsg>
	    ;%out ! offmsg
	endif
    endif
endm

error MACRO msg
bug
;%out E r r o r ----- msg
ENDM
;%out cMacros Version 1.04
outif memS,0,<Small Model>
outif memM,0,<Medium Model>
outif memL,0,<Large Model>
outif memC,0,<Compact Model>
outif memH,0,<Huge Model>
memMOD= memS + memM + memL + memC + memH
if memMOD ne 1
if memMOD eq 0
memS= 1
outif memS,0,<Small Model>
else
error <Must have only 1 memory model selected>
endif
endif
sizeC= memM + memL + memH
sizeD= memL + memC + (memH*2)
outif ?DF,0,<No segments or groups will be defined>
outif ?WIN,0,<Windows Support>
outif ?PLM,0,<PLM calling convention>
endif
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
?RSL = 0
?CPD = 0
?ArgL = 0
?ArgC = 0
?BA = 0
?ACB = 0
??? = 0
?PO = 0
?PAS = 0
?PC = 0
?IA = 0
?PU = 0
?ADJ = 0
?lblpu = 0
?N = 0000000000000000B
?AX = 0000000000000011B
?AH = 0000000000000001B
?AL = 0000000000000010B
?BX = 0000000000001100B
?BH = 0000000000000100B
?BL = 0000000000001000B
?CX = 0000000000110000B
?CH = 0000000000010000B
?CL = 0000000000100000B
?DX = 0000000011000000B
?DH = 0000000001000000B
?DL = 0000000010000000B
?SI = 0000000100000000B
?DI = 0000001000000000B
?ES = 0000010000000000B
?DS = 0000100000000000B
?BP = 0001000000000000B
?SP = 0010000000000000B
?SS = 0100000000000000B
?CS = 1000000000000000B
uconcat macro n1,n2,o1,o2,p1,p2
n1&n2 o1&o2 p1&p2
endm
mpush macro rV
irp x,<ax,bx,cx,dx,si,di,es,ds,bp,sp,ss,cs>
if rV AND ?&x
push x
endif
endm
endm
mpop macro rV
irp x,<cs,ss,sp,bp,ds,es,di,si,dx,cx,bx,ax>
if rV AND ?&x
pop x
endif
endm
endm
SAVE macro rL
?RSL = 0
?RI ?RSL,<rL>
endm
smashes macro n,rL
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
?SM&n = 0
?RI ?SM&n,<rL>
endm
?RI macro n,rL
irp x,<rL>
ifdef ?&x
n = n or ?&x
endif
endm
endm
parmB macro nL
?pp <&nL>,<byte>,2,1
endm
parmW macro nL
?pp <&nL>,<word>,2,2
endm
parmD macro nL
ife ?PLM
irp x,<nL>
?pp <&x>,<DWORD>,0,4
?pp <Off_&x>,<WORD>,2,2
?pp <Seg_&x>,<WORD>,2,2
endm
else
irp x,<nL>
?pp <Seg_&x>,<WORD>,2,2
?pp <Off_&x>,<WORD>,2,2
?pp <&x>,<DWORD>,0,4
endm
endif
endm
parmQ macro nL
?pp <&nL>,<QWORD>,8,8
endm
parmT macro nL
?pp <&nL>,<TBYTE>,10,10
endm
if sizeC
parmCP macro nl
parmD <nl>
endm
else
parmCP macro nl
parmW <nl>
endm
endif
if sizeD
parmDP macro nl
parmD <nl>
endm
else
parmDP macro nl
parmW <nl>
endm
endif
?pp macro nL,t,l,s
if ?CPD
; (listing directive removed for WASM)
ife ?PLM
irp x,<nL>
?pp1 x,<t>,%?PO,%?adj,%(?PO+?adj)
?PO = ?PO + l
; (listing directive removed for WASM)
?T&x = s
endm
else
irp x,<nL>
?PO = ?PO + l
?pp1 x,<t>,%?PO,%?adj,%(?PO+?adj)
; (listing directive removed for WASM)
?T&x = s
endm
endif
; (listing directive removed for WASM)
else
;%out Parm(s) "&nl" declared outside proc def.
endif
endm
?pp1 macro n,t,o,a,b
ife ?PLM
n equ t ptr [bp+b]
else
n equ t ptr [bp+a+?PO-o]
endif
endm
localB macro nL
?aLoc <&nL>,<BYTE ptr>,1,1,0
endm
localW macro nL
?aLoc <&nL>,<WORD PTR>,2,2,1
endm
localD macro nL
irp x,<nL>
?aLoc <Seg_&x>,<WORD PTR>,2,2,1
?aLoc <Off_&x>,<WORD PTR>,2,2,1
?aLoc <&x>,<DWORD PTR>,0,4,1
endm
endm
localQ macro nL
?aLoc <&nL>,<QWORD PTR>,8,8,1
endm
localT macro nL
?aLoc <&nL>,<TBYTE PTR>,10,10,1
endm
if sizeC
localCP macro nL
localD <nL>
endm
else
localCP macro nL
localW <nL>
endm
endif
if sizeD
localDP macro nL
localD <nL>
endm
else
localDP macro nL
localW <nL>
endm
endif
localV macro n,a
?aLoc <&n>,,%(&a),0,1
endm
?aLoc macro nL,t,l,s,a
if ?CPD
; (listing directive removed for WASM)
??? = ??? + l
if a
??? = ((??? + 1) AND 0FFFEH)
endif
irp x,<nL>
?aL1 x,<t>,%???
; (listing directive removed for WASM)
?T&x = s
endm
; (listing directive removed for WASM)
else
;%out Locals "&nl" declared outside procedure def.
endif
endm
?aL1 macro n,t,o
if ?IA
n equ t [bp-?IA-o]
else
n equ t [bp-o]
endif
endm
globalB macro n,i,s
?aD <n>,1
?dd n,1,<BYTE>,<DB>,<i>,<s>
endm
globalW macro n,i,s
?aD <n>,2
?dd n,1,<WORD>,<DW>,<i>,<s>
endm
globalD macro n,i,s
?aD <n>,4
?dd n,1,<DWORD>,<DD>,<i>,<s>
endm
globalQ macro n,i,s
?aD <n>,8
?dd n,1,<QWORD>,<DQ>,<i>,<s>
endm
globalT macro n,i,s
?aD <n>,10
?dd n,1,<TBYTE>,<DT>,<i>,<s>
endm
if sizeC
globalCP macro n,i,s
globalD n,<i>,<s>
endm
else
globalCP macro n,i,s
globalW n,<i>,<s>
endm
endif
if sizeD
globalDP macro n,i,s
globalD n,<i>,<s>
endm
else
globalDP macro n,i,s
globalW n,<i>,<s>
endm
endif
staticB macro n,i,s
?aD <n>,1
?dd n,0,<BYTE>,<DB>,<i>,<s>
endm
staticW macro n,i,s
?aD <n>,2
?dd n,0,<WORD>,<DW>,<i>,<s>
endm
staticD macro n,i,s
?aD <n>,4
?dd n,0,<DWORD>,<DD>,<i>,<s>
endm
staticQ macro n,i,s
?aD <n>,8
?dd n,0,<QWORD>,<DQ>,<i>,<s>
endm
staticT macro n,i,s
?aD <n>,10
?dd n,0,<TBYTE>,<DT>,<i>,<s>
endm
if sizeC
staticCP macro n,i,s
staticD n,<i>,<s>
endm
else
staticCP macro n,i,s
staticW n,<i>,<s>
endm
endif
if sizeD
staticDP macro n,i,s
staticD n,<i>,<s>
endm
else
staticDP macro n,i,s
staticW n,<i>,<s>
endm
endif
?dd macro n,p,t,d,i,s
ife ?PLM
n label t
?dd1 _&n,p,<d>,<i>,<s>
else
?dd1 n,p,<d>,<i>,<s>
endif
endm
?dd1 macro n,p,d,i,s
if p
PUBLIC n
endif
ifb <s>
n d i
else
ifb <i>
n d s DUP (?)
else
n d s DUP (i)
endif
endif
endm
externB macro nL
?ex1 <&nL>,1,<BYTE>
endm
externW macro nL
?ex1 <&nL>,2,<WORD>
endm
externD macro nL
?ex1 <&nL>,4,<DWORD>
endm
externQ macro nL
?ex1 <&nL>,8,<QWORD>
endm
externT macro nL
?ex1 <&nL>,10,<TBYTE>
endm
externNP macro nL
?ex1 <&nL>,2,<NEAR>
endm
externFP macro nL
?ex1 <&nL>,4,<FAR>
endm
if sizeC
externP macro nL
?ex1 <&nL>,4,<FAR>
endm
else
externP macro nL
?ex1 <&nL>,2,<NEAR>
endm
endif
if sizeC
externCP macro nL
?ex1 <&nL>,4,<DWORD>
endm
else
externCP macro nL
?ex1 <&nL>,2,<WORD>
endm
endif
if sizeD
externDP macro nL
?ex1 <&nL>,4,<DWORD>
endm
else
externDP macro nL
?ex1 <&nL>,2,<WORD>
endm
endif
?ex1 macro nL,s,d
irp x,<nL>
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
?T&x = s
ife ?PLM
extrn _&x:&d
x equ _&x
else
extrn x:&d
endif
endm
endm
labelB macro nL
?lb1 <&nL>,1,<BYTE>
endm
labelW macro nL
?lb1 <&nL>,2,<WORD>
endm
labelD macro nL
?lb1 <&nL>,4,<DWORD>
endm
labelQ macro nL
?lb1 <&nL>,8,<QWORD>
endm
labelT macro nL
?lb1 <&nL>,10,<TBYTE>
endm
labelNP macro nL
?lb1 <&nL>,2,<NEAR>
endm
labelFP macro nL
?lb1 <&nL>,4,<FAR>
endm
if sizeC
labelP macro nL
?lb1 <&nL>,4,<FAR>
endm
else
labelP macro nL
?lb1 <&nL>,2,<NEAR>
endm
endif
if sizeC
labelCP macro nL
?lb1 <&nL>,4,<DWORD>
endm
else
labelCP macro nL
?lb1 <&nL>,2,<WORD>
endm
endif
if sizeD
labelDP macro nL
?lb1 <&nL>,4,<DWORD>
endm
else
labelDP macro nL
?lb1 <&nL>,2,<WORD>
endm
endif
?lb1 macro nL,s,d
?lblpu = 0
irp x,<nL>
ifidn <x>,<PUBLIC>
?lblpu = 1
else
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
?T&x = s
ife ?PLM
if ?lblpu
public _&x
endif
_&x label &d
x equ _&x
else
if ?lblpu
public x
endif
x label &d
endif
endif
endm
endm
defB macro nL
?aD <&nL>,1
endm
defW macro nL
?aD <&nL>,2
endm
defD macro nL
?aD <&nL>,4
endm
defQ macro nL
?aD <&nL>,8
endm
defT macro nL
?aD <&nL>,10
endm
if sizeC
defCP macro nL
defD <nL>
endm
else
defCP macro nL
defW <nL>
endm
endif
if sizeD
defDP macro nL
defD <nL>
endm
else
defDP macro nL
defW <nL>
endm
endif
?aD macro nL,s
irp x,<nL>
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
?T&x = s
endm
endm
regPtr macro n,S,O
; (listing directive removed for WASM)
; (listing directive removed for WASM)
; (listing directive removed for WASM)
?T&n = 0FFFFH
?SR&n = 0
?RI ?SR&n,<&S>
?OR&n = 0
?RI ?OR&n,<&O>
endm
; WASM: ?Atal is not used (ccall inlines args via IRP); Arg is defined with ccall
?Atal macro n,i
endm
?AP macro n
?argl = ?argl + 2
ifdef ?T&n
ife ?T&n-1
push word ptr (n)
exitm
endif
ife ?T&n-2
push n
exitm
endif
ife ?T&n-4
push word ptr (n)+2
push word ptr (n)
?argl = ?argl + 2
exitm
endif
ife ?T&n-8
push word ptr (n)+6
push word ptr (n)+4
push word ptr (n)+2
push word ptr (n)
?argl = ?argl + 6
exitm
endif
ife ?T&n-0FFFFH
mpush %(?SR&n),1
mpush %(?OR&n),1
?argl = ?argl + 2
exitm
endif
ife ?T&n
push word ptr (n)
exitm
endif
endif
push n
endm
; WASM: simplified ccall — pushes args left-to-right via IRP.
; For C cdecl calling convention with multiple args, list them right-to-left.
; For single-arg calls this is always correct.
Arg macro aL
; WASM: Arg is a no-op; args are passed directly to ccall
endm
ccall macro n,a,sleaze
?argl = 0
ifnb <a>
irp x,<a>
?AP x
endm
endif
ifdef ?SM&n
?RSL = ?RSL AND ?SM&n
endif
mpush %?RSL
ife ?PLM
ifb <sleaze>
call _&n
else
call n
endif
else
call n
endif
if ?argl
add sp,?argl
endif
mpop %?RSL
?RSL = 0
?argc = 0
?argl = 0
endm
cProc macro n,cl,s
?pd n,<cl>,<s>,4
endm
?pd macro n,c,a,i
if ?CPD
?UTPE
endif
?CPD = 1
??? = 0
?argc = 0
?BA = 0
?PO = 0
?PU = 0
?IA = 0
?adj = i
?PAS = 0
ifnb <a>
?RI ?PAS,<a>
endif
?PC = sizeC
irp x,<c>
ifidn <x>,<FAR>
?PC = 1
endif
ifidn <x>,<NEAR>
?PC = 0
endif
ifidn <x>,<PUBLIC>
?PU = 1
endif
endm
if ?PC
if ?WIN
?IA = 2
endif
?adj = ?adj + 2
endif
ife ?PLM
ife ?PC
n label near
else
n label far
endif
?pg <_&n>,%?PU,%?PC,%?PAS
else
?pg <n>,%?PU,%?PC,%?PAS
endif
endm
; WASM: ?pg stores proc attributes; cBegin/cEnd are static macros
; that take the proc name as the first argument (injected by preprocess-wasm).
__wasm_proc_c = 0  ; is FAR?
__wasm_proc_p = 0  ; is PUBLIC?
__wasm_proc_a = 0  ; save regs mask
?pg macro n,p,cc,aa
    __wasm_proc_c = cc
    __wasm_proc_p = p
    __wasm_proc_a = aa
endm
?UTPE macro
endm
; WASM: ?pg1 is a no-op; cEnd is a static macro defined below
?pg1 macro n,cc,aa,oo
endm
; WASM static cBegin: first arg is the proc name (injected by preprocess-wasm)
cBegin macro procname,g
    ?CPD = 0
    ?argc = 0
    ?BA = 1
    ??? = (???+1) AND 0FFFEH
    IF __wasm_proc_p
	PUBLIC &procname
    ENDIF
    IFIDN <g>,<nogen>
    ELSE
	IF __wasm_proc_c
	    &procname proc FAR
	ELSE
	    &procname proc NEAR
	ENDIF
	IF ?IA
	    mov ax,ds
	    nop
	    inc bp
	    push bp
	    mov bp,sp
	    push ds
	    mov ds,ax
	ELSE
	    push bp
	    mov bp,sp
	ENDIF
	IF ???
	    sub sp,???
	ENDIF
	mPush %__wasm_proc_a,1
    ENDIF
endm
; WASM static cEnd: first arg is the proc name (injected by preprocess-wasm)
cEnd macro procname,g
    ?BA = 0
    IFIDN <g>,<nogen>
    ELSE
	mPop %__wasm_proc_a,1
	IF ?IA
	    sub bp,2
	    mov sp,bp
	    pop ds
	    pop bp
	    dec bp
	ELSE
	    mov sp,bp
	    pop bp
	ENDIF
	ife ?PLM
	    ret
	else
	    ret ?PO
	endif
    ENDIF
    &procname endp
endm

assumes macro s,g
local assumed
assumed = 0
ifidn <code>,<g>
?cas <s>
assumed = 1
endif
ifidn <CODE>,<g>
?cas <s>
assumed = 1
endif
ifidn <data>,<g>
assume s&:dgroup
assumed = 1
endif
ifidn <DATA>,<g>
assume s&:dgroup
assumed = 1
endif
ife assumed
assume s&:&g
endif
endm
if sizeC
?cas macro s
assume s&:_TEXT
endm
else
?cas macro s
assume s&:IGROUP
endm
endif
createSeg macro n,ln,a,co,cl,grp
ifnb <grp>
ifidn <grp>,<IGROUP>
ife sizeC
addSeg IGROUP,n
endif
else
addSeg grp,n
endif
endif
ifnb <cl>
n segment a co '&cl'
else
n segment a co
endif
n ends
?cs1 <n>,<ln>
endm
if1
ASMpass=1
else
ASMpass=2
endif
; WASM: addSeg — emit GROUP directive immediately when a segment is provided.
; When called with only grp (no seg), it is a no-op (GROUP already emitted).
addSeg macro grp,seg
ifnb <seg>
grp GROUP seg
endif
endm
defGrp macro nam
addSeg nam
endm
; WASM: ?cs1 defines beginXX macro using nested MACRO (without & prefix).
; Each call gets a unique segment-tracking index via IRP+%operator.
; ?cs2 is a no-op; sEnd is defined once as a static macro below.
__wasm_cms_seg = 0   ; currently open segment index (0=none)
__wasm_cms_cnt = 0   ; counter for segment index assignment
?cs1 macro n,ln
    __wasm_cms_cnt = __wasm_cms_cnt + 1
    irp __idx, <%__wasm_cms_cnt>
	begin&ln macro
	    __wasm_cms_seg = __idx
	    &n segment
	endm
    endm
endm
?cs2 macro n
endm
; WASM static sEnd: closes the currently open segment based on __wasm_cms_seg.
; Supports up to 4 segments (code, data, initcode, initdata).
sEnd macro
    if __wasm_cms_seg eq 1
	_TEXT ends
    elseif __wasm_cms_seg eq 2
	_DATA ends
    elseif __wasm_cms_seg eq 3
	_INITTEXT ends
    elseif __wasm_cms_seg eq 4
	_INITDATA ends
    endif
    __wasm_cms_seg = 0
endm

sBegin macro ln
    begin&ln
endm

ife ?DF
    createSeg _TEXT,code,byte,public,CODE,IGROUP
    createSeg _DATA,data,word,public,DATA,DGROUP
    if ?WIN
	ife sizeC
	    createSeg _INITTEXT,initcode,byte,public,CODE,IGROUP
	    createSeg _INITDATA,initdata,word,public,DATA,DGROUP
	endif
    endif
    ife sizeC
	defGrp IGROUP
    endif
    defGrp DGROUP
    if sizeC
	codeOFFSET equ OFFSET _TEXT:
    else
    endif
endif

errnz macro x
    if2
	if x
	    errnz1 <x>,%(x)
	endif
    endif
endm

errnz1 macro x1,x2
    = *ERRNZ* x1 = x2
endm

errn$ macro l,x
    errnz <OFFSET $ - OFFSET l x>
endm
