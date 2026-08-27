; A 16-bit fixup to FINE must contain 0xffff, not fail frame validation.
EXTRN FINE:ABS

CODE SEGMENT PARA PUBLIC 'CODE'
ASSUME CS:CODE

start:
    mov dx,FINE
    mov ax,4C00h
    int 21h

CODE ENDS
END start
