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
copy A:\other\edit.bin bin/edit.bin
copy A:\other\bitz.bin bin/bitz.bin
copy A:\winver.bin bin/winver.bin
copy A:\uicalc.bin bin/uicalc.bin
copy A:\other\cale.bin bin/cale.bin
copy A:\other\copy.bin bin/copy.bin
copy A:\other\calc.bin bin/calc.bin
echo Please Insert DISK2.
pause
pause
pause
A:
C:
copy A:\uigobang.bin bin/uigobang.bin
copy A:\aigobang.bin bin/aigobang.bin
copy A:\snake.bin bin/snake.bin
copy A:\gobang.bin bin/gobang.bin
copy A:\cgobang.bin bin/cgobang.bin
copy A:\game.bin bin/game.bin
copy A:\login.bin bin/login.bin
copy A:\pwsh.bin bin/pwsh.bin
copy A:\pfn.bin bin/pfn.bin
copy A:\RandNum.bin bin/RandNum.bin
copy A:\sort.bin bin/sort.bin
copy A:\student.bin bin/student.bin
copy A:\bf.bin bin/bf.bin
copy A:\bf.txt bin/bf.txt
echo Writting Path.Sys
echo Set Path To: C:\BIN;C:\;
mkfile path.sys
cmdedit path.sys C:\BIN;C:\;
A:
pause
pause
pause
reboot