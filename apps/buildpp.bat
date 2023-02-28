@echo off
echo Building %1
gcc -m32 -I./include -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -O0 -finput-charset=UTF-8 -fexec-charset=GB2312  -fno-use-cxa-atexit -fleading-underscore -fno-exceptions -fno-builtin -nostdlib -mno-mmx -mno-sse -mfpmath=387 -w -c .\src\%1*.cpp -o .\out\%1.obj
"../tools/OBJ2BIM.exe"  @make.rul  out:./out/%1.bim stack:1024k map:./out/%1.map ./out/%1.obj %2 %3 %4 %5 %6 %7 %8 %9
"../tools/BIM2HRB.exe" ./out/%1.bim ./out/%1.bin 0