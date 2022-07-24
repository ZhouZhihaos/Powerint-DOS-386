include ../cdef.mk
out_path_src = ../obj/
incpath_src = ../include/
obj_io = $(out_path_src)Draw.o $(out_path_src)HighTextMode.o $(out_path_src)input.o \
	     $(out_path_src)Input_Stack.o $(out_path_src)log.o $(out_path_src)screen.o \
	     $(out_path_src)tty.o $(out_path_src)sheet.o
default : $(obj_io)
$(out_path_src)%.o : ./%.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 $(CDEFS) -w -c ./$*.c -o $(out_path_src)$*.o
