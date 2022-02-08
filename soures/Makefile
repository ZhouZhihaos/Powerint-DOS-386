OBJS_BOOTPACK = kernelc.obj screen.obj shell.obj timer.obj fifo.obj \
				mem.obj file.obj naskfunc.obj setup.obj vga.obj hd.obj \
				CASM.obj mouse.obj acpi.obj \

TOOLPATH = ../z_tools/
INCPATH  = ./include/
CAPPPATH = ../capp/

MAKE     = $(TOOLPATH)make.exe -r
NASK     = $(TOOLPATH)nask.exe
NASM     = $(TOOLPATH)nasm.exe
CC1      = $(TOOLPATH)cc1.exe -I$(INCPATH) -w -Os -quiet
GAS2NASK = $(TOOLPATH)gas2nask.exe -a
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIN2OBJ  = $(TOOLPATH)bin2obj.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
RULEFILE = make.rul
EDIMG    = $(TOOLPATH)edimg.exe
COPY     = copy
DEL      = del

# 默认动作

default :
	$(MAKE) clean
	$(MAKE) Mimg

# 镜像文件生成

dosldr : src/dosldr.asm Makefile
	$(NASM) src/dosldr.asm -o dosldr.bin
	rename dosldr.bin dosldr

naskfunc.obj : src/naskfunc.nas Makefile
	$(NASK) src/naskfunc.nas naskfunc.obj naskfunc.lst

kernel.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:kernel.bim stack:3136k map:kernel.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

kernel.bin : kernel.bim Makefile
	$(BIM2HRB) kernel.bim kernel.hrb 0
	rename kernel.hrb kernel.bin

Mimg : fdboot.bin kernel.bin hdboot.bin game.bin gobang.bin fdboot.bin hdboot.bin dosldr Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:fdboot.bin len:512 from:0 to:0 \
		copy from:dosldr to:@: \
		copy from:../menu.lst to:@: \
		copy from:kernel.bin to:@: \
		copy from:autoexec.bat to:@: \
		copy from:hdboot.bin to:@: \
		copy from:../command.bin to:@: \
		copy from:../shell.bin to:@: \
		copy from:../clock.bin to:@: \
		copy from:../z_tools/font/font1.bin to:@: \
		copy from:../z_tools/font/HZK16 to:@: \
		copy from:game.bin to:@: \
		copy from:gobang.bin to:@: \
		copy from:$(CAPPPATH)Cgobang.bin to:@: \
		copy from:$(CAPPPATH)bainian.bin to:@: \
		copy from:$(CAPPPATH)test.bin to:@: \
		copy from:demo.cas to:@: \
		copy from:view.bmp to:@: \
		copy from:../readme.txt to:@: \
		imgout:Powerint_DOS_386.img

# 其他指令

%.gas : .\src\%.c Makefile
	$(CC1) -o $*.gas .\src\$*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

%.bin : .\src\%.asm Makefile
	$(NASM) .\src\$*.asm -o $*.bin

# 运行程序

run :
	$(MAKE) clean
	$(MAKE) Mimg
	qemu-system-i386 -fda Powerint_DOS_386.img

clean :
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.obj
	-$(DEL) kernel.map
	-$(DEL) kernel.bim
	-$(DEL) dosldr
