TOOLPATH = ../tools/
APPSPATH = ../apps/
APPSOUTPATH = ../apps/out/
FAT12TOOLSPATH = ../fat12tools/
BOOTPATH = ./boot/
DOSPATH = ./dos/
CMDPATH = ./cmd/
GUIPATH = ./gui/
DRIVERSPATH = ./drivers/
IOPATH = ./io/
STDPATH = ./std/
FSPATH = ./fs/
NETPATH = ./net/
ZLIBPATH = ./zlib/
RESPATH = ./res/
INCPATH  = ./include/
OUTPATH = ./obj/
IMGPATH = ./img/
LOADERPATH = ../Loader/out/

MAKE     = $(TOOLPATH)make.exe -r
NASM     = $(TOOLPATH)nasm.exe
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
FAT12TOOLS = $(FAT12TOOLSPATH)fat12tools.exe -file $(IMGPATH)Powerint_DOS_386.img
RULEFILE = make.rul
EDIMG    = $(TOOLPATH)edimg.exe
COPY     = copy
DEL      = del

OBJS_BOOTPACK = $(OUTPATH)kernelc.o $(OUTPATH)screen.o $(OUTPATH)command.o $(OUTPATH)execbatch.o $(OUTPATH)timer.o $(OUTPATH)fifo.o \
				$(OUTPATH)mem.o $(OUTPATH)file.o $(OUTPATH)setjmp.obj $(OUTPATH)int32.obj $(OUTPATH)i386.obj \
				$(OUTPATH)handlers.obj $(OUTPATH)errors.obj $(OUTPATH)vga.o $(OUTPATH)harddisk.o $(OUTPATH)TextMode.o  \
				$(OUTPATH)mouse.o $(OUTPATH)acpi.o $(OUTPATH)Input_Stack.o $(OUTPATH)gdtidt.o $(OUTPATH)arg.o $(OUTPATH)CASM.o \
				$(OUTPATH)other.o $(OUTPATH)task.o $(OUTPATH)floppy.o $(OUTPATH)dma.o $(OUTPATH)init.o $(OUTPATH)main.o \
				$(OUTPATH)std.o $(OUTPATH)sheet.o $(OUTPATH)graphic.o $(OUTPATH)vbe.o $(OUTPATH)cmos.o $(OUTPATH)pci.o \
				$(OUTPATH)fat.o $(OUTPATH)beep.o $(OUTPATH)jpeg.o  $(OUTPATH)gui_syscall.o $(OUTPATH)syscall.o \
				$(OUTPATH)list.o $(OUTPATH)input.o $(OUTPATH)keyboard.o $(OUTPATH)pcnet.o $(OUTPATH)ipc.o \
				$(OUTPATH)tty.o $(OUTPATH)page.o $(OUTPATH)serial.o $(OUTPATH)log.o $(OUTPATH)path.o $(OUTPATH)pra.o \
				$(OUTPATH)Draw.o $(OUTPATH)HighTextMode.o $(OUTPATH)pic.o $(OUTPATH)bmp.o $(OUTPATH)button.o $(OUTPATH)window.o \
				$(OUTPATH)sb16.o $(OUTPATH)lock.o $(OUTPATH)elf.o $(OUTPATH)rtc.o $(OUTPATH)driver.o $(OUTPATH)md5.o $(OUTPATH)libcpp.o \
				$(OUTPATH)rand.o $(OUTPATH)time.o $(OUTPATH)png.o $(OUTPATH)listbox.o $(OUTPATH)etherframe.o \
				$(OUTPATH)arp.o $(OUTPATH)ipv4.o $(OUTPATH)icmp.o $(OUTPATH)udp.o $(OUTPATH)dhcp.o $(OUTPATH)tcp.o $(OUTPATH)chat.o \
				$(OUTPATH)dns.o $(OUTPATH)socket.o $(OUTPATH)bit.o $(OUTPATH)loadbox.o $(OUTPATH)http.o \
				$(OUTPATH)gui.o $(OUTPATH)gui_login.o  $(OUTPATH)adler32.o $(OUTPATH)compress.o $(OUTPATH)crc32.o \
				$(OUTPATH)deflate.o $(OUTPATH)gzclose.o $(OUTPATH)gzlib.o \
				$(OUTPATH)gzread.o $(OUTPATH)gzwrite.o $(OUTPATH)infback.o \
				$(OUTPATH)inffast.o $(OUTPATH)inflate.o $(OUTPATH)inftrees.o \
				$(OUTPATH)trees.o $(OUTPATH)uncompr.o $(OUTPATH)zutil.o $(OUTPATH)ftp.o\
			    $(OUTPATH)linux_style_file.o $(OUTPATH)netgobang.o $(OUTPATH)ide.o $(OUTPATH)net_syscall.o $(OUTPATH)ntp.o \
				$(OUTPATH)vdisk.o $(OUTPATH)rootfs.o $(OUTPATH)mount.o $(OUTPATH)network.o $(OUTPATH)rtl8139.o $(OUTPATH)GraphicMode.o \
				$(OUTPATH)vfs.o

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
	$(MAKE) -C $(ZLIBPATH)
	$(MAKE) Mimg
	
# 镜像文件生成

$(OUTPATH)boot.bin : $(BOOTPATH)boot.asm Makefile
	$(NASM) $(BOOTPATH)boot.asm -o $(OUTPATH)boot.bin

$(OUTPATH)kernel.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(OUTPATH)kernel.bim stack:3136k map:$(OUTPATH)kernel.map \
		$(OBJS_BOOTPACK)
# 3MB+64KB=3136KB

$(OUTPATH)kernel.bin : $(OUTPATH)kernel.bim Makefile
	$(BIM2HRB) $(OUTPATH)kernel.bim $(OUTPATH)kernel.bin 0

Mimg : $(OUTPATH)kernel.bin $(OUTPATH)boot.bin Makefile
	$(EDIMG)   imgin:../tools/fdimg0at.tek \
		wbinimg src:$(OUTPATH)boot.bin len:512 from:0 to:0 \
		copy from:$(LOADERPATH)dosldr.bin to:@: \
		copy from:$(OUTPATH)kernel.bin to:@: \
		copy from:$(RESPATH)autoexec.bat to:@: \
		copy from:$(RESPATH)path.sys to:@: \
		copy from:$(OUTPATH)boot.bin to:@: \
		copy from:$(RESPATH)setup.bat to:@: \
		copy from:$(APPSOUTPATH)setup1.bin to:@: \
		copy from:$(APPSOUTPATH)creus.bin to:@: \
		copy from:$(APPSOUTPATH)bainian.bin to:@: \
		imgout:$(IMGPATH)Powerint_DOS_386.img
		$(FAT12TOOLS) -mkdir other \
		-copy other/font.bin ../tools/font/font.bin \
		-copy other/HZK16 ../tools/font/HZK16 \
		-copy other/copy.bin $(APPSOUTPATH)copy.bin \
		-copy other/uname.bin $(APPSOUTPATH)uname.bin
	$(EDIMG)   imgin:../tools/fdimg0at.tek \
		wbinimg src:$(OUTPATH)boot.bin len:512 from:0 to:0 \
		copy from:$(APPSOUTPATH)uigobang.bin to:@: \
		copy from:$(APPSOUTPATH)aigobang.bin to:@: \
		copy from:$(APPSOUTPATH)snake.bin to:@: \
		copy from:$(APPSOUTPATH)gobang.bin to:@: \
		copy from:$(APPSOUTPATH)cgobang.bin to:@: \
		copy from:$(APPSOUTPATH)game.bin to:@: \
		copy from:$(APPSOUTPATH)login.bin to:@: \
		copy from:$(APPSOUTPATH)pwsh.bin to:@: \
		copy from:$(APPSOUTPATH)pfn.bin to:@: \
		copy from:$(APPSOUTPATH)RandNum.bin to:@: \
		copy from:$(APPSOUTPATH)sort.bin to:@: \
		copy from:$(APPSOUTPATH)student.bin to:@: \
		copy from:$(APPSOUTPATH)bf.bin to:@: \
		copy from:$(APPSOUTPATH)copy.bin to:@: \
		copy from:$(APPSOUTPATH)basic.bin to:@: \
		copy from:$(APPSOUTPATH)lua.bin to:@: \
		copy from:$(APPSOUTPATH)cale.bin to:@: \
		copy from:$(APPSOUTPATH)bitz.bin to:@: \
		copy from:$(APPSOUTPATH)editor.bin to:@: \
		copy from:$(APPSOUTPATH)codeedit.bin to:@: \
		copy from:$(APPSOUTPATH)calc.bin to:@: \
		copy from:$(APPSOUTPATH)clock.bin to:@: \
		copy from:$(APPSOUTPATH)Maze.bin to:@: \
		copy from:$(APPSOUTPATH)winver.bin to:@: \
		copy from:$(APPSOUTPATH)uicalc.bin to:@: \
		copy from:$(APPSOUTPATH)invader.bin to:@: \
		copy from:$(APPSOUTPATH)tetris.bin to:@: \
		copy from:$(APPSOUTPATH)cc.bin to:@: \
		copy from:$(APPSOUTPATH)spgame.bin to:@: \
		copy from:$(APPSOUTPATH)ps2test.bin to:@: \
		copy from:$(APPSOUTPATH)msttest.bin to:@: \
		copy from:$(RESPATH)codeedit.ini to:@: \
		copy from:$(RESPATH)editor.mst to:@: \
		imgout:$(IMGPATH)Disk2.img
	$(EDIMG)   imgin:../tools/fdimg0at.tek \
		wbinimg src:$(OUTPATH)boot.bin len:512 from:0 to:0 \
		copy from:$(APPSOUTPATH)ttf.bin to:@: \
		copy from:$(RESPATH)font.ttf to:@: \
		copy from:$(APPSOUTPATH)tedit.bin to:@: \
		copy from:$(APPSOUTPATH)image.bin to:@: \
		copy from:$(APPSOUTPATH)c4.bin to:@: \
		copy from:$(APPSOUTPATH)luac.bin to:@: \
		imgout:$(IMGPATH)Disk3.img
	$(EDIMG)   imgin:../tools/fdimg0at.tek \
		wbinimg src:$(OUTPATH)boot.bin len:512 from:0 to:0 \
		copy from:$(RESPATH)demo.asm to:@: \
		copy from:$(RESPATH)demo2.asm to:@: \
		copy from:$(RESPATH)coin.wav to:@: \
		copy from:$(RESPATH)test.wav to:@: \
		copy from:$(RESPATH)desktop.jpg to:@: \
		copy from:$(RESPATH)huaji.jpg to:@: \
		copy from:$(RESPATH)load.jpg to:@: \
		copy from:$(RESPATH)io.bas to:@: \
		copy from:$(RESPATH)shot.png to:@: \
		copy from:$(RESPATH)shot1.png to:@: \
		copy from:$(RESPATH)test.htm to:@: \
		copy from:$(RESPATH)test1.htm to:@: \
		copy from:$(RESPATH)yee.bmp to:@: \
		copy from:$(RESPATH)fib.lua to:@: \
		copy from:$(RESPATH)guess.lua to:@: \
		copy from:$(RESPATH)gobang.lua to:@: \
		copy from:$(RESPATH)bainian.lua to:@: \
		copy from:$(RESPATH)window.lua to:@: \
		copy from:$(RESPATH)pluaui.lua to:@: \
		copy from:$(RESPATH)uicalc.lua to:@: \
		copy from:$(RESPATH)chinese.txt to:@: \
		copy from:$(RESPATH)bf.txt to:@: \
		copy from:../apps/src/c4.c to:@: \
		imgout:$(IMGPATH)res.img


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
	$(MAKE) -C $(ZLIBPATH)
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
	$(MAKE) -C $(ZLIBPATH)
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
	$(MAKE) -C $(ZLIBPATH)
	$(MAKE) -C $(APPSPATH)
	$(MAKE) Mimg
	.\run.bat
clean :
	-$(DEL) obj /s/q
	$(MAKE) -C $(APPSPATH) clean
