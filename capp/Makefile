OBJS_PACK = syscall.obj string.obj math.obj rand.obj \

build : $(OBJS_PACK) Makefile
	build bainian $(OBJS_PACK)
	build test $(OBJS_PACK)

clean:
	del *.obj
	del *.lst
	del *.nas
	del *.gas
	del *.bim
	del *.map
	del *.hrb

%.gas : src\%.c Makefile
	"../z_tools/cc1.exe" -I./include -w  -Os -quiet -o $*.gas src/$*.c

%.nas : %.gas Makefile
	"../z_tools/GAS2NASK.exe" $*.gas $*.nas

%.obj : %.nas Makefile
	"../z_tools/nask.exe" $*.nas $*.obj $*.lst

%.obj : src\%.asm Makefile
	"../z_tools/nask.exe" src\$*.asm $*.obj $*.lst