include ../cdef.mk
out_path_src = ../obj/
incpath_src = ../include/
obj_gui = $(out_path_src)graphic.o $(out_path_src)button.o $(out_path_src)window.o $(out_path_src)gui_syscall.o \
					$(out_path_src)listbox.o $(out_path_src)loadbox.o $(out_path_src)gui.o $(out_path_src)gui_login.o
default : $(obj_gui)
$(out_path_src)%.o : ./%.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -c ./$*.c -o $(out_path_src)$*.o
$(out_path_src)%.o : %.cpp Makefile
	g++ -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312  -fno-use-cxa-atexit -fleading-underscore -fno-exceptions -fno-builtin -nostdlib -fno-rtti -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -c $*.cpp -o $(out_path_src)$*.o
