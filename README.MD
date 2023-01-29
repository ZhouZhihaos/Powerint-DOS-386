# About Powerint DOS
+ Powerint DOS is an operating system used for learning purposes only.
- At first, the operating system was in 16-bit real mode, but now it is in 32-bit protected mode (386 version).
- Due to COVID-19, Zhou Zhihao stayed at home and started the project in December 2020.
- The project was about operating systems and Zhou Zhihao named it 'Powerint,' meaning powerful interrupts that we can use. After coding for about a year, the operating system had the normal functionality like MS-DOS but it was still in 16-bit real mode.
- In December 2021, the writer of Simple OS, Qiu Chenjun, collaborated with Zhou Zhihao. They helped Powerint DOS transition into a new world, 32-bit protected mode, and renamed it Powerint DOS 386.
- After over a year of coding, Powerint DOS 386 is continuously improving.
# How to Build?
### Warning: Powerint DOS can only be built on Windows at the moment! You must have TDM-GCC & QEMU installed. Are you all set? Let's do it!
+ First, you must clone the repo, like this:
```cmd
git clone https://github.com/ZhouZhihaos/Powerint-DOS-386.git
```
+ Second step: go to the apps folder
```cmd
cd apps
```

+ Then, type "make" in the cmd prompt:

```cmd
make
```
+ If you don't see an error message, proceed to the next step. Goto the Loader folder then type "make" in the cmd prompt:

```cmd
cd ..
cd Loader
make
```

+ If you don't see an error message, proceed to the next step. Go to the Kernel folder, then make a choice: either make the kernel and create an image, or make apps and kernel and create an image.
### First choice
```cmd
cd ..
cd kernel
make img_run
```
You will see four images in kernel/img folder


### Second choice
```cmd
cd ..
cd kernel
make img_run
```
You will see four images in kernel/img folder

+ Done!

# developer

- zhouzhihao ([ZhouZhihaos (github.com)](https://github.com/ZhouZhihaos))

- min0911_ ([min0911Y (github.com)](https://github.com/min0911Y))

# Thanks

- theflysong
- yywd_123
- Oildum-was-ejected
- ......
