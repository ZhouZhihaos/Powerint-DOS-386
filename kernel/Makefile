FONTPATH = ../tools/font/
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
FATTOOLS = $(FATTOOLSPATH)fattools
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
				$(OUTPATH)pconsole.o $(OUTPATH)pgui_api.o

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
	$(FATTOOLS) -file $(IMGPATH)Powerint_DOS_386.img \
		-format $(OUTPATH)boot.bin 12 \
		-copy $(LOADERPATH)dosldr.bin dosldr.bin \
		-copy $(OUTPATH)kernel.bin kernel.bin \
		-copy $(RESPATH)autoexec.bat autoexec.bat \
		-copy $(RESPATH)env.cfg env.cfg \
		-copy $(OUTPATH)boot.bin boot.bin \
		-copy $(OUTPATH)boot32.bin boot32.bin \
		-copy $(RESPATH)setup.bat setup.bat \
		-copy $(APPSOUTPATH)setup1.bin setup1.bin \
		-copy $(APPSOUTPATH)CREUS.bin creus.bin \
		-copy $(APPSOUTPATH)bainian.bin bainian.bin \
		-copy $(APPSOUTPATH)winver.bin winver.bin \
		-mkdir other \
		-copy $(FONTPATH)font.bin other/font.bin \
		-copy $(FONTPATH)HZK16 other/HZK16 \
		-copy $(APPSOUTPATH)copy.bin other/copy.bin \
		-copy $(APPSOUTPATH)uname.bin other/uname.bin \
		-copy $(RESPATH)coin.wav other/coin.wav
	$(FATTOOLS) -file $(IMGPATH)Disk2.img \
		-format $(OUTPATH)boot.bin 12 \
		-copy $(APPSOUTPATH)uigobang.bin uigobang.bin \
		-copy $(APPSOUTPATH)aigobang.bin aigobang.bin \
		-copy $(APPSOUTPATH)snake.bin snake.bin \
		-copy $(APPSOUTPATH)cgobang.bin cgobang.bin \
		-copy $(APPSOUTPATH)pfn.bin pfn.bin \
		-copy $(APPSOUTPATH)RandNum.bin RandNum.bin \
		-copy $(APPSOUTPATH)bf.bin bf.bin \
		-copy $(APPSOUTPATH)copy.bin copy.bin \
		-copy $(APPSOUTPATH)basic.bin basic.bin \
		-copy $(APPSOUTPATH)lua.bin lua.bin \
		-copy $(APPSOUTPATH)cale.bin cale.bin \
		-copy $(APPSOUTPATH)bitz.bin bitz.bin \
		-copy $(APPSOUTPATH)editor.bin editor.bin \
		-copy $(APPSOUTPATH)calc.bin calc.bin \
		-copy $(APPSOUTPATH)clock.bin clock.bin \
		-copy $(APPSOUTPATH)Maze.bin Maze.bin \
		-copy $(APPSOUTPATH)winver.bin winver.bin \
		-copy $(APPSOUTPATH)invader.bin invader.bin \
		-copy $(RESPATH)make.rul make.rul \
		-copy $(BIDPATH)nask.bin nask.bin \
		-copy $(BIDPATH)obj2bim.bin obj2bim.bin \
		-copy $(APPSOUTPATH)bim2hrb.bin bim2hrb.bin \
		-copy $(BIDPATH)plib.lib plib.lib \
		-copy $(RESPATH)asm.lua asm.lua
	$(FATTOOLS) -file $(IMGPATH)Disk3.img \
		-format $(OUTPATH)boot.bin 12 \
		-copy $(APPSOUTPATH)ttf.bin ttf.bin \
		-copy $(RESPATH)font.ttf font.ttf \
		-copy $(APPSOUTPATH)tedit.bin tedit.bin \
		-copy $(APPSOUTPATH)image.bin image.bin \
		-copy $(APPSOUTPATH)c4.bin c4.bin \
		-copy $(APPSOUTPATH)luac.bin luac.bin
	$(FATTOOLS) -file $(IMGPATH)res.img \
		-format $(OUTPATH)boot.bin 12 \
		-copy $(RESPATH)hello.nas hello.nas \
		-copy $(RESPATH)asm.lua asm.lua \
		-copy $(RESPATH)coin.wav coin.wav \
		-copy $(RESPATH)test.wav test.wav \
		-copy $(RESPATH)desktop.jpg desktop.jpg \
		-copy $(RESPATH)huaji.jpg huaji.jpg \
		-copy $(RESPATH)load.jpg load.jpg \
		-copy $(RESPATH)io.bas io.bas \
		-copy $(RESPATH)shot.png shot.png \
		-copy $(RESPATH)shot1.png shot1.png \
		-copy $(RESPATH)test.htm test.htm \
		-copy $(RESPATH)test1.htm test1.htm \
		-copy $(RESPATH)yee.bmp yee.bmp \
		-copy $(RESPATH)fib.lua fib.lua \
		-copy $(RESPATH)guess.lua guess.lua \
		-copy $(RESPATH)gobang.lua gobang.lua \
		-copy $(RESPATH)bainian.lua bainian.lua \
		-copy $(RESPATH)window.lua window.lua \
		-copy $(RESPATH)pluaui.lua pluaui.lua \
		-copy $(RESPATH)uicalc.lua uicalc.lua \
		-copy $(RESPATH)chinese.txt chinese.txt \
		-copy $(RESPATH)bf.txt bf.txt \
		-copy ../apps/src/c4.c c4.c


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
	qemu-system-i386 -net nic,model=pcnet -net user -serial stdio -device sb16 -device floppy -fda ./img/Powerint_DOS_386.img -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/res.img -boot a -m 2048
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
