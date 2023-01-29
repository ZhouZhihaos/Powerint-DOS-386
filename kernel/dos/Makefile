include ../cdef.mk
out_path_src = ../obj/
incpath_src = ../include/
obj_dos = $(out_path_src)kernelc.o $(out_path_src)task.o $(out_path_src)page.o $(out_path_src)ipc.o \
			$(out_path_src)init.o $(out_path_src)main.o $(out_path_src)mem.o $(out_path_src)timer.o \
			$(out_path_src)other.o $(out_path_src)gdtidt.o $(out_path_src)fifo.o $(out_path_src)list.o \
			$(out_path_src)nasmfunc.obj $(out_path_src)syscall.o $(out_path_src)lock.o $(out_path_src)md5.o \
			$(out_path_src)rand.o $(out_path_src)time.o $(out_path_src)arg.o $(out_path_src)bit.o \
			$(out_path_src)mount.o
default : $(obj_dos)
	
$(out_path_src)%.o : %.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -c $*.c -o $(out_path_src)$*.o

$(out_path_src)%.obj : %.asm Makefile
	"../../tools/nasm.exe" --prefix _ -f win32 $*.asm -o $(out_path_src)$*.obj
