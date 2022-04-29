这是我和@min0911_花了5个月写出来的操作系统。
目前功能还不太完善，希望见谅。
最近把多任务和分页（PAE分页）写完了，总算松了口气呢！
系统基本信息：
由GCC和nasm编译而成，由Haribote的工具链接而成，但仅仅支持执行文件格式为HRB和COFF，不支持ELF。
保护模式，分页内存管理，多任务系统，最低内存16MB。
顾名思义，和DOS系统很接近，基本命令和DOS系统无太大区别。
基本命令：
cls dir mkfile mkdir reboot halt jpgview bmp(32)view
tl kill type pause ver time date pcils pcinfo echo poke visit
mem chview beep mouse pak unpak vbetest get_build_info color
font cd casm setup win
实现多任务（多控制台）管理，VGA显示模式（VBE高分辨率模式）切换，ACPI电源管理，IDE硬盘读写，1.44MB软盘读写，ASM编译器，
FAT12文件系统管理，程序应用运行，BAT批处理文件运行，BMP JPG图片浏览，中文文档浏览，支持PS/2鼠标，计时器，类GRUB引导多系统，等功能。
因为软盘读写速度很慢所以有开机界面。
第一次使用会有友好的向导，引导注册用户密码。在注册后输入reboot或halt后存盘再使用本系统，有登入功能。
有类Win95样子的图形界面（摆设），输入win进入后，有一个DOSMODE，鼠标点击返回命令行。
有很多命令行小程序的，在other目录中，有兴趣可以玩玩。
DOC文件夹和SRC/API中有开发手册便于理解代码，不过本身思路就较为简单，所以还是很易懂的。
话说一个DOS系统没必要进长模式吧（笑）。
