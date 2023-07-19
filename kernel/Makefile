FONTPATH = ../font/
APPSPATH = ../apps/
APPSOUTPATH = ../apps/out/
BIDPATH = ../apps/build_tools_in_pdos/
FATTOOLSPATH = ../fattools/
BOOTPATH = ./boot/
DOSPATH = ./dos/
CMDPATH = ./cmd/
GUIPATH = ./gui/
DRIVERSPATH = ./drivers/
IOPATH = ./io/
STDPATH = ./std/
FSPATH = ./fs/
NETPATH = ./net/
MSTPATH = ./mst/
RESPATH = ./res/
INCPATH  = ./include/
OUTPATH = ./obj/
IMGPATH = ./img/
LOADERPATH = ../Loader/out/

MAKE     = make -r
NASM     = nasm
EDIMG    = ../tools/edimg
FATTOOLS = $(FATTOOLSPATH)fattools -file $(IMGPATH)Powerint_DOS_386.img
RULEFILE = make.rul
COPY     = copy
DEL      = rm

OBJS_BOOTPACK = $(OUTPATH)kernelc.o $(OUTPATH)screen.o $(OUTPATH)command.o $(OUTPATH)execbatch.o $(OUTPATH)timer.o \
				$(OUTPATH)mem.o $(OUTPATH)file.o $(OUTPATH)setjmp.obj $(OUTPATH)int32.obj $(OUTPATH)i386.obj \
				$(OUTPATH)handlers.obj $(OUTPATH)errors.obj $(OUTPATH)vga.o $(OUTPATH)harddisk.o $(OUTPATH)TextMode.o  \
				$(OUTPATH)mouse.o $(OUTPATH)acpi.o $(OUTPATH)Input_Stack.o $(OUTPATH)gdtidt.o $(OUTPATH)arg.o \
				$(OUTPATH)other.o $(OUTPATH)task.o $(OUTPATH)floppy.o $(OUTPATH)dma.o $(OUTPATH)init.o $(OUTPATH)main.o \
				$(OUTPATH)std.o $(OUTPATH)sheet.o $(OUTPATH)graphic.o $(OUTPATH)vbe.o $(OUTPATH)cmos.o $(OUTPATH)pci.o \
				$(OUTPATH)fat.o $(OUTPATH)beep.o $(OUTPATH)gui_syscall.o $(OUTPATH)syscall.o $(OUTPATH)fifo.o \
				$(OUTPATH)list.o $(OUTPATH)input.o $(OUTPATH)keyboard.o $(OUTPATH)pcnet.o $(OUTPATH)ipc.o \
				$(OUTPATH)tty.o $(OUTPATH)page.o $(OUTPATH)serial.o $(OUTPATH)log.o $(OUTPATH)path.o $(OUTPATH)Draw.o \
				$(OUTPATH)HighTextMode.o $(OUTPATH)pic.o $(OUTPATH)bmp.o $(OUTPATH)button.o $(OUTPATH)window.o \
				$(OUTPATH)sb16.o $(OUTPATH)lock.o $(OUTPATH)elf.o $(OUTPATH)driver.o $(OUTPATH)md5.o $(OUTPATH)libcpp.o \
				$(OUTPATH)rand.o $(OUTPATH)time.o $(OUTPATH)listbox.o $(OUTPATH)etherframe.o $(OUTPATH)arp.o $(OUTPATH)ipv4.o \
				$(OUTPATH)icmp.o $(OUTPATH)udp.o $(OUTPATH)dhcp.o $(OUTPATH)tcp.o $(OUTPATH)chat.o $(OUTPATH)dns.o \
				$(OUTPATH)socket.o $(OUTPATH)bit.o $(OUTPATH)loadbox.o $(OUTPATH)http.o $(OUTPATH)gui.o $(OUTPATH)gui_login.o \
				$(OUTPATH)ftp.o $(OUTPATH)linux_style_file.o $(OUTPATH)netgobang.o $(OUTPATH)ide.o $(OUTPATH)net_syscall.o \
				$(OUTPATH)ntp.o $(OUTPATH)vdisk.o $(OUTPATH)rootfs.o $(OUTPATH)mount.o $(OUTPATH)network.o $(OUTPATH)rtl8139.o \
				$(OUTPATH)GraphicMode.o $(OUTPATH)vfs.o $(OUTPATH)build.o $(OUTPATH)mst.o $(OUTPATH)str.o $(OUTPATH)pvector.o \
				$(OUTPATH)psheet.o $(OUTPATH)pgui.o $(OUTPATH)box.o $(OUTPATH)pwindow.o $(OUTPATH)pbutton.o $(OUTPATH)ptextbox.o \
				$(OUTPATH)pconsole.o

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
	$(MAKE) -C $(MSTPATH)
	$(MAKE) Mimg
	
# 镜像文件生成

$(OUTPATH)boot.bin : $(BOOTPATH)boot.asm Makefile
	$(NASM) $(BOOTPATH)boot.asm -o $(OUTPATH)boot.bin
$(OUTPATH)boot32.bin : $(BOOTPATH)boot32.asm Makefile
	$(NASM) $(BOOTPATH)boot32.asm -o $(OUTPATH)boot32.bin
$(OUTPATH)kernel.bin : $(OBJS_BOOTPACK) Makefile
	ld -m elf_i386 -static -Ttext 0x280000 -e KernelMain $(OBJS_BOOTPACK) -o $(OUTPATH)kernel.bin
Mimg : $(OUTPATH)kernel.bin $(OUTPATH)boot.bin $(OUTPATH)boot32.bin Makefile
	$(EDIMG)   imgin:../tools/fdimg0at.tek \
		wbinimg src:$(OUTPATH)boot.bin len:512 from:0 to:0 \
		copy from:$(LOADERPATH)dosldr.bin to:@: \
		copy from:$(OUTPATH)kernel.bin to:@: \
		copy from:$(RESPATH)autoexec.bat to:@: \
		copy from:$(RESPATH)env.cfg to:@: \
		copy from:$(OUTPATH)boot.bin to:@: \
		copy from:$(OUTPATH)boot32.bin to:@: \
		copy from:$(RESPATH)setup.bat to:@: \
		copy from:$(APPSOUTPATH)setup1.bin to:@: \
		copy from:$(APPSOUTPATH)CREUS.bin to:@: \
		copy from:$(APPSOUTPATH)bainian.bin to:@: \
		imgout:$(IMGPATH)Powerint_DOS_386.img
		$(FATTOOLS) -mkdir other \
		-copy other/font.bin ../tools/font/font.bin \
		-copy other/HZK16 ../tools/font/HZK16 \
		-copy other/copy.bin $(APPSOUTPATH)copy.bin \
		-copy other/uname.bin $(APPSOUTPATH)uname.bin \
		-copy other/coin.wav $(RESPATH)coin.wav
	$(EDIMG)   imgin:../tools/fdimg0at.tek \
		wbinimg src:$(OUTPATH)boot.bin len:512 from:0 to:0 \
		copy from:$(APPSOUTPATH)uigobang.bin to:@: \
		copy from:$(APPSOUTPATH)aigobang.bin to:@: \
		copy from:$(APPSOUTPATH)snake.bin to:@: \
		copy from:$(APPSOUTPATH)cgobang.bin to:@: \
		copy from:$(APPSOUTPATH)pfn.bin to:@: \
		copy from:$(APPSOUTPATH)RandNum.bin to:@: \
		copy from:$(APPSOUTPATH)bf.bin to:@: \
		copy from:$(APPSOUTPATH)copy.bin to:@: \
		copy from:$(APPSOUTPATH)basic.bin to:@: \
		copy from:$(APPSOUTPATH)lua.bin to:@: \
		copy from:$(APPSOUTPATH)cale.bin to:@: \
		copy from:$(APPSOUTPATH)bitz.bin to:@: \
		copy from:$(APPSOUTPATH)editor.bin to:@: \
		copy from:$(APPSOUTPATH)calc.bin to:@: \
		copy from:$(APPSOUTPATH)clock.bin to:@: \
		copy from:$(APPSOUTPATH)Maze.bin to:@: \
		copy from:$(APPSOUTPATH)winver.bin to:@: \
		copy from:$(APPSOUTPATH)invader.bin to:@: \
		copy from:$(RESPATH)make.rul to:@: \
		copy from:$(BIDPATH)nask.bin to:@: \
		copy from:$(BIDPATH)obj2bim.bin to:@: \
		copy from:$(APPSOUTPATH)bim2hrb.bin to:@: \
		copy from:$(BIDPATH)plib.lib to:@: \
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
		copy from:$(RESPATH)hello.nas to:@: \
		copy from:$(RESPATH)asm.lua to:@: \
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
	$(MAKE) -C $(MSTPATH)
	$(MAKE) Mimg
	qemu-system-i386 -net nic,model=pcnet -net user -serial stdio -device sb16 -device floppy -fda ./img/Powerint_DOS_386.img -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/res.img -boot a -m 512
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
	$(MAKE) -C $(MSTPATH)
	$(MAKE) Mimg
run:
	qemu-system-i386 -net nic,model=pcnet -net user -serial stdio -device sb16 -device floppy -fda ./img/Powerint_DOS_386.img -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/res.img -boot a -m 512
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
	qemu-system-i386 -net nic,model=pcnet -net user -serial stdio -device sb16 -device floppy -fda ./img/Powerint_DOS_386.img -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/res.img -boot a -m 512
clean :
	rm obj/*
	$(MAKE) -C $(APPSPATH) clean
