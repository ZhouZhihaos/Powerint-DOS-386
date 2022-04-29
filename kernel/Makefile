TOOLPATH = ../z_tools/
INCPATH  = ./include/
CAPPPATH = ../capp/
out_path = ./obj
img_path = ./img

MAKE     = $(TOOLPATH)make.exe -r
NASM     = $(TOOLPATH)nasm.exe
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
FAT12TOOLS = $(TOOLPATH)fat12tools.exe -file $(img_path)/Powerint_DOS_386.img
RULEFILE = make.rul
EDIMG    = $(TOOLPATH)edimg.exe
COPY     = copy
DEL      = del
CDEFS    = -DGRAPHICS_LOADING_UI

OBJS_BOOTPACK = $(out_path)/kernelc.o $(out_path)/screen.o $(out_path)/shell.o $(out_path)/timer.o $(out_path)/fifo.o \
				$(out_path)/mem.o $(out_path)/file.o $(out_path)/nasmfunc.obj $(out_path)/setup.o $(out_path)/vga.o $(out_path)/hd.o \
				$(out_path)/CASM.o $(out_path)/mouse.o $(out_path)/acpi.o $(out_path)/Input_Stack.o $(out_path)/gdtidt.o \
				$(out_path)/other.o $(out_path)/task.o $(out_path)/floppy.o $(out_path)/irq.o $(out_path)/dma.o $(out_path)/init.o $(out_path)/main.o \
				$(out_path)/std.o $(out_path)/sheet.o $(out_path)/graphic.o $(out_path)/vbe.o $(out_path)/cmos.o $(out_path)/pci.o\
				$(out_path)/fs.o $(out_path)/pak.o $(out_path)/beep.o $(out_path)/jpeg.o $(out_path)/inthandler.o \
				$(out_path)/syscall.o $(out_path)/int32.o $(out_path)/input.o $(out_path)/keyboard.o \
				$(out_path)/tty.o $(out_path)/page.o $(out_path)/com.o $(out_path)/log.o
# 默认动作

default :
	$(MAKE) -C $(CAPPPATH)
	$(MAKE) Mimg
	
# 镜像文件生成

$(out_path)/dosldr : boot/dosldr.asm Makefile
	$(NASM) boot/dosldr.asm -o $(out_path)/dosldr

$(out_path)/kernel.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(out_path)/kernel.bim stack:3136k map:$(out_path)/kernel.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

$(out_path)/kernel.bin : $(out_path)/kernel.bim Makefile
	$(BIM2HRB) $(out_path)/kernel.bim $(out_path)/kernel.bin 0

Mimg : $(out_path)/fdboot.bin $(out_path)/kernel.bin $(out_path)/hdboot.bin $(out_path)/game.bin $(out_path)/gobang.bin $(out_path)/fdboot.bin $(out_path)/hdboot.bin $(out_path)/dosldr Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:$(out_path)/fdboot.bin len:512 from:0 to:0 \
		copy from:$(out_path)/dosldr to:@: \
		copy from:boot/menu.lst to:@: \
		copy from:$(out_path)/kernel.bin to:@: \
		copy from:autoexec.bat to:@: \
		copy from:$(out_path)/hdboot.bin to:@: \
		copy from:demo.cas to:@: \
		copy from:yee.bmp to:@: \
		copy from:huaji.jpg to:@: \
		copy from:load.jpg to:@: \
		copy from:$(CAPPPATH)test.bin to:@: \
		copy from:$(CAPPPATH)CREUS.bin to:@: \
		copy from:bf.txt to:@: \
		imgout:$(img_path)/Powerint_DOS_386.img
		$(FAT12TOOLS) -mkdir other \
		-copy other/AIGOBANG.bin $(CAPPPATH)AIGOBANG.bin \
		-copy other/snake.bin $(CAPPPATH)snake.bin \
		-copy other/pwsh.bin $(CAPPPATH)pwsh.bin \
		-copy other/pfn.bin $(CAPPPATH)pfn.bin \
		-copy other/sort.bin $(CAPPPATH)sort.bin \
		-copy other/calc.bin $(CAPPPATH)calc.bin \
		-copy other/hello.bin $(CAPPPATH)hello.bin \
		-copy other/cale.bin $(CAPPPATH)cale.bin \
		-copy other/copy.bin $(CAPPPATH)copy.bin \
		-copy other/ps2test.bin $(CAPPPATH)ps2test.bin \
		-copy other/bitz.bin $(CAPPPATH)bitz.bin \
		-copy other/bainian.bin $(CAPPPATH)bainian.bin \
		-copy other/font1.bin ../z_tools/font/font1.bin \
		-copy other/font.bin ../z_tools/font/font.bin \
		-copy other/HZK16 ../z_tools/font/HZK16 \
		-copy other/game.bin $(out_path)/game.bin \
		-copy other/gobang.bin $(out_path)/gobang.bin \
		-copy other/RandNum.bin $(CAPPPATH)RandNum.bin \
		-copy other/bf.bin $(CAPPPATH)bf.bin \
		-copy other/edit.bin $(CAPPPATH)edit.bin \
		-copy other/readme.txt readme.txt

# 其他指令

$(out_path)/%.o : .\src\%.c Makefile
	gcc -m32 -I$(INCPATH) -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312 $(CDEFS) -w -c .\src\$*.c -o $(out_path)/$*.o
$(out_path)/%.obj : .\src\%.asm Makefile
	$(NASM) -f win32 src/$*.asm -o $(out_path)/$*.obj
$(out_path)/%.bin : .\src\%.asm Makefile
	$(NASM) .\src\$*.asm -o $(out_path)/$*.bin

# 运行程序

img_run :
	$(MAKE) Mimg
	.\run.bat
full :
	$(MAKE) -C $(CAPPPATH)
	$(MAKE) Mimg
run:
	.\run.bat
full_run:
	$(MAKE) -C $(CAPPPATH)
	$(MAKE) Mimg
	.\run.bat
clean :
	-$(DEL) obj /s/q
	$(MAKE) -C $(CAPPPATH) clean
