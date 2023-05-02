@echo off
start gdb -x debug.gdb
qemu-system-x86_64 -serial stdio -device sb16 -device floppy -s -S -fda ./img/Powerint_DOS_386.img -hda ./d.img -hdb ./img/Disk2.img -boot a