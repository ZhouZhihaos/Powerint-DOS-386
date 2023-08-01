include ../cdef.mk
out_path_src = ../obj/
incpath_src = ../include/
obj_fs = $(out_path_src)fat.o $(out_path_src)file.o $(out_path_src)bmp.o \
	     $(out_path_src)path.o $(out_path_src)elf.o $(out_path_src)rootfs.o \
		 $(out_path_src)vfs.o $(out_path_src)pfs.o
default : $(obj_fs)
$(out_path_src)%.o : ./%.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -Wno-packed-bitfield-compat -c ./$*.c -o $(out_path_src)$*.o
