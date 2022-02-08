"../z_tools/cc1.exe" -I./include -w  -Os -quiet -o %1.gas src/%1.c
"../z_tools/GAS2NASK.exe" %1.gas %1.nas
"../z_tools/nask.exe" %1.nas %1.obj %1.lst
"../z_tools/OBJ2BIM.exe"  @make.rul  out:%1.bim stack:1024k map:%1.map %1.obj %2 %3 %4 %5 %6
"../z_tools/BIM2HRB.exe" %1.bim %1.hrb 0
copy /b load.bin+%1.hrb %1.bin
