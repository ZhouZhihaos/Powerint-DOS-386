include ../cdef.mk
out_path_src = ../obj/
incpath_src = ../include/
obj_std = $(out_path_src)std.o $(out_path_src)libcpp.o $(out_path_src)linux_style_file.o
default : $(obj_std)
$(out_path_src)%.o : ./%.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -Os -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -c ./$*.c -o $(out_path_src)$*.o
$(out_path_src)%.o : %.cpp Makefile
	g++ -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -Os -finput-charset=UTF-8 -fexec-charset=GB2312  -fno-use-cxa-atexit -fno-exceptions -fno-builtin -nostdlib -fno-rtti -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -c $*.cpp -o $(out_path_src)$*.o
