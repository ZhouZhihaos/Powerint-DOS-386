#!/bin/bash
qemu-system-x86_64 -net nic,model=pcnet -net user -serial stdio -device sb16 -device floppy -s -S -hda disk.img -hdb ./img/Disk2.img -hdc ./img/Disk3.img -hdd ./img/res.img -boot a -m 128
