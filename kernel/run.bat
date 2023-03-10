@echo off
start gdb -x debug.gdb
qemu-system-x86_64 -serial stdio -device sb16 -device floppy -s -S -fda ./img/Powerint_DOS_386.img -hda ./img/disk2.img -hdb ./img/res.img -hdc disk.img -boot a