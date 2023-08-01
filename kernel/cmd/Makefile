include ../cdef.mk
out_path_src = ../obj/
incpath_src = ../include/
obj_cmd = $(out_path_src)command.o  $(out_path_src)execbatch.o $(out_path_src)chat.o \
          $(out_path_src)netgobang.o
default : $(obj_cmd)
$(out_path_src)%.o : ./%.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -Wall -Wno-packed-bitfield-compat -c ./$*.c -o $(out_path_src)$*.o
