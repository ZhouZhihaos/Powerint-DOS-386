TOOLPATH = ../tools/
DOSPATH = ./base/
DRVPATH = ./driver/
OUTPATH = ./out/
FSPATH  = ./fs/
MAKE     = $(TOOLPATH)make.exe -r
NASM     = $(TOOLPATH)nasm.exe
OBJ2BIM  = $(TOOLPATH)obj2bim.exe
BIM2HRB  = $(TOOLPATH)bim2hrb.exe
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

$(OUTPATH)dosldr.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(OUTPATH)dosldr.bim stack:3136k map:$(OUTPATH)dosldr.map \
		$(OBJS_BOOTPACK)

$(OUTPATH)dosldr.bin : $(OUTPATH)dosldr.bim Makefile
	$(BIM2HRB) $(OUTPATH)dosldr.bim $(OUTPATH)dosldr.bin 0

clean :
	-$(DEL) out /s/q
	$(MAKE) -C $(APPSPATH) clean
