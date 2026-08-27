all: abs.exe

absdef.obj: absdef.asm
	wasm -q -Foabsdef.obj absdef.asm

absuse.obj: absuse.asm
	wasm -q -Foabsuse.obj absuse.asm

abs.exe: absdef.obj absuse.obj
	wlink option quiet system dos name abs.exe file absdef.obj,absuse.obj

clean: .SYMBOLIC
	rm -f absdef.obj absuse.obj absdef.o absuse.o abs.exe
