NASM = nasm
OBJS_PACK = ./out/entry.obj ./out/syscall.obj ./out/rand.obj ./out/arg.obj \
			./out/mouse.obj ./out/math.obj ./out/libc.obj ./out/net_syscall.obj ./out/time.obj ./out/posix.obj
OBJS_GUI = ./out/WinEntry.obj ./out/syscall.obj ./out/rand.obj ./out/arg.obj \
			./out/math.obj ./out/gui_syscall.obj ./out/libc.obj ./out/net_syscall.obj
OBJS_PACK_CPP = ./out/CppEntry.obj ./out/syscall.obj ./out/rand.obj ./out/arg.obj \
			./out/mouse.obj ./out/math.obj ./out/libc.obj ./out/net_syscall.obj
MST_LIB = ./out/mst.obj ./out/list.obj
build : $(OBJS_PACK) $(OBJS_GUI) $(OBJS_PACK_CPP) $(MST_LIB) Makefile
	@python3 build.py bainian $(OBJS_PACK)
	@python3 build.py uigobang $(OBJS_GUI)
	@python3 build.py bitz $(OBJS_PACK)
	@python3 build.py calc $(OBJS_PACK)
	@python3 build.py AIGOBANG $(OBJS_PACK)
	@python3 build.py RandNum $(OBJS_PACK)
	@python3 build.py cale $(OBJS_PACK)
	@python3 build.py pfn $(OBJS_PACK)
	@python3 build.py sort $(OBJS_PACK)
	@python3 build.py copy $(OBJS_PACK)
	@python3 build.py pwsh $(OBJS_PACK)
	@python3 build.py snake $(OBJS_PACK)
	@python3 build.py cgobang $(OBJS_PACK)
	@python3 build.py CREUS $(OBJS_PACK)
	@python3 build.py bf $(OBJS_PACK)
	@python3 build.py edit $(OBJS_PACK)
	@python3 build.py cc $(OBJS_PACK)
	@python3 build.py uicalc $(OBJS_GUI)
	@python3 build.py invader $(OBJS_GUI)
	@python3 build.py winver $(OBJS_GUI)
	@python3 build.py login $(OBJS_GUI)
	@python3 build.py student $(OBJS_GUI)
	@python3 build.py tetris $(OBJS_GUI)
	@python3 build.py basic $(OBJS_PACK)
	@python3 build.py setup1 $(OBJS_PACK)
	@python3 build.py Maze $(OBJS_PACK)
	@python3 build.py clock $(OBJS_PACK)
	@python3 build.py nettest $(OBJS_PACK)
	@python3 buildpp.py editor $(OBJS_PACK_CPP) $(MST_LIB)
	@python3 build.py lua $(OBJS_PACK) ./out/gui_syscall.obj
	@python3 build.py luac $(OBJS_PACK)
	@python3 build.py ttf $(OBJS_PACK)
	@python3 build.py tedit $(OBJS_GUI)
	@python3 build.py c4 $(OBJS_PACK)
	@python3 build.py image $(OBJS_PACK)
	@python3 build.py uname $(OBJS_PACK)
	@python3 build.py bim2hrb $(OBJS_PACK)
	@python3 build.py msttest $(OBJS_PACK) $(MST_LIB)
clean :
	rm out/*

./out/%.obj : ./src/%.c Makefile
	@gcc -m32 -I./include -nostdinc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -mno-mmx -mno-sse -mfpmath=387 -Os -w -c ./src/$*.c -o ./out/$*.obj
./out/%.obj : ./src/%.cpp Makefile
	@gcc -m32 -I./include -nostdinc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -mno-mmx -mno-sse -mfpmath=387 -Os -w -c ./src/$*.cpp -o ./out/$*.obj
%.bin : ./src/%.asm Makefile
	@$(NASM) ./src/$*.asm -o ./out/$*.bin

./out/%.obj : src/%.asm Makefile
	nasm -f elf src/$*.asm -o ./out/$*.obj
