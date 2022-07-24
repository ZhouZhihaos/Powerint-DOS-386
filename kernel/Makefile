TOOLPATH = ../z_tools/
APPSPATH = ../apps/
APPSOUTPATH = ../apps/out/
BOOTPATH = ./boot/
DOSPATH = ./dos/
CMDPATH = ./cmd/
GUIPATH = ./gui/
DRIVERSPATH = ./drivers/
IOPATH = ./io/
STDPATH = ./std/
FSPATH = ./fs/
NETPATH = ./net/
RESPATH = ./res/
INCPATH  = ./include/
OUTPATH = ./obj/
IMGPATH = ./img/

MAKE     = $(TOOLPATH)make.exe -r
NASM     = $(TOOLPATH)nasm.exe
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
FAT12TOOLS = $(TOOLPATH)fat12tools.exe -file $(IMGPATH)Powerint_DOS_386.img
RULEFILE = make.rul
EDIMG    = $(TOOLPATH)edimg.exe
COPY     = copy
DEL      = del

OBJS_BOOTPACK = $(OUTPATH)kernelc.o $(OUTPATH)screen.o $(OUTPATH)command.o $(OUTPATH)execbatch.o $(OUTPATH)timer.o $(OUTPATH)fifo.o \
				$(OUTPATH)mem.o $(OUTPATH)file.o $(OUTPATH)nasmfunc.obj $(OUTPATH)setup.o $(OUTPATH)vga.o $(OUTPATH)harddisk.o \
				$(OUTPATH)CASM.o $(OUTPATH)mouse.o $(OUTPATH)acpi.o $(OUTPATH)Input_Stack.o $(OUTPATH)gdtidt.o $(OUTPATH)arg.o \
				$(OUTPATH)other.o $(OUTPATH)task.o $(OUTPATH)floppy.o $(OUTPATH)dma.o $(OUTPATH)init.o $(OUTPATH)main.o \
				$(OUTPATH)std.o $(OUTPATH)sheet.o $(OUTPATH)graphic.o $(OUTPATH)vbe.o $(OUTPATH)cmos.o $(OUTPATH)pci.o \
				$(OUTPATH)fat12.o $(OUTPATH)pak.o $(OUTPATH)beep.o $(OUTPATH)jpeg.o  $(OUTPATH)gui_syscall.o \
				$(OUTPATH)syscall.o $(OUTPATH)list.o $(OUTPATH)input.o $(OUTPATH)keyboard.o $(OUTPATH)pcnet.o $(OUTPATH)ipc.o \
				$(OUTPATH)tty.o $(OUTPATH)page.o $(OUTPATH)com.o $(OUTPATH)log.o $(OUTPATH)path.o $(OUTPATH)pra.o \
				$(OUTPATH)Draw.o $(OUTPATH)HighTextMode.o $(OUTPATH)pic.o $(OUTPATH)bmp.o $(OUTPATH)button.o $(OUTPATH)window.o \
				$(OUTPATH)sb16.o $(OUTPATH)lock.o $(OUTPATH)elf.o $(OUTPATH)rtc.o $(OUTPATH)driver.o $(OUTPATH)md5.o $(OUTPATH)libcpp.o \
				$(OUTPATH)rand.o $(OUTPATH)time.o $(OUTPATH)png.o $(OUTPATH)listbox.o $(OUTPATH)etherframe.o \
				$(OUTPATH)arp.o $(OUTPATH)ipv4.o $(OUTPATH)icmp.o $(OUTPATH)udp.o $(OUTPATH)dhcp.o $(OUTPATH)tcp.o $(OUTPATH)chat.o \
				$(OUTPATH)dns.o $(OUTPATH)cdrom.o $(OUTPATH)socket.o $(OUTPATH)bit.o $(OUTPATH)loadbox.o $(OUTPATH)http.o
# 默认动作

default :
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(IOPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) -C $(DRIVERSPATH)
	$(MAKE) -C $(STDPATH)
	$(MAKE) -C $(CMDPATH)
	$(MAKE) -C $(GUIPATH)
	$(MAKE) -C $(NETPATH)
	$(MAKE) Mimg
	
# 镜像文件生成

$(OUTPATH)dosldr : $(BOOTPATH)dosldr.asm Makefile
	$(NASM) $(BOOTPATH)dosldr.asm -o $(OUTPATH)dosldr

$(OUTPATH)fdboot.bin : $(BOOTPATH)fdboot.asm Makefile
	$(NASM) $(BOOTPATH)fdboot.asm -o $(OUTPATH)fdboot.bin

$(OUTPATH)hdboot.bin : $(BOOTPATH)hdboot.asm Makefile
	$(NASM) $(BOOTPATH)hdboot.asm -o $(OUTPATH)hdboot.bin

$(OUTPATH)kernel.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(OUTPATH)kernel.bim stack:3136k map:$(OUTPATH)kernel.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

$(OUTPATH)kernel.bin : $(OUTPATH)kernel.bim Makefile
	$(BIM2HRB) $(OUTPATH)kernel.bim $(OUTPATH)kernel.bin 0

Mimg : $(OUTPATH)kernel.bin $(OUTPATH)fdboot.bin $(OUTPATH)hdboot.bin $(OUTPATH)dosldr Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:$(OUTPATH)fdboot.bin len:512 from:0 to:0 \
		copy from:$(OUTPATH)dosldr to:@: \
		copy from:$(BOOTPATH)menu.lst to:@: \
		copy from:$(OUTPATH)kernel.bin to:@: \
		copy from:$(RESPATH)autoexec.bat to:@: \
		copy from:$(RESPATH)path.sys to:@: \
		copy from:$(RESPATH)load.jpg to:@: \
		copy from:$(APPSOUTPATH)CREUS.bin to:@: \
		copy from:$(APPSOUTPATH)uigobang.bin to:@: \
		copy from:$(APPSOUTPATH)test.bin to:@: \
		copy from:$(APPSOUTPATH)uicalc.bin to:@: \
		copy from:$(APPSOUTPATH)invader.bin to:@: \
		copy from:$(APPSOUTPATH)winver.bin to:@: \
		copy from:$(APPSOUTPATH)memz.bin to:@: \
		copy from:$(APPSOUTPATH)login.bin to:@: \
		copy from:$(APPSOUTPATH)student.bin to:@: \
		copy from:$(APPSOUTPATH)tetris.bin to:@: \
		imgout:$(IMGPATH)Powerint_DOS_386.img
		$(FAT12TOOLS) -mkdir other \
		-copy other/font.bin ../z_tools/font/font.bin \
		-copy other/HZK16 ../z_tools/font/HZK16 \
		-copy other/demo.asm $(RESPATH)demo.asm \
		-copy other/AIGOBANG.bin $(APPSOUTPATH)AIGOBANG.bin \
		-copy other/snake.bin $(APPSOUTPATH)snake.bin \
		-copy other/pwsh.bin $(APPSOUTPATH)pwsh.bin \
		-copy other/cale.bin $(APPSOUTPATH)cale.bin \
		-copy other/copy.bin $(APPSOUTPATH)copy.bin \
		-copy other/bitz.bin $(APPSOUTPATH)bitz.bin \
		-copy other/edit.bin $(APPSOUTPATH)edit.bin \

# 运行程序

img_run :
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(IOPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) -C $(DRIVERSPATH)
	$(MAKE) -C $(STDPATH)
	$(MAKE) -C $(CMDPATH)
	$(MAKE) -C $(GUIPATH)
	$(MAKE) -C $(NETPATH)
	$(MAKE) Mimg
	.\run.bat
full :
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(IOPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) -C $(DRIVERSPATH)
	$(MAKE) -C $(STDPATH)
	$(MAKE) -C $(CMDPATH)
	$(MAKE) -C $(GUIPATH)
	$(MAKE) -C $(NETPATH)
	$(MAKE) -C $(APPSPATH)
	$(MAKE) Mimg
run:
	.\run.bat
full_run:
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(IOPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) -C $(DRIVERSPATH)
	$(MAKE) -C $(STDPATH)
	$(MAKE) -C $(CMDPATH)
	$(MAKE) -C $(GUIPATH)
	$(MAKE) -C $(NETPATH)
	$(MAKE) -C $(APPSPATH)
	$(MAKE) Mimg
	.\run.bat
clean :
	-$(DEL) obj /s/q
	$(MAKE) -C $(APPSPATH) clean
