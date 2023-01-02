include ../cdef.mk
out_path_src = ../obj/
incpath_src = ../include/
obj_zlib =  $(out_path_src)adler32.o $(out_path_src)compress.o $(out_path_src)crc32.o \
			$(out_path_src)deflate.o $(out_path_src)gzclose.o $(out_path_src)gzlib.o \
			$(out_path_src)gzread.o $(out_path_src)gzwrite.o $(out_path_src)infback.o \
			$(out_path_src)inffast.o $(out_path_src)inflate.o $(out_path_src)inftrees.o \
			$(out_path_src)trees.o $(out_path_src)uncompr.o $(out_path_src)zutil.o
default :  $(obj_zlib)
$(out_path_src)%.o : ./%.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -c ./$*.c -o $(out_path_src)$*.o
