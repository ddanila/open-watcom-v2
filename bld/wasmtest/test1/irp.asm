; Test: IRP / IRPC with the <list> arg form must read the bracket triple
; correctly. Before the redesign the arg was a single TC_RAW_TEXT token;
; after, it is TC_OP_ANGLE TC_RAW_TEXT TC_CL_ANGLE and for.c walks the
; triple via STRING_VALUE_BODY.
.8086
.model  small
.code
    IRP X, <1, 2, 3>
        DB X
    ENDM
    IRPC X, abc
        DB '&X'
    ENDM
end
