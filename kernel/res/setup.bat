format C
echo Copy files.
C:
copy A:\dosldr.bin dosldr.bin
copy A:\kernel.bin kernel.bin
copy A:\autoexec.bat autoexec.bat
copy A:\creus.bin creus.bin
mkdir other
copy A:\other\font.bin other/font.bin
copy A:\other\hzk16 other/hzk16
echo System is copy OK, Now Copy Apps
mkdir bin
echo Please Insert DISK2.
pause
pause
pause
A:
C:
copy A:\edit.bin bin/edit.bin
copy A:\bitz.bin bin/bitz.bin
copy A:\cale.bin bin/cale.bin
copy A:\copy.bin bin/copy.bin
copy A:\calc.bin bin/calc.bin
copy A:\aigobang.bin bin/aigobang.bin
copy A:\snake.bin bin/snake.bin
copy A:\cgobang.bin bin/cgobang.bin
copy A:\pwsh.bin bin/pwsh.bin
copy A:\pfn.bin bin/pfn.bin
copy A:\RandNum.bin bin/RandNum.bin
copy A:\sort.bin bin/sort.bin
copy A:\bf.bin bin/bf.bin
copy A:\bf.txt bin/bf.txt
copy A:\basic.bin bin/basic.bin
copy A:\lua.bin bin/lua.bin
copy A:\editor.bin bin/editor.bin
echo Please Insert DISK3.
pause
pause
pause
A:
C:
copy A:\ttf.bin bin/ttf.bin
copy A:\font.ttf bin/font.ttf
echo Writting Path.Sys
echo Set Path To: C:\BIN;C:\;
mkfile path.sys
cmdedit path.sys C:\BIN;C:\;
A:
pause
pause
pause
reboot
