;;***********************************************************************
;; NAME: pathlabl
;; DESC: creates a public label at the spot it is placed, using the name
;;       given.
;; INPUT: either module name or procedure name
;; OUTPUT: public label
;; LOGIC: LBL-parameter-name cycles 0->1->2->3->0->... (mod 4)
;;        START label created at 0 or 2 (even), STOP at 1 or 3 (odd).
;;        Designed for MASM two-pass (4 invocations) or WASM multi-pass.
;;***********************************************************************
pathlabl MACRO pnam
IFNDEF LBL_&pnam                        ;;IF THIS IS THE FIRST TIME,
    LBL_&pnam = 0                       ;;DEFINE IT, INITIALLY ZERO
ENDIF

IF ((LBL_&pnam MOD 4) EQ 0) OR ((LBL_&pnam MOD 4) EQ 2) ;;ready for START?
    $$A_START_&pnam:                    ;;create START label
    PUBLIC $$A_START_&pnam              ;;make it public
ELSE                                    ;;SINCE SWITCH IS ODD, CREATE STOP
    $$A_STOP_&pnam:                     ;;create STOP label
    PUBLIC $$A_STOP_&pnam               ;;make it public
ENDIF
LBL_&pnam = LBL_&pnam + 1               ;;INCREMENT SWITCH
ENDM
