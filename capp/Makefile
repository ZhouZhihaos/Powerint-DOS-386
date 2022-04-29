NASM = ../z_tools/nasm.exe
OBJS_PACK = syscall.obj string.obj rand.obj arg.obj mouse.obj \

build : $(OBJS_PACK) load.bin Makefile
	gcc -m32 -I../kernel/include/ -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -w -c ../kernel/src/std.c -o std.obj
	gcc -m32 -I./include/ -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -w -c ./src/entry.c -o entry.obj
	build bainian entry.obj std.obj $(OBJS_PACK)
	build test entry.obj std.obj $(OBJS_PACK)
	build bitz entry.obj std.obj $(OBJS_PACK)
	build calc entry.obj std.obj $(OBJS_PACK)
	build AIGOBANG entry.obj std.obj $(OBJS_PACK)
	build RandNum entry.obj std.obj $(OBJS_PACK)
	build cale entry.obj std.obj $(OBJS_PACK)
	build pfn entry.obj std.obj $(OBJS_PACK)
	build hello entry.obj std.obj $(OBJS_PACK)
	build sort entry.obj std.obj $(OBJS_PACK)
	build copy entry.obj std.obj $(OBJS_PACK)
	build ps2test entry.obj std.obj $(OBJS_PACK)
	build pwsh entry.obj std.obj $(OBJS_PACK)
	build snake entry.obj std.obj $(OBJS_PACK)
	build cgobang entry.obj std.obj $(OBJS_PACK)
	build CREUS entry.obj std.obj $(OBJS_PACK)
	build bf entry.obj std.obj $(OBJS_PACK)
	build edit entry.obj std.obj $(OBJS_PACK)
clean:
	del *.obj
	del *.bim
	del *.map
	del *.hrb
	del *.bin
%.obj : .\src\%.c Makefile
	gcc -m32 -I./include -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -w -c .\src\$*.c -o $*.obj

%.bin : .\src\%.asm Makefile
	$(NASM) .\src\$*.asm -o $*.bin

%.obj : src\%.asm Makefile
	"../z_tools/nasm.exe" -f win32 src\$*.asm -o $*.obj
