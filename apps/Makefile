NASM = ../tools/nasm.exe
OBJS_PACK = ./out/entry.obj ./out/syscall.obj ./out/rand.obj ./out/arg.obj \
			./out/mouse.obj ./out/math.obj ./out/libc.obj ./out/net_syscall.obj ./out/time.obj
OBJS_GUI = ./out/WinEntry.obj ./out/syscall.obj ./out/rand.obj ./out/arg.obj \
			./out/math.obj ./out/gui_syscall.obj ./out/libc.obj ./out/net_syscall.obj
OBJS_PACK_CPP = ./out/CppEntry.obj ./out/syscall.obj ./out/rand.obj ./out/arg.obj \
			./out/mouse.obj ./out/math.obj ./out/libc.obj ./out/net_syscall.obj
build : $(OBJS_PACK) $(OBJS_GUI) $(OBJS_PACK_CPP) game.bin gobang.bin memz.bin Makefile
	@python build.py bainian $(OBJS_PACK)
	@python build.py test $(OBJS_PACK)
	@python build.py uigobang $(OBJS_GUI)
	@python build.py bitz $(OBJS_PACK)
	@python build.py calc $(OBJS_PACK)
	@python build.py AIGOBANG $(OBJS_PACK)
	@python build.py RandNum $(OBJS_PACK)
	@python build.py cale $(OBJS_PACK)
	@python build.py pfn $(OBJS_PACK)
	@python build.py oi $(OBJS_PACK)
	@python build.py sort $(OBJS_PACK)
	@python build.py copy $(OBJS_PACK)
	@python build.py ps2test $(OBJS_PACK)
	@python build.py pwsh $(OBJS_PACK)
	@python build.py snake $(OBJS_PACK)
	@python build.py cgobang $(OBJS_PACK)
	@python build.py CREUS $(OBJS_PACK)
	@python build.py bf $(OBJS_PACK)
	@python build.py edit $(OBJS_PACK)
	@python build.py cc $(OBJS_PACK)
	@python build.py uicalc $(OBJS_GUI)
	@python build.py invader $(OBJS_GUI)
	@python build.py winver $(OBJS_GUI)
	@python build.py login $(OBJS_GUI)
	@python build.py student $(OBJS_GUI)
	@python build.py tetris $(OBJS_GUI)
	@python build.py basic $(OBJS_PACK)
	@python build.py setup1 $(OBJS_PACK)
	@python build.py Maze $(OBJS_PACK)
	@python build.py clock $(OBJS_PACK)
	@python build.py nettest $(OBJS_PACK)
	@python buildpp.py editor $(OBJS_PACK_CPP)
	@python build.py lua $(OBJS_PACK) ./out/gui_syscall.obj
	@python build.py luac $(OBJS_PACK)
	@python buildpp.py SPGAME $(OBJS_PACK_CPP)
	@python buildpp.py CodeEdit $(OBJS_PACK_CPP)
	@python build.py ttf $(OBJS_PACK)
	@python build.py tedit $(OBJS_GUI)
	@python build.py c4 $(OBJS_PACK)
	@python build.py image $(OBJS_PACK)
clean :
	del out /s/q

./out/%.obj : ./src/%.c Makefile
	@gcc -m32 -I./include -nostdinc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -mno-mmx -mno-sse -mfpmath=387 -Os -w -c ./src/$*.c -o ./out/$*.obj
./out/%.obj : ./src/%.cpp Makefile
	@gcc -m32 -I./include -nostdinc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -mno-mmx -mno-sse -mfpmath=387 -Os -w -c ./src/$*.cpp -o ./out/$*.obj
%.bin : ./src/%.asm Makefile
	@$(NASM) ./src/$*.asm -o ./out/$*.bin

./out/%.obj : src/%.asm Makefile
	"../tools/nasm.exe" --prefix _ -f win32 src/$*.asm -o ./out/$*.obj
