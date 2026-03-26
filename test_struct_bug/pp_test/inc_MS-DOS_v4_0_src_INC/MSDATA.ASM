;	SCCSID = @(#)ibmdata.asm	1.1 85/04/10
;
; DATA Segment for DOS.
;
; (listing directive removed for WASM)
include MSSW.ASM
include DOSSEG.ASM
debug = FALSE                           ; No dossym (too big)
INCLUDE DOSMAC.INC
INCLUDE SF.INC
INCLUDE DIRENT.INC
INCLUDE CURDIR.INC
INCLUDE DPB.INC
INCLUDE BUFFER.INC
INCLUDE ARENA.INC
INCLUDE VECTOR.INC
INCLUDE DEVSYM.INC
INCLUDE PDB.INC
INCLUDE FIND.INC
INCLUDE MI.INC
; (listing directive removed for WASM)

TITLE   IBMDATA - DATA segment for DOS
NAME    IBMDATA

installed = TRUE

include MS_DATA.ASM
include MSINIT.ASM
	END
