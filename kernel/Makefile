FONTPATH = ../font/
APPSPATH = ../apps/
APPSOUTPATH = ../apps/out/
BIDPATH = ../apps/build_tools_in_pdos/
FATTOOLSPATH = ../fattools/
BOOTPATH = ./boot/
DOSPATH = ./dos/
CMDPATH = ./cmd/
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
RULEFILE = make.rul
COPY     = copy
DEL      = rm

OBJS_BOOTPACK = $(OUTPATH)kernelc.o $(OUTPATH)screen.o $(OUTPATH)command.o $(OUTPATH)execbatch.o $(OUTPATH)timer.o \
				$(OUTPATH)mem.o $(OUTPATH)file.o $(OUTPATH)setjmp.obj $(OUTPATH)int32.obj $(OUTPATH)i386.obj \
				$(OUTPATH)handlers.obj $(OUTPATH)errors.obj $(OUTPATH)vga.o $(OUTPATH)harddisk.o $(OUTPATH)TextMode.o  \
				$(OUTPATH)mouse.o $(OUTPATH)acpi.o $(OUTPATH)Input_Stack.o $(OUTPATH)gdtidt.o $(OUTPATH)arg.o \
				$(OUTPATH)other.o $(OUTPATH)task.o $(OUTPATH)floppy.o $(OUTPATH)dma.o $(OUTPATH)init.o $(OUTPATH)main.o \
				$(OUTPATH)std.o $(OUTPATH)sheet.o $(OUTPATH)vbe.o $(OUTPATH)cmos.o $(OUTPATH)pci.o $(OUTPATH)fat.o \
				$(OUTPATH)beep.o $(OUTPATH)syscall.o $(OUTPATH)fifo.o $(OUTPATH)list.o $(OUTPATH)input.o $(OUTPATH)keyboard.o \
				$(OUTPATH)pcnet.o $(OUTPATH)ipc.o $(OUTPATH)tty.o $(OUTPATH)page.o $(OUTPATH)serial.o $(OUTPATH)log.o \
				$(OUTPATH)path.o $(OUTPATH)Draw.o $(OUTPATH)HighTextMode.o $(OUTPATH)pic.o $(OUTPATH)bmp.o $(OUTPATH)sb16.o \
				$(OUTPATH)lock.o $(OUTPATH)elf.o $(OUTPATH)driver.o $(OUTPATH)md5.o $(OUTPATH)libcpp.o $(OUTPATH)rand.o \
				$(OUTPATH)time.o $(OUTPATH)etherframe.o $(OUTPATH)arp.o $(OUTPATH)ipv4.o $(OUTPATH)icmp.o $(OUTPATH)udp.o \
				$(OUTPATH)dhcp.o $(OUTPATH)tcp.o $(OUTPATH)chat.o $(OUTPATH)dns.o $(OUTPATH)socket.o $(OUTPATH)bit.o \
				$(OUTPATH)http.o $(OUTPATH)ftp.o $(OUTPATH)linux_style_file.o $(OUTPATH)netgobang.o $(OUTPATH)ide.o \
				$(OUTPATH)net_syscall.o $(OUTPATH)ntp.o $(OUTPATH)vdisk.o $(OUTPATH)rootfs.o $(OUTPATH)mount.o \
				$(OUTPATH)network.o $(OUTPATH)rtl8139.o $(OUTPATH)vfs.o $(OUTPATH)build.o $(OUTPATH)mst.o $(OUTPATH)str.o $(OUTPATH)pfs.o

# 默认动作

default :
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(IOPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) -C $(DRIVERSPATH)
	$(MAKE) -C $(STDPATH)
	$(MAKE) -C $(CMDPATH)
	$(MAKE) -C $(NETPATH)
	$(MAKE) -C $(MSTPATH)
	$(MAKE) Mimg
	
# 镜像文件生成

$(OUTPATH)boot.bin : $(BOOTPATH)boot.asm Makefile
	$(NASM) $(BOOTPATH)boot.asm -o $(OUTPATH)boot.bin
$(OUTPATH)boot_pfs.bin : $(BOOTPATH)boot_pfs.asm Makefile
	$(NASM) $(BOOTPATH)boot_pfs.asm -o $(OUTPATH)boot_pfs.bin
$(OUTPATH)boot32.bin : $(BOOTPATH)boot32.asm Makefile
	$(NASM) $(BOOTPATH)boot32.asm -o $(OUTPATH)boot32.bin

$(OUTPATH)kernel.bin : $(OBJS_BOOTPACK) Makefile
	ld -m elf_i386 -static -Ttext 0x280000 -e KernelMain $(OBJS_BOOTPACK) -o $(OUTPATH)kernel.bin

make_img:
	qemu-img create $(IMGPATH)Powerint_DOS_386.img 1474560
	qemu-img create $(IMGPATH)Disk2.img 1474560
	qemu-img create $(IMGPATH)Disk3.img 1474560
	qemu-img create $(IMGPATH)res.img 1474560
	qemu-img create $(IMGPATH)tcc.img 1474560
	mformat -f 1440 -B $(OUTPATH)boot.bin -i $(IMGPATH)Powerint_DOS_386.img
	mformat -f 1440 -B $(OUTPATH)boot.bin -i $(IMGPATH)Disk2.img
	mformat -f 1440 -B $(OUTPATH)boot.bin -i $(IMGPATH)Disk3.img
	mformat -f 1440 -B $(OUTPATH)boot.bin -i $(IMGPATH)res.img
	mformat -f 1440 -B $(OUTPATH)boot.bin -i $(IMGPATH)tcc.img
	
Mimg : $(OUTPATH)kernel.bin $(OUTPATH)boot.bin $(OUTPATH)boot_pfs.bin $(OUTPATH)boot32.bin Makefile
	$(MAKE) make_img
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(LOADERPATH)dosldr.bin ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(OUTPATH)kernel.bin ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(RESPATH)autoexec.bat ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(RESPATH)env.cfg ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(OUTPATH)boot.bin ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(OUTPATH)boot32.bin ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(OUTPATH)boot_pfs.bin ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(RESPATH)setup.bat ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(APPSOUTPATH)setup1.bin ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(APPSOUTPATH)CREUS.bin ::
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(APPSOUTPATH)bainian.bin ::
		mmd -i $(IMGPATH)Powerint_DOS_386.img other
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(FONTPATH)font.bin ::/other
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(FONTPATH)HZK16 ::/other
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(APPSOUTPATH)copy.bin ::/other
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(APPSOUTPATH)uname.bin ::/other
		mcopy -i $(IMGPATH)Powerint_DOS_386.img $(RESPATH)coin.wav ::/other
		
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)aigobang.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)snake.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)RandNum.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)bf.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)copy.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)basic.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)lua.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)cale.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)bitz.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)editor.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)calc.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)clock.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)pwsh.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(RESPATH)editor.mst ::
		mcopy -i $(IMGPATH)Disk2.img $(RESPATH)make.rul ::
		mcopy -i $(IMGPATH)Disk2.img $(BIDPATH)nask.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(BIDPATH)obj2bim.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(APPSOUTPATH)bim2hrb.bin ::
		mcopy -i $(IMGPATH)Disk2.img $(BIDPATH)plib.lib ::
		
		mcopy -i $(IMGPATH)Disk3.img $(APPSOUTPATH)ttf.bin ::
		mcopy -i $(IMGPATH)Disk3.img $(RESPATH)font.ttf ::
		mcopy -i $(IMGPATH)Disk3.img $(APPSOUTPATH)image.bin ::
		mcopy -i $(IMGPATH)Disk3.img $(APPSOUTPATH)c4.bin ::
		mcopy -i $(IMGPATH)Disk3.img $(APPSOUTPATH)lox.bin ::
		mcopy -i $(IMGPATH)Disk3.img $(APPSOUTPATH)luac.bin ::
		mcopy -i $(IMGPATH)Disk3.img $(BIDPATH)litenes.bin ::
		mcopy -i $(IMGPATH)Disk3.img $(BIDPATH)5.NES ::
		
		mcopy -i $(IMGPATH)res.img $(RESPATH)hello.nas ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)coin.wav ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)test.wav ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)desktop.jpg ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)huaji.jpg ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)load.jpg ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)io.bas ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)shot.png ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)shot1.png ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)test.htm ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)test1.htm ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)yee.bmp ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)fib.lua ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)guess.lua ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)gobang.lua ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)bainian.lua ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)student.lua ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)asm.lua ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)chinese.txt ::
		mcopy -i $(IMGPATH)res.img $(RESPATH)bf.txt ::
		mcopy -i $(IMGPATH)res.img ../apps/src/c4.c ::
		
		mmd -i $(IMGPATH)tcc.img tcc
		mmd -i $(IMGPATH)tcc.img tcc/lib
		mmd -i $(IMGPATH)tcc.img tcc/crt
		mmd -i $(IMGPATH)tcc.img tcc/include
		mmd -i $(IMGPATH)tcc.img tcc/inst
		mcopy -i $(IMGPATH)tcc.img $(APPSOUTPATH)tcc.bin ::
		mcopy -i $(IMGPATH)tcc.img $(APPSOUTPATH)tccinst.bin ::
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)src/tcc/crti.c ::
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)libp.a ::/tcc/lib
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)libtcc1.a ::/tcc/inst
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/arg.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/assert.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/ctype.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/ctypes.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/errno.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/fcntl.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/features.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/inttypes.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/limits.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/locale.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/math.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/net.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/rand.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/setjmp.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/stdarg.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/stdbool.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/stddef.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/stdint.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/stdio.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/stdlib.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/string.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/syscall.h ::/tcc/include
		mcopy -i $(IMGPATH)tcc.img $(APPSPATH)include/time.h ::/tcc/include
# 运行程序

img_run :
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(IOPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) -C $(DRIVERSPATH)
	$(MAKE) -C $(STDPATH)
	$(MAKE) -C $(CMDPATH)
	$(MAKE) -C $(NETPATH)
	$(MAKE) -C $(MSTPATH)
	$(MAKE) Mimg
	qemu-system-i386 -net nic,model=pcnet -net user -serial stdio -device sb16 -device floppy -fda ./img/Powerint_DOS_386.img -hda disk.img -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/tcc.img -boot a -m 2048 -enable-kvm
full :
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(IOPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) -C $(DRIVERSPATH)
	$(MAKE) -C $(STDPATH)
	$(MAKE) -C $(CMDPATH)
	$(MAKE) -C $(NETPATH)
	$(MAKE) -C $(APPSPATH)
	$(MAKE) -C $(MSTPATH)
	$(MAKE) Mimg
run:
	qemu-system-i386 -d in_asm -no-reboot -net nic,model=pcnet -net user -serial stdio -device sb16 -device floppy -fda ./img/Powerint_DOS_386.img -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/res.img -boot a -m 512
full_run:
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(IOPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) -C $(DRIVERSPATH)
	$(MAKE) -C $(STDPATH)
	$(MAKE) -C $(CMDPATH)
	$(MAKE) -C $(NETPATH)
	$(MAKE) -C $(APPSPATH)
	$(MAKE) Mimg
	qemu-system-i386 -net nic,model=pcnet -net user -serial stdio -device sb16 -device floppy -fda ./img/Powerint_DOS_386.img -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/res.img -boot a -m 512 -enable-kvm
clean :
	rm obj/*
	$(MAKE) -C $(APPSPATH) clean
