include ../cdef.mk
incpath_src = ../include/
define File
$(shell dir /s /w /b $(patsubst %.o,%.c,$(notdir $@)))
endef
SRCS := BasicVideo.c 8295a.c gdtidt.c ide.c dsk.c fdc.c dma.c cmos.c
OBJS := $(patsubst %.c,../out/%.o,$(SRCS)) ../out/asmfunc.obj
CC = gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w
default : $(OBJS)
../out/%.o: %.c
	$(CC) -c $*.c -o $@
../out/%.obj: %.asm
	nasm --prefix _ -f win32 $*.asm -o $@