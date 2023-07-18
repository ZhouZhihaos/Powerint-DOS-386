TOOLPATH = ../tools/
DOSPATH = ./base/
DRVPATH = ./driver/
OUTPATH = ./out/
FSPATH  = ./fs/
MAKE     = make -r
NASM     = nasm
RULEFILE = make.rul
DEL      = del

OBJS_BOOTPACK = $(OUTPATH)8295a.o $(OUTPATH)asmfunc.obj $(OUTPATH)BasicVideo.o $(OUTPATH)cmos.o $(OUTPATH)dma.o $(OUTPATH)dosldr.o \
			    $(OUTPATH)vdisk.o $(OUTPATH)fat.o $(OUTPATH)fdc.o $(OUTPATH)file.o $(OUTPATH)gdtidt.o $(OUTPATH)ide.o $(OUTPATH)libc.o \
				$(OUTPATH)List.o $(OUTPATH)mm.o $(OUTPATH)vfs.o $(OUTPATH)other.o

default :
	$(MAKE) -C $(DOSPATH)
	$(MAKE) -C $(DRVPATH)
	$(MAKE) -C $(FSPATH)
	$(MAKE) $(OUTPATH)dosldr.bin

$(OUTPATH)dosldr.bin : $(OBJS_BOOTPACK) Makefile
	ld -m elf_i386 -static -Ttext 0x100000 -e loader_main $(OBJS_BOOTPACK) -o $(OUTPATH)dosldr.bin

clean :
	rm out/*
