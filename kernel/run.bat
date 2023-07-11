@echo off
start gdb -x debug.gdb
qemu-system-x86_64 -serial stdio -device sb16 -device floppy -s -S -fda ./img/Powerint_DOS_386.img -drive id=disk,file=disk.img,if=none -device ahci,id=ahci -device ide-hd,drive=disk,bus=ahci.0 -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/res.img -boot a -m 2048