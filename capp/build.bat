gcc -m32 -I./include -nostdinc -nolibc -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -Qn -fno-pic -fno-pie -fno-asynchronous-unwind-tables -mpreferred-stack-boundary=2 -fomit-frame-pointer -finput-charset=UTF-8 -fexec-charset=GB2312 -Qn -O0 -w -c .\src\%1*.c -o %1.obj
"../z_tools/OBJ2BIM.exe"  @make.rul  out:%1.bim stack:1024k map:%1.map %1.obj %2 %3 %4 %5 %6 %7 %8 %9
"../z_tools/BIM2HRB.exe" %1.bim %1.hrb 0
copy /b load.bin+%1.hrb %1.bin
