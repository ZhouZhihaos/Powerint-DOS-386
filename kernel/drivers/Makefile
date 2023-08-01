include ../cdef.mk
out_path_src := ../obj/
incpath_src := ../include/
TOOLPATH := ../../tools/
MAKE     := $(TOOLPATH)make.exe -r
obj_driver = $(out_path_src)acpi.o $(out_path_src)beep.o $(out_path_src)cmos.o \
	     $(out_path_src)serial.o $(out_path_src)dma.o $(out_path_src)floppy.o \
	     $(out_path_src)harddisk.o $(out_path_src)keyboard.o $(out_path_src)mouse.o \
	     $(out_path_src)pci.o $(out_path_src)pcnet.o $(out_path_src)vbe.o $(out_path_src)pic.o \
	     $(out_path_src)vga.o $(out_path_src)sb16.o $(out_path_src)driver.o $(out_path_src)timer.o \
		 $(out_path_src)ide.o $(out_path_src)vdisk.o $(out_path_src)network.o $(out_path_src)rtl8139.o
default : $(obj_driver)
$(out_path_src)%.o : ./%.c Makefile
	gcc -m32 -I$(incpath_src) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 -mno-mmx -mno-sse -mfpmath=387 $(CDEFS) -w -Wno-packed-bitfield-compat -c ./$*.c -o $(out_path_src)$*.o
