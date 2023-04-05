include ../cdef.mk
out_path_src = ../obj/
incpath_src = ../include/
obj_fs = $(out_path_src)fat12.o $(out_path_src)file.o $(out_path_src)bmp.o \
	     $(out_path_src)jpeg.o $(out_path_src)path.o $(out_path_src)pra.o \
		 $(out_path_src)elf.o $(out_path_src)png.o $(out_path_src)rootfs.o $(out_path_src)vfs.o
default : $(obj_fs)
$(out_path_src)%.o : ./%.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -c ./$*.c -o $(out_path_src)$*.o
