@echo off
start gdb -x debug.gdb
qemu-system-x86_64 -serial stdio -device sb16 -device floppy -s -S -fda ./img/Powerint_DOS_386.img -hda ./disk.img -hdb ./img/Disk2.img -hdc ./img/Disk3.img -boot a