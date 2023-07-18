include ../cdef.mk
out_path_src := ../obj/
incpath_src := ../include/
TOOLPATH := ../../tools/
MAKE     := make
default :
	$(MAKE) -C asm
	$(MAKE) -C cpu
	$(MAKE) -C hal
	$(MAKE) -C init
	$(MAKE) -C mm
	$(MAKE) -C syscall
	$(MAKE) -C task
	$(MAKE) -C tools
	
