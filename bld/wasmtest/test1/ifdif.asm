; Test: IFDIF / IFIDN families must read their two args correctly when
; the args use the angle-bracket form (which now lexes as a TC_OP_ANGLE
; TC_RAW_TEXT TC_CL_ANGLE triple rather than a single TC_RAW_TEXT).
; Mixed bracket/string and bracket/bareword pairs are also valid.
.8086
.model  small
.code
    IFDIF <abc>, <abc>          ; identical brackets -> 0
        DB 1
    ELSE
        DB 0
    ENDIF
    IFDIF <abc>, <def>          ; different brackets -> 1
        DB 1
    ELSE
        DB 0
    ENDIF
    IFIDN <abc>, <abc>          ; identical -> 1
        DB 1
    ELSE
        DB 0
    ENDIF
    IFIDN <abc>, <def>          ; different -> 0
        DB 1
    ELSE
        DB 0
    ENDIF
    IFDIF <abc>, "abc"          ; mixed bracket/quoted, same content -> 0
        DB 1
    ELSE
        DB 0
    ENDIF
    IFDIFI <ABC>, <abc>         ; case-insensitive, same -> 0
        DB 1
    ELSE
        DB 0
    ENDIF
    IFIDNI <ABC>, <abc>         ; case-insensitive identical -> 1
        DB 1
    ELSE
        DB 0
    ENDIF
end
