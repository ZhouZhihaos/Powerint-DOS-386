@echo off
start gdb -x debug.gdb
qemu-system-x86_64 -serial stdio -device sb16 -net nic,model=pcnet -net user -device floppy -s -S -fda ./img/Powerint_DOS_386.img -hda ./disk.img -boot a