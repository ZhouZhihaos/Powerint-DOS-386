# About Powerint DOS

- Powerint DOS is an operating system used for learning purposes only.
- At first, the operating system was in 16-bit real mode, but now it is in 32-bit protected mode (386 version).
- Due to COVID-19, Zhou Zhihao stayed at home and started the project in December 2020.
- The project was about operating systems and Zhou Zhihao named it 'Powerint,' meaning powerful interrupts that we can use. After coding for about a year, the operating system had the normal functionality like MS-DOS but it was still in 16-bit real mode.
- In December 2021, the writer of Simple OS, Qiu Chenjun, collaborated with Zhou Zhihao. They helped Powerint DOS transition into a new world, 32-bit protected mode, and renamed it Powerint DOS 386.
- After over a year of coding, Powerint DOS 386 is continuously improving.

## Build

**Note: you may need to install nasm, gcc, g++ and qemu before build**

First, you have to clone the repo, like this:

```cmd
git clone https://github.com/ZhouZhihaos/Powerint-DOS-386.git
```

Second, go to the apps folder:

```cmd
cd apps
```

Then, use `make` to compile the apps:

```cmd
make
```

If you don't see an error message, then go to the `Loader` folder then type `make` in the cmd prompt:

```cmd
cd ..
cd Loader
make
```

If you don't see an error message, then you can run flowing commands to go to the `kernel` folder and build the kernel:

```cmd
cd ..
cd kernel
make
```

Or you can add `run` in order to start debug after the compilation:

```cmd
make run
```

You will see Powerint DOS splitted into four images in kernel/img folder.

**Done! You can try Powerint DOS by using qemu or any other virtualization software you like right now!**

## Boot

The following command is an example of qemu:

```cmd
qemu-system-i386 -fda img/Powerint_DOS_386.img
```

If you need network support, add some parameters to the command and it will be like this:

```cmd
qemu-system-i386 -fda img/Powerint_DOS_386.img -net nic,model=pcnet -net user
```

## Developer

- zhouzhihao <https://github.com/ZhouZhihaos>

- min0911_ <https://github.com/min0911Y>

## Thanks

- TheFlySong
- yywd_123
- Oildum-was-ejected
- wenxuanjun
- duoduo70(time.c)
- ...
