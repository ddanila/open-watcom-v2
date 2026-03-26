;	SCCSID = @(#)dosmac.asm	1.1 85/04/10
;	SCCSID = @(#)dosmac.asm	1.1 85/04/10
;
; Macro file for MSDOS.
; Include guard: WASM (single-pass) rejects macro redefinitions on second inclusion.
IFNDEF DOSMAC_INC_
DOSMAC_INC_ EQU 1
; IF1/IF2 pass-dependent conditionals removed for WASM (single-pass).
; .xcref/.cref listing directives removed (unsupported by WASM).
;

IFNDEF TRUE
IFNDEF TRUE
TRUE    EQU 0FFFFh
ENDIF
ENDIF
IFNDEF FALSE
IFNDEF FALSE
FALSE   EQU 0
ENDIF
ENDIF

;; WASM: guard against re-definition (COMSEG.ASM may define Break before DOSMAC.INC)
IFNDEF _BREAK_DEFINED_
_BREAK_DEFINED_ EQU 1
BREAK   MACRO   subtitle
ENDM
ENDIF

BREAK   <ASMVAR - handle assembly variables once and for all>

AsmVars Macro   varlist
IRP     var,<varlist>
AsmVar  var
ENDM
ENDM

AsmVar  Macro   var
IFNDEF  var
var = FALSE
ENDIF
ENDM

BREAK <I_NEED: declare a variable external, if necessary, and allocate a size>

;
; declare a variable external and allocate a size
;
AsmVar  InstalledData
I_NEED  MACRO   sym,len
IF InstalledData EQ 0
	DATA    SEGMENT WORD PUBLIC 'DATA'
    IFIDN   <len>,<WORD>
	EXTRN   &sym:WORD
    ELSE
	IFIDN   <len>,<DWORD>
	EXTRN   &sym:DWORD
	ELSE
	EXTRN   &sym:BYTE
	ENDIF
    ENDIF
	DATA    ENDS
ENDIF
ENDM

;
; call a procedure that may be external.  The call will be short.
;
DOSInvoke  MACRO   name
	;; IFNDEF/EXTRN removed for WASM: forward refs resolved within segment;
	;; truly external symbols need explicit EXTRN at file level.
	CALL    name
ENDM

PAGE
;
; jump to a label that may be external.  The jump will be near.
;
transfer    MACRO   name
	JUMP    name
ENDM

;
; get a short address in a word
;
short_addr  MACRO   name
    IFDIF   <name>,<?>
	DW  OFFSET DOSGROUP:name
    ELSE
	DW  ?
    ENDIF
ENDM

;
; get a long address in a dword
;
long_addr   MACRO   name
	DD  name
ENDM

;
; declare a PROC near or far but PUBLIC nonetheless
;
?frame        =   0                     ; initial
?aframe       =   0                     ; initial
?stackdepth   =   0                     ; initial stack size
?initstack    =   0                     ; initial stack size


procedure   MACRO   name,distance
	?frame =    0
	?aframe =   2                   ;; remember the pushed BP
	PUBLIC  name
name    PROC    distance
	?initstack    = ?stackdepth     ;; beginning of procedure
ENDM

;
; end a procedure and check that stack depth is preserved
;
EndProc MACRO   name, chk
	IFDIF   <chk>,<NoCheck>       ;; check the stack size
	IF ?initstack NE ?stackdepth    ;; is it different?
	;%OUT    ***** Possible stack size error in name *****
	ENDIF
	ENDIF
name    ENDP
ENDM

PAGE
;
; define a data item to be public and of an appropriate size/type
;

I_AM    MACRO   name,size,init
;; declare the object public
	PUBLIC  name
;; declare the SIZE of the object
    IFIDN <size>,<WORD>
name    LABEL WORD
	I_AM_SIZE   =   1
	I_AM_LEN    =   2
    ELSE
	IFIDN <size>,<DWORD>
name    LABEL DWORD
	I_AM_SIZE   =   2
	I_AM_LEN    =   2
	ELSE
	    IFIDN <size>,<BYTE>
name    LABEL BYTE
	I_AM_SIZE   =   1
	I_AM_LEN    =   1
	    ELSE
name    LABEL BYTE
	I_AM_SIZE   =   size
	I_AM_LEN    =   1
	    ENDIF
	ENDIF
    ENDIF
;; if no initialize then allocate blank storage
    IFB <init>
	DB  I_AM_SIZE*I_AM_LEN DUP (?)
    ELSE
IF InstalledData EQ 0
	IFIDN <size>,<WORD>
	    DW  init
	ELSE
	IFIDN <size>,<DWORD>
	    DW  init
	ELSE
	    DB  init
	ENDIF
	ENDIF
ELSE
	DB  I_AM_SIZE*I_AM_LEN DUP (?)
ENDIF
    ENDIF
ENDM

I_AM_SIZE   = 0
I_AM_LEN    = 0

PAGE

;
; define an entry in a procedure
;
entry macro name
	PUBLIC  name
name:
endm

BREAK <ERROR - store an error code then jump to a label>

error macro code
	MOV AL,code
	transfer    SYS_RET_ERR
ENDM

BREAK <JUMP - plain near jump>
;
; jump to a label (plain near JMP — short-jump optimization removed for WASM)
;

jump    macro lbl
	JMP     lbl
endm

BREAK <RETURN - return from a function>

return  macro x
    local a
a:
	RET
ret_l = a
endm

BREAK <CONDRET - conditional return>

condret macro   cc,ncc
    local   a
    ;; Short-jump optimizer removed for WASM single-pass compatibility.
    ;; Always emit: j<NCC> over RET.
    j&ncc   a
    return
    a:
endm

BREAK <RETZ - return if zero, links up shortwise if necessary>

retz    macro
    condret z,nz
endm

BREAK <RETNZ - return if not zero, links up shortwise if necessary>

retnz   macro
    condret nz,z
endm

BREAK <RETC - return if carry set, links up shortwise if necessary>

retc    macro
    condret c,nc
endm

BREAK <RETNC - return if not carry, links up shortwise if necessary>

retnc   macro
    condret nc,c
endm

BREAK <CONTEXT - set the DOS context to a particular register>

context macro   r
	PUSH    SS
	POP     r
	ASSUME  r:DOSGROUP
endm

BREAK <SaveReg - save a set of registers>

SaveReg MACRO   reglist                 ;; push those registers
IRP reg,<reglist>
	?stackdepth = ?stackdepth + 1
	PUSH    reg
ENDM
ENDM

BREAK <RestoreReg - unsave some registers>

RestoreReg  MACRO   reglist             ;; pop those registers
IRP reg,<reglist>
	?stackdepth = ?stackdepth - 1
	POP     reg
ENDM
ENDM

BREAK <Critical section macros>

EnterCrit MACRO section
	DOSInvoke  E&section
ENDM

LeaveCrit MACRO section
	DOSInvoke  L&section
ENDM

Break   <message - display a message>

AsmVars <ShareF,Cargs,Redirector>

if debug
fmt     MACRO   typ,lev,fmts,args
local   a,b,c
	PUSHF
IFNB <typ>
	TEST    BugTyp,typ
	JZ      c
	CMP     BugLev,lev
	JB      c
ENDIF
	PUSH    AX
	PUSH    BP
	MOV     BP,SP
If (not sharef) and (not redirector)
Table   segment
a       db      fmts,0
Table   ends
	MOV     AX,OFFSET DOSGROUP:a
else
	jmp     short b
a       db      fmts,0
if sharef
b:      mov     ax,offset share:a
else
b:      mov     ax,offset netwrk:a
endif
endif
	PUSH    AX
cargs = 2
IRP item,<args>
IFIDN   <AX>,<item>
	MOV     AX,[BP+2]
ELSE
	MOV     AX,item
ENDIF
	PUSH    AX
cargs = cargs + 2
ENDM
	DOSInvoke  PFMT
	ADD     SP,Cargs
	POP     BP
	POP     AX
c:
	POPF
ENDM
else
fmt macro typ,lev,fmts,args
endm
endif

Break   <DOSAssume - validate assumes>

AsmVar  Debug,$temp

IF debug
DOSAssume   Macro   reg,reglist,message
local a,b
    IFIDN <reg>,<CS>
	$temp   =   1
    ELSE
	IFIDN <reg>,<SS>
	    $temp   =   0
	ELSE
	    ;%out ***** Invalid DOS register reg in DOSAssume *****
	ENDIF
    ENDIF
    IRP r,<reglist>
	IFIDN <r>,<DS>
	    $temp = $temp OR 2
	ELSE
	    IFIDN <r>,<ES>
		$temp = $temp OR 4
	    ELSE
		;%out ***** Invalid register reg in DOSAssume *****
	    ENDIF
	ENDIF
    ENDM
	PUSH    AX
	MOV     AX,$temp
	PUSH    AX
IF SHAREF
	MOV     AX,OFFSET a
ELSE
	MOV     AX,OFFSET DOSGroup:a
ENDIF
	PUSH    AX
	DOSInvoke  SegCheck
	POP     AX
IF SHAREF EQ 0
Table   SEGMENT
a       DB      message,0
Table   ends
ELSE
	JMP     SHORT a
b       DB      message,0
a:
ENDIF
IRP r,<reglist>
    ASSUME  r:DOSGroup
ENDM
ENDM
ELSE
DOSAssume   Macro   reg,reglist,message
IRP r,<reglist>
    ASSUME  r:DOSGroup
ENDM
ENDM
ENDIF

BREAK   <ASSERT - make assertions about registers>

IF DEBUG
Assert  MACRO   kind, objs, message
	LOCAL   a,b
    IFIDN   <kind>,<Z>
	CMP     objs,0
	JZ      a
	fmt <>,<>,<message>
a:
    ELSE
	IFIDN   <kind>,<NZ>
	CMP     objs,0
	JNZ     a
	fmt <>,<>,<message>
a:
	ELSE
    PUSH    AX
	    IRP obj,<objs>
	PUSH    obj
	    ENDM
	    IF SHAREF
    MOV     AX,OFFSET b
	    ELSE
    MOV     AX,OFFSET DOSGroup:b
	    ENDIF
    PUSH    AX
	    IFIDN   <kind>,<ISBUF>
	DOSInvoke  BUFCheck
	    ENDIF
	    IFIDN   <kind>,<ISSFT>
	DOSInvoke  SFTCheck
	    ENDIF
	    IFIDN   <kind>,<ISDPB>
	DOSInvoke  DPBCheck
	    ENDIF
    POP     AX
	    IF SHAREF
    JMP SHORT a
b   DB  Message,0
a:
	    ELSE
Table   segment
b   db  Message,0
Table   ends
	    ENDIF
	ENDIF
    ENDIF
ENDM
ELSE
Assert  Macro
ENDM
ENDIF

Break   <CallInstall - hook to installable pieces>

CallInstall MACRO   name,mpx,fn,save,restore
IF Installed
    IFNB    <save>
	SaveReg <save>
    ENDIF
	MOV     AX,(mpx SHL 8) + fn
	INT     2Fh
    IFNB    <restore>
	RestoreReg  <restore>
    ENDIF
ELSE
	DOSInvoke  name
ENDIF
ENDM

Break   <Stack frame manipulators>

localvar    macro   name,length
local   a
	ifidn   <length>,<BYTE>
	    ?frame =  ?frame + 1
	    a = ?frame
	    name EQU  BYTE PTR [BP-a]
	else
	    ifidn   <length>,<WORD>
		?frame =  ?frame + 2
		a = ?frame
		name EQU  WORD PTR [BP-a]
	    else
		ifidn   <length>,<DWORD>
		    ?frame =  ?frame + 4
		    a = ?frame
		    name EQU  DWORD PTR [BP-a]
		    name&l EQU  WORD PTR [BP-a]
		    name&h EQU  WORD PTR [BP-a+2]
		else
		    ?frame =  ?frame + length
		    a = ?frame
		    name EQU  BYTE PTR [BP-a]
		endif
	    endif
	endif
endm

;; enter/leave renamed to FrameEnter/FrameLeave: WASM treats enter/leave as
;; 80186 instructions and ignores user macro definitions with those names.
FrameEnter   macro
	push    bp
	mov     bp,sp
	sub     sp,?frame
endm

FrameLeave   macro
	mov     sp,bp
	pop     bp
endm

Argvar  macro   name,length
local   a
	ifidn   <length>,<BYTE>
	    a = ?aframe
	    ?aframe =  ?aframe + 1
	    name EQU  BYTE PTR [BP+a]
	else
	    ifidn   <length>,<WORD>
		a = ?aframe
		?aframe =  ?aframe + 2
		name EQU  WORD PTR [BP+a]
	    else
		ifidn   <length>,<DWORD>
		    a = ?aframe
		    ?aframe =  ?aframe + 4
		    name EQU  DWORD PTR [BP+a]
		    name&l EQU  WORD PTR [BP+a]
		    name&h EQU  WORD PTR [BP+a+2]
		else
		    a = ?aframe
		    ?aframe =  ?aframe + length
		    name EQU  BYTE PTR [BP+a]
		endif
	    endif
	endif
endm

BREAK   <errnz - generate compilation errors>

errnz   macro   x
if x NE 0
    ;%out ***** FATAL error: x <> 0
foobar
endif
endm

ENDIF ;; DOSMAC_INC_
