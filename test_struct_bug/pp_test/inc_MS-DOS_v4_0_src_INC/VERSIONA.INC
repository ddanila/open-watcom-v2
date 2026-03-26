
IFNDEF major_version
major_version	    equ     4	    ;Major DOS version
ENDIF
IFNDEF minor_version
minor_version	    equ     00	    ;Minor DOS Version
ENDIF
IFNDEF expected_version
expected_version    equ     (minor_version SHL 8)+major_version  ;; WASM: fix case to match definitions above
ENDIF
;; WASM -Mx: add uppercase alias used by MSGSERV.ASM and COMMAND.CL? message files
IFNDEF EXPECTED_VERSION
EXPECTED_VERSION    EQU     expected_version
ENDIF


       ;******************************
       ;Each assembler program should:
       ;  mov ah,030h			;DOS Get Version function
       ;  int 021h			;Version ret. in AX,minor version first
       ;  cmp ax,expected_version	;ALL utilities should check for an
       ;  jne error_handler		; EXACT version match.
       ;******************************

