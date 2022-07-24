这是我和@min0911_花了8个月写出来的操作系统。
对代码进行了整理分割，规整了许多。
目前功能还不太完善，希望见谅。
系统基本信息：
由GCC和nasm编译而成，由Haribote的工具链接而成，支持执行文件格式为HRB,COFF和ELF。
保护模式，分页内存管理，多任务系统，最低内存32MB。
顾名思义，和DOS系统很接近，基本命令和DOS系统无太大区别。
不过，现在我们实现了基本的图形界面（真彩色）。也区分了图形界面程序和命令行程序。
风格接近Windows2000。
有基本的网络通讯功能（TCP/IP协议栈），但应用层的HTTP协议等还未完善。
chat目录是我们基于UDP实现的聊天器的Windows版的服务器/客户端，在kernel/cmd/chat.c里是本系统版的客户端。
基本命令：
cls dir mkfile mkdir reboot halt jpgview bmp(32)view
tl kill type pause ver time date pcils pcinfo echo poke visit
mem chview beep mouse pak unpak vbetest get_build_info color
font cd casm setup win pcnet chat http socket desktop cpptest
md5s md5f sb16 switch nslookup ping arp ipconfig fork
实现多任务（多控制台）管理，VGA显示模式（VBE高分辨率模式）切换，ACPI电源管理，IDE硬盘读写，1.44MB软盘读写，ASM编译器，
FAT12文件系统管理，程序应用运行，BAT批处理文件运行，BMP JPG图片浏览，中文文档浏览，支持PS/2鼠标，计时器，类GRUB引导多系统，
MD5加密，外部驱动程序，SB16声卡，PCNET网卡驱动及TCP/IP协议栈等功能。
在doc目录里，有对此系统编译的说明以及一些没用的废话。
命令行小程序被砍了很多（软盘装不下），不过你可以修改Makefile。
图形界面应用程序多半是没写完善（比如说俄罗斯方块）。
一个DOS系统没必要进长模式吧（笑）。
BUG向我们反馈：我：2901197932@qq.com min0911_：1474635462@qq.com

因为笔者懒，有很多关于系统的介绍没写出来，所以快下载代码看看吧（笑）。
