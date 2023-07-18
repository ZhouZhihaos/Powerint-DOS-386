import sys
import os
argv = sys.argv[0:]
print("Building " + argv[1])
build_com = "gcc -m32 -I./include -nostdinc -nolibc -nostdlib -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -finput-charset=UTF-8 -fexec-charset=GB2312 -Qn -O0 -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-3dnow -mfpmath=387 -w -c ./src/" + argv[1] + ".c -o ./out/" + argv[1] + ".obj"
os.system(build_com)
build_com = "ld -m elf_i386 -static -Ttext 0x0 -e Main ./out/" + argv[1] + ".obj "
for i in range(2,len(argv)):
    build_com += argv[i]
    build_com += " "
build_com += "-o ./out/" + argv[1] + ".bin"
os.system(build_com)
