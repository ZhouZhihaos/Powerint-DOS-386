include ../../cdef.mk
out_path_src := ../../obj/
incpath_src := ../../include/
TOOLPATH := ../../../tools/
MAKE     := make
obj_dos := $(out_path_src)errors.obj $(out_path_src)handlers.obj $(out_path_src)i386.obj $(out_path_src)int32.obj $(out_path_src)setjmp.obj
default : $(obj_dos)
	
$(out_path_src)%.o : %.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -c $*.c -o $(out_path_src)$*.o

$(out_path_src)%.obj : %.asm Makefile
	nasm -f elf $*.asm -o $(out_path_src)$*.obj
