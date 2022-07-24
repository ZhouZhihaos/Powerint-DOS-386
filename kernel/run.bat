@echo off
start gdb -x debug.gdb
qemu-system-x86_64 -serial stdio -device sb16 -netdev user,id=network0 -device pcnet,netdev=network0 -device floppy -s -S -fda ./img/Powerint_DOS_386.img

