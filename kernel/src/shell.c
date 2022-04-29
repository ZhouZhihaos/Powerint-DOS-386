/*
	Shell.c : 用户操作界面的实现
	主要函数：command_run
	command_run : 执行用户输入的命令，此文件一切函数都是为了这个函数做保障
	依赖：vbe.c vga.c task.c other.c screen.c floppy.c
	@author: zhouzhihao & min0911_
	@date: 2022-1-1 Created
-----------0000000000000000000000000000000000----------------------
	Copyright (C) 2020-2022 zhouzhihao & min0911_
*/
//  注意：此文件使用UTF-8编码，请勿使用其他编码打开
//  可能导致乱码，所有注释将无法正常显示
//  *NOTE*: This file uses UTF-8 encoding, please do not use other encoding to open it
// --------------------------------------------------------------
#include "system.h"		   // 引入头文件
int dictaddr = 0x002600;   // 目前目录的地址
char cons_color = 0x0f;	   // 颜色
int change_dict_times = 0; // 目录改变次数
int app_task_num = -1;	   // 应用程序的任务号（-1代表没在运行应用程序）
void command_run(char *cmdline)
{
	//命令解析器
	int addr;
	char c;
	char *p;
	int flag = 0;

	if (cmdline[0] == 0)
	{
		return;
	}
	for (int i = 0; i < strlen(cmdline); i++)
	{
		if (cmdline[i] != ' ' && cmdline[i] != '\n' && cmdline[i] != '\r')
		{
			strcpy(cmdline, cmdline + i);
			goto CHECK_OK;
		}
	}
	//都是空格，直接返回
	return;
CHECK_OK:

	strtoupper(cmdline); //转换成大写
	if (strcmp("WIN", cmdline) == 0)
	{
		//进入图形化环境
		graphic();
		return;
	}
	else if (strcmp("VBETEST", cmdline) == 0)
	{
		//对vBE测试
		int result = check_vbe_mode(MODE_1024X768X256, (struct VBEINFO *)VBEINFO_ADDRESS);
		if (result == -1)
		{
			printf("VBE Error: You Computer does not support VBE mode!\n");
		}
		else
		{
			printf("VBE OK: You Computer supports VBE mode!\n");
			printf("Do you want to test vbe?(Y/N)\n");
			int c = getch();
			if (c == 'Y' || c == 'y')
			{
				struct VBEINFO *vbeinfo = (struct VBEINFO *)VBEINFO_ADDRESS;

				regs16_t r;
				r.ax = 0x4f00;
				r.es = 0x07e0;
				r.di = 0x0000;
				INT(0x10, &r);
				VESAControllerInfo *info = (VESAControllerInfo *)0x07e00;
				printf("VBE OemString: %s\n", rmfarptr2ptr(info->oemString));
				printf("VBE Version: %04x\n", info->Version);
				printf("Video Memory: %d MB\n", info->totalMemory * 64 / 1024);
			}
			else
			{
				printf("\n");
				return;
			}
		}
	}
	else if (strncmp("BMPVIEW32 ", cmdline, 10) == 0)
	{
		struct VBEINFO *vbeinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
		SwitchVBEMode(MODE_1024X768XRGB);
		BMPVIEW32(cmdline + 10, vbeinfo->vram, 1024);
		getch();
		SwitchToText8025_BIOS();
		Set_Font("A:\\other\\font.bin");
		clear();
		return;
	}else if(strcmp("GET_BUILD_INFO",cmdline)==0)
	{
		printf("Build Time: %s %s\n",__DATE__,__TIME__);
		return;
	}
	else if (strncmp("JPGVIEW ", cmdline, 8) == 0)
	{
		struct VBEINFO *vbeinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
		SwitchVBEMode(MODE_1024X768XRGB);
		jpgview32(cmdline + 8, vbeinfo->vram, 1024);
		getch();
		SwitchToText8025_BIOS();
		Set_Font("A:\\other\\font.bin");
		clear();
		return;
	}
	else if (strcmp("DIR", cmdline) == 0)
	{
		cmd_dir();
		return;
	}
	else if (strcmp("TL", cmdline) == 0)
	{
		cmd_tl();
		return;
	}
	else if (strncmp("KILL ", cmdline, 5) == 0)
	{
		cmdline += 5;
		for (int i = 0; GetTask(i) != 0; i++)
		{
			if (strtol(cmdline, NULL, 10) == i)
			{
				if (Get_Running_Task_Num() == 1)
				{
					printf("Cannot kill the last task.\n");
					return;
				}
				if (GetTask(i)->level == 1)
				{
					printf("Cannot kill the system task.\n");
					return;
				}
				SubTask(GetTask(i));
				return;
			}
		}
		printf("No such task.\n");
		return;
	}
	else if (strncmp("TYPE ", cmdline, 5) == 0)
	{
		type_deal(cmdline);
		return;
	}
	else if (strcmp("CLS", cmdline) == 0)
	{

		clear();
		return;
	}
	else if (strcmp("PAUSE", cmdline) == 0)
	{
		printf("Press any key to continue. . .");
		getch();
		printf("\n");
	}
	else if (strcmp("VER", cmdline) == 0)
	{
		printf("Powerint DOS 386 Version %s\n", VERSION);
		print("Copyright (C) 2021-2022 zhouzhihao & min0911\n");
		print("THANKS Link TOOLS BY Kawai\n\n");
		printf("C Build tools by GNU C Compiler\n");
		printf("ASM Build tools by NASM\n\n");
		printf("I love you Kawai\n");
		return;
	}
	else if (strcmp("TIME", cmdline) == 0)
	{
		char *time = "The current time is:00:00:00";
		io_out8(0x70, 0);
		c = io_in8(0x71);
		time[27] = (c & 0x0f) + 0x30;
		time[26] = (c >> 4) + 0x30;
		io_out8(0x70, 2);
		c = io_in8(0x71);
		time[24] = (c & 0x0f) + 0x30;
		time[23] = (c >> 4) + 0x30;
		io_out8(0x70, 4);
		c = io_in8(0x71);
		time[21] = (c & 0x0f) + 0x30;
		time[20] = (c >> 4) + 0x30;
		print(time);
		print("\n\n");
		return;
	}
	else if (strcmp("DATE", cmdline) == 0)
	{
		char *date = "The current date is:2000\\00\\00,";
		io_out8(0x70, 9);
		c = io_in8(0x71);
		date[23] = (c & 0x0f) + 0x30;
		date[22] = (c >> 4) + 0x30;
		io_out8(0x70, 8);
		c = io_in8(0x71);
		date[26] = (c & 0x0f) + 0x30;
		date[25] = (c >> 4) + 0x30;
		io_out8(0x70, 7);
		c = io_in8(0x71);
		date[29] = (c & 0x0f) + 0x30;
		date[28] = (c >> 4) + 0x30;
		print(date);
		io_out8(0x70, 6);
		c = io_in8(0x71);
		if (c == 1)
			print("Sunday");
		if (c == 2)
			print("Monday");
		if (c == 3)
			print("Tuesday");
		if (c == 4)
			print("Wednesday");
		if (c == 5)
			print("Thursday");
		if (c == 6)
			print("Friday");
		if (c == 7)
			print("Saturday");
		print("\n\n");
		return;
	}
	else if (strcmp("PCILS", cmdline) == 0)
	{
		pci_list();
	}
	else if (strncmp("ECHO ", cmdline, 5) == 0)
	{
		print(cmdline + 5);
		print("\n");
		return;
	}
	else if (strncmp("MKDIR ", cmdline, 6) == 0)
	{
		if (change_dict_times == 0)
		{
			mkdir(cmdline + 6, 0);
		}
		else
		{
			struct FILEINFO *finfo = dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
			mkdir(cmdline + 6, finfo->clustno);
		}
	}
	else if (strncmp("POKE ", cmdline, 5) == 0)
	{
		addr = ascii2num(cmdline[5]) * 0x10000000 + ascii2num(cmdline[6]) * 0x1000000;
		addr = addr + ascii2num(cmdline[7]) * 0x100000 + ascii2num(cmdline[8]) * 0x10000;
		addr = addr + ascii2num(cmdline[9]) * 0x1000 + ascii2num(cmdline[10]) * 0x100;
		addr = addr + ascii2num(cmdline[11]) * 0x10 + ascii2num(cmdline[12]);
		p = addr;
		c = ascii2num(cmdline[14]) * 0x10 + ascii2num(cmdline[15]);
		p[0] = c;
		print("\n");
		return;
	}
	else if (strncmp("VISIT ", cmdline, 6) == 0)
	{
		addr = ascii2num(cmdline[6]) * 0x10000000 + ascii2num(cmdline[7]) * 0x1000000;
		addr = addr + ascii2num(cmdline[8]) * 0x100000 + ascii2num(cmdline[9]) * 0x10000;
		addr = addr + ascii2num(cmdline[10]) * 0x1000 + ascii2num(cmdline[11]) * 0x100;
		addr = addr + ascii2num(cmdline[12]) * 0x10 + ascii2num(cmdline[13]);
		p = addr;
		c = p[0];
		printchar(num2ascii(c >> 4));
		printchar(num2ascii(c & 0x0f));
		print("\n");
		return;
	}
	else if (strcmp("PCINFO", cmdline) == 0)
	{
		pcinfo();
	}
	else if (strcmp("MEM", cmdline) == 0)
	{
		mem();
	}
	else if (strncmp("BMPVIEW ", cmdline, 8) == 0)
	{
		bmpview(cmdline + 8);
		char c;
		for (;;)
		{
			c = input_char_inSM();
			if (c == 0x01)
			{
				SwitchToText8025_BIOS();
				Set_Font("A:\\other\\font.bin");
				break;
			}
		}
		return;
	}
	else if (strncmp("CHVIEW ", cmdline, 7) == 0)
	{
		chview(cmdline + 7);
	}
	else if (strncmp("BEEP ", cmdline, 5) == 0)
	{
		int point, notes, dup;
		point = ascii2num(*(char *)(cmdline + 5));
		notes = ascii2num(*(char *)(cmdline + 7));
		dup = ascii2num(*(char *)(cmdline + 9));
		beep(point, notes, dup);
	}
	else if (strcmp("MOUSE", cmdline) == 0)
	{
		struct FIFO8 *kfifo;
		struct FIFO8 *mfifo;
		char *kbuf;
		char *mbuf;
		unsigned char *stack;
		if (flag == 0)
		{
			struct TASK *mouse_task;
			stack = page_kmalloc(64 * 1024);
			mouse_task = AddTask("Mouse", 2, 2 * 8, (int)mouseinput, 1 * 8, 1 * 8,stack + 64 * 1024);
			kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
			mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
			kbuf = (char *)page_kmalloc(32);
			mbuf = (char *)page_kmalloc(128);
			fifo8_init(kfifo,32,kbuf);
			fifo8_init(mfifo,128,mbuf);
			TaskSetFIFO(mouse_task,kfifo,mfifo);
			flag = 1;
		}
		else
		{
			page_kfree(stack,64 * 1024);
			page_kfree(kfifo,sizeof(struct FIFO8));
			page_kfree(mfifo,sizeof(struct FIFO8));
			page_kfree(kbuf,32);
			page_kfree(mbuf,128);
			SubTask(GetTaskForName("Mouse"));
			flag = 0;
		}
	}
	else if (strcmp("REBOOT", cmdline) == 0)
	{
		save_all_floppy();
		io_out8(0xcf9, 0x0e);
	}
	else if (strcmp("HALT", cmdline) == 0)
	{
		// 先保存，再调用ACPI进行关机
		save_all_floppy();
		acpi_shutdown();
	}
	else if (strncmp("PAK ", cmdline, 4) == 0)
	{
		int i;
		for (i = 4; cmdline[i] != ' '; i++)
			;
		pak(cmdline + 4, cmdline + i + 1);
	}
	else if (strncmp("UNPAK ", cmdline, 6) == 0)
	{
		unpak(cmdline + 6);
	}
	else if (strncmp("COLOR ", cmdline, 6) == 0)
	{
		cons_color = (ascii2num(cmdline[6]) << 4) + ascii2num(cmdline[7]);
		int i;
		for (i = 0; i != 160 * 25; i += 2)
		{
			*(char *)(0xb8000 + i + 1) = cons_color;
		}
	}
	else if (strcmp("CASM", cmdline) == 0)
	{
		casm_shell();
	}
	else if (strncmp("MKFILE ", cmdline, 7) == 0)
	{
		mkfile(cmdline + 7);
		return;
	}
	else if (strncmp("DEL ", cmdline, 4) == 0)
	{
		del(cmdline);
		return;
	}
	// else if (strncmp("EDIT ", cmdline, 5) == 0)
	// {
	// 	edit(cmdline);
	// 	return;
	// }
	else if (strncmp("FONT ", cmdline, 5) == 0)
	{
		Set_Font(cmdline + 5);
	}
	else if (strncmp("CD ", cmdline, 3) == 0)
	{
		changedict(cmdline + 3);
	}
	else if (*(char *)(0x7dfd) == 'A')
	{
		if (strcmp("SETUP", cmdline) == 0)
		{
			setup();
			return;
		}
		else if (cmd_app(cmdline) == 0)
		{
			if (run_bat(cmdline) == 0)
			{
				print("Bad Command!\n\n");
				return;
			}
		}
	}
	else
	{
		if (cmd_app(cmdline) == 0)
		{
			if (run_bat(cmdline) == 0)
			{
				print("Bad Command!\n\n");
				return;
			}
		}
	}
}

void print_date(unsigned short _date, unsigned short _time)
{
	//打印日期
	unsigned short year = _date & 65024;
	year = year >> 9;
	unsigned short month = _date & 480;
	month = month >> 5;
	unsigned short day = _date & 31;

	unsigned short hour = _time & 63488;
	hour = hour >> 11;
	unsigned short minute = _time & 2016;
	minute = minute >> 5;
	printf("%04d-%02d-%02d %02d:%02d", (year + 1980), month, day, hour, minute);
}
void cmd_dir()
{
	// DIR命令的实现
	struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + dictaddr);
	extern int MaxLine;
	int i, j, k, line = 0;
	int FileSize = 0; //所有文件的大小
	char s[30];
	for (i = 0; i != 30; i++)
	{
		s[i] = 0;
	}
	print("FILENAME   EXT    LENGTH       TYPE   DATE\n");
	for (i = 0; i < 224; i++, line++)
	{
		if (line == MaxLine)
		{
			print("Press any key to continue...");
			getch();
			print("\n");
			line = 0;
		}
		if (finfo[i].name[0] == 0x00)
		{
			break;
		}
		if (finfo[i].name[0] != 0xe5)
		{
			if ((finfo[i].type & 0x18) == 0 || finfo[i].type == 0x10)
			{

				for (j = 0; j < 8; j++)
				{
					s[j] = finfo[i].name[j];
				}
				s[9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];

				if (s[0] != '+')
				{
					for (k = 0; k < 12; ++k)
					{
						if (k == 9)
						{
							print("   ");
						}
						if (s[k] == '\n')
						{
							print("   ");
						}
						else
						{

							putchar(s[k]);
						}
					}
					print("    ");
					print(itoa(finfo[i].size));
					gotoxy(31, get_y());
					if ((finfo[i].type & 0x18) == 0)
					{
						print("<FILE> ");
					}
					if (finfo[i].type == 0x10)
					{
						print("<DIR>  ");
					}
					print_date(finfo[i].date, finfo[i].time);
					print("\n");
					FileSize += finfo[i].size;
				}
			}
		}
	}
	print("\n");
	printf("    %d byte(s)\n",FileSize);
	printf("    %d byte(s) free\n",2880*512-FileSize);
	//&s = 0;
	return;
}
void type_deal(char *cmdline)
{
	// type命令的实现
	char *name;
	int i;
	struct FILEINFO *finfo;
	for (i = 0; i < strlen(cmdline); i++)
	{
		name[i] = cmdline[i + 5];
	}
	finfo = Get_File_Address(name);
	if (finfo == 0)
	{
		print(name);
		print(" not found!\n\n");
	}
	else
	{
		char *p = fopen(name);
		for (i = 0; i != finfo->size; i++)
		{
			printchar(p[i]);
		}
		print("\n");
	}
	return;
}

void edit(char *cmdline)
{
	char *name;
	int i;
	struct FILEINFO *finfo;
	for (i = 0; i < strlen(cmdline); i++)
	{
		name[i] = cmdline[i + 5];
	}
	finfo = Get_File_Address(name);
	if (finfo == 0)
	{
		print(name);
		print(" not found!\n\n");
		return;
	}
	finfo->size = 0;
	char *p = fopen(name);
	for (;; finfo->size++)
	{
		p[finfo->size] = getch();
		printchar(p[finfo->size]);
		if (p[finfo->size] == 0x0a)
		{
			break;
		}
	}
	int size = finfo->size;
	int Bmp = dictaddr;
	char BMP_PATH[224 * 13 + 1];
	for (i = 0; i < 224 * 13; i++)
	{
		BMP_PATH[i] = 0;
	}
	strcpy(BMP_PATH, path);
	while (dictaddr != 0x2600)
	{
		if (dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224) != 0)
		{
			struct FILEINFO *finfo_this_dict_clust = dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
			struct FILEINFO *finfo_this_dict = clust_sech(finfo_this_dict_clust->clustno, (struct FILEINFO *)(ADR_DISKIMG + Get_dictaddr("../")), 224);
			finfo_this_dict->size += size;
		}
		changedict("..");
	}
	dictaddr = Bmp;
	strcpy(path, BMP_PATH); //杩樺師path
	return;
}

void bmpview(char *filename)
{
	int i, j;
	char *p = fopen(filename);
	unsigned short pxsize, pysize, start;
	unsigned int length;
	if (p == 0)
	{
		print("Can't find file ");
		print(filename);
		print("\n");
		return;
	}
	if (p[0] != 'B' || p[1] != 'M')
	{
		print("Isn't BMP photo,format error.\n");
		return;
	}
	pxsize = *(unsigned short *)(p + 0x12);
	pysize = *(unsigned short *)(p + 0x16);
	length = *(unsigned int *)(p + 2);
	start = *(unsigned short *)(p + 0xa);
	SwitchVBEMode(MODE_1024X768X256);
	io_out8(VGA_DAC_WRITE_INDEX, 0);
	for (i = 0; i != 256; i++)
	{
		io_out8(VGA_DAC_DATA, p[0x36 + i * 4 + 2] / 4);
		io_out8(VGA_DAC_DATA, p[0x36 + i * 4 + 1] / 4);
		io_out8(VGA_DAC_DATA, p[0x36 + i * 4] / 4);
	}
	struct VBEINFO *vinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
	unsigned char *vram = vinfo->vram;
	for (i = 0; i < pysize; i++)
	{
		for (j = 0; j < pxsize; j++)
		{
			// Draw_Px(j, i, p[length - (i * pxsize + pxsize - j)]);
			vram[i * 1024 + j] = p[length - (i * pxsize + pxsize - j)];
		}
	}
	return;
}
void chview(char *filename)
{
	struct FILEINFO *finfo = Get_File_Address(filename);
	char *p = fopen(filename);
	if (p == 0)
	{
		print("Can't find file ");
		print(filename);
		print("\n");
		return;
	}
	SwitchToHighTextMode(); //切换到高分辨率文本模式
	HighPutStr(p, finfo->size);
	getch();
	Close_High_Text_Mode();
	SwitchToText8025_BIOS();
	Set_Font("A:\\other\\font.bin");
	return;
}

void pcinfo()
{
	char cpu[100] = {0};
	int cpuid[3] = {get_cpu1(), get_cpu3(), get_cpu2()};
	//根据CPUID信息打印出来
	cpu[0] = cpuid[0] & 0xff;
	cpu[1] = (cpuid[0] >> 8) & 0xff;
	cpu[2] = (cpuid[0] >> 16) & 0xff;
	cpu[3] = (cpuid[0] >> 24) & 0xff;
	cpu[4] = cpuid[1] & 0xff;
	cpu[5] = (cpuid[1] >> 8) & 0xff;
	cpu[6] = (cpuid[1] >> 16) & 0xff;
	cpu[7] = (cpuid[1] >> 24) & 0xff;
	cpu[8] = cpuid[2] & 0xff;
	cpu[9] = (cpuid[2] >> 8) & 0xff;
	cpu[10] = (cpuid[2] >> 16) & 0xff;
	cpu[11] = (cpuid[2] >> 24) & 0xff;
	cpu[12] = 0;
	printf("CPU:%s ", cpu);
	char cpu1[100] = {0};
	getCPUBrand(cpu1);
	printf("Ram Size:%dMB\n", get_memsize() / (1024 * 1024));
	return;
}
void mem()
{
	int free = 0;
	for (int i = 0; i != 1024*1024; i++) {
		extern char *pages;
		if (pages[i] == 0)
			free++;
	}
	printf("free vpages:%d free kpages:%d\nfree:%dKB\n",free,free-(1024*1024-get_memsize()/(4*1024)),(free-(1024*1024-get_memsize()/(4*1024)))*4);
	return;
}

char get_cons_color()
{
	//获取颜色
	return cons_color;
}

int run_bat(char *cmdline)
{
	// 运行批处理文件
	struct FILEINFO *finfo;
	char *file;
	int i, j = 0;
	char name[18] = {0};
	char *file1 = page_kmalloc(1024);
	for (i = 0; i < 13; i++)
	{
		if (cmdline[i] <= ' ')
		{
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0;
	finfo = Get_File_Address(name);
	file = fopen(name);
	if (finfo == 0)
	{
		if (finfo == 0 && name[i - 1] != '.')
		{
			/*没有填写后缀，加上.BAT再次查找*/
			name[i] = '.';
			name[i + 1] = 'B';
			name[i + 2] = 'A';
			name[i + 3] = 'T';
			name[i + 4] = 0;
			finfo = Get_File_Address(name);
			file = fopen(name);
			if (finfo == 0)
			{
				page_kfree(file1,1024);
				return 0;
			}
			goto ok;
		}
		page_kfree(file1,1024);
		return 0;
	}
ok:
	//读取每行的内容，然后调用命令解析函数（command_run）
	for (i = 0; i != finfo->size; i++)
	{
		if (file[i] == 0x0d && file[i + 1] == 0x0a)
		{
			command_run(file1);
			i++;
			j = 0;
			int p;
			clean(file1,1024);
			continue;
		}
		file1[j] = file[i];
		j++;
	}
	int p;
	command_run(file1);
	page_kfree(file1,1024);
	return 1;
}
int cmd_app(char *cmdline)
{
	// cmd_app：启动程序
	// cmdline:命令行
	extern int pressed; //为kbhit系统API做准备
	pressed = 0;		// pressed变量：键盘是否被按下
				 //这里设置为0：初始化为未按下
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	char *name, *p, *q, *alloc; // name:文件名，p:代码段，q:数据段，alloc:分配的内存
	char *stack;				//堆栈
	struct TASK *app_task;		//任务
	int i, segsiz, datsiz, dathrb, esp;
	name = (char *)page_kmalloc(300); //分配300字节内存
	clean(name, 300);							 //清空内存(为了避免脏数据，这里全部填充为0)
	for (i = 0; i < strlen(cmdline); i++)
	{
		if (cmdline[i] == ' ')
		{
			//如果碰到空格（参数部分），就说明文件名的部分结束了，这时候我们直接跳出循环即可
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0; //置字符串结束符0
				 //虽然前面已经全部填充为0了，这里为了保险，还是重新加上字符串结束符（'\0'）
	finfo = Get_File_Address(name); //获取文件所在的地址，保存为FILEINFO数据结构，以便我们获取信息
	if (finfo == 0)					//没找到这个文件
	{
		//加上后缀再试一遍
		name = strcat(name, ".BIN");
		finfo = Get_File_Address(name);
	}
	if (finfo != 0)
	{
		for (i = 0; i != 1024; i++)
		{
			pro_memman[i] = 0x00; //首先初始化为程序提前分配的内存
		}
		// 代码段的物理内存必须是连续的
		if ((fopen(name))[0] == 'A') // A=ASM,这是汇编语言编写的程序（不分代码段和数据段，所以我们将他们两个分开，以便我们编写代码）
		{
			p = (char *)page_kmalloc(finfo->size + 128 * 1024); //分配程序的代码段（由于这里是汇编语言的程序，所以在代码段后面要加上128KB的待分配内存）
			// 因为只有代码段，所以cs_base和ds_base是一样的
			*((int *)(0xfe8)) = p;										  // cs_base
			*((int *)(0xfec)) = p;										  // ds_base
			*((int *)(0xff0)) = p + finfo->size;						  // 我也不知道为啥要写这句，自己看着理解吧2333
			memcpy(p, fopen(name), finfo->size);						  // 将文件拷贝到代码段中（由于是只有一个段的程序，所以直接无脑拷贝即可）
			set_segmdesc(gdt + 3, finfo->size - 1, (int)p, AR_CODE32_ER); //设置他们的段描述符
			set_segmdesc(gdt + 4, finfo->size - 1 + 128 * 1024, (int)p, AR_DATA32_RW + 0x60);
			stack = page_vmalloc(64 * 1024) + 64 * 1024; //为程序的stack分配空间
			io_cli();
			struct TASK *this_task = AddTask(name, 2, 3 * 8, 1, 4 * 8, 1 * 8, stack);
			char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
			char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
			char *kbuf = (char *)page_kmalloc(32);
			char *mbuf = (char *)page_kmalloc(128);
			fifo8_init(kfifo,32,kbuf);
			fifo8_init(mfifo,128,mbuf);
			TaskSetFIFO(this_task,kfifo,mfifo);
			app_task_num = this_task->sel / 8 - 103;
			SleepTaskFIFO(GetTaskForName("Shell"));
			io_sti();
			while(GetTaskForName(name) != 0);
			// 下半部会帮我们结束程序
			//SubTask(GetTaskForName(name)); //程序退出咯，我们来把它杀掉吧【doge】
			WakeUp(GetTaskForName("Shell"));
			app_task_num = -1;
			page_kfree(kfifo,sizeof(struct FIFO8));
			page_kfree(mfifo,sizeof(struct FIFO8));
			page_kfree(kbuf,32);
			page_kfree(mbuf,128);
			page_kfree((int)p, finfo->size + 128 * 1024);
			page_vfree(stack-64*1024, 64*1024);
			print("\n");
		}
		else if ((fopen(name))[0] == 'C') // C = C语言程序，也代码拥有两个段的程序（代码段和数据段）
		{								  // C
			/*
				有些代码的逻辑和运行汇编语言程序是一样的，
				这里只注释新的代码
			*/
			p = (char *)page_kmalloc(finfo->size);
			*((int *)(0xfe8)) = p; // cs_base
			memcpy(p, fopen(name), finfo->size);
			// hrb文件头（是的，就是那个《30天》的文件头，挺简单的）
			// hrb文件头的结构是：
			/**
			 * 偏移地址       描述
			 *  0x14         段大小
			 *  0x0c         esp与数据存储的地址
			 *  0x10         数据大小
			 *  0x14         数据存储的地址
			 */
			// 因为前面有一个汇编头来引导，所以要加上26
			// 文件开头有一个'C'，所以要加上1
			segsiz = *((int *)(p + 0x0000 + 26 + 1));
			esp = *((int *)(p + 0x000c + 26 + 1));
			datsiz = *((int *)(p + 0x0010 + 26 + 1));
			dathrb = *((int *)(p + 0x0014 + 26 + 1));
			q = (char *)page_vmalloc(segsiz + 128 * 1024); //分配数据段的内存
			*((int *)(0xfec)) = q;									  // ds_base
			*((int *)(0xff0)) = q + segsiz;
			set_segmdesc(gdt + 3, finfo->size - 1, (int)p, AR_CODE32_ER);
			set_segmdesc(gdt + 4, segsiz - 1 + 128 * 1024, (int)q, AR_DATA32_RW + 0x60);
			for (i = 0; i < datsiz; i++)
			{
				q[esp + i] = p[dathrb + i + 26 + 1]; //这里通过头数据拷贝数据段数据
			}
			stack = page_vmalloc(64 * 1024) + 64 * 1024;
			io_cli();
			char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
			char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
			char *kbuf = (char *)page_kmalloc(32);
			char *mbuf = (char *)page_kmalloc(128);
			//printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
			//printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
			//printk("p:%08x\n",p);
			//printk("q:%08x\n",q);
			//printk("stack:%08x\n",stack);
			struct TASK *this_task = AddTask(name, 2, 3 * 8, 1, 4 * 8, 1 * 8, stack);
			fifo8_init(kfifo,32,kbuf);
			fifo8_init(mfifo,128,mbuf);
			TaskSetFIFO(this_task,kfifo,mfifo);
			app_task_num = this_task->sel / 8 - 103;
			SleepTaskFIFO(GetTaskForName("Shell"));
			io_sti();
			while(GetTaskForName(name) != 0);
			//SubTask(GetTaskForName(name));
			WakeUp(GetTaskForName("Shell"));
			app_task_num = -1;
			//printk("kfifo:%08x,mfifo:%08x\n",kfifo,mfifo);
			//printk("kbuf:%08x,mbuf:%08x\n",kbuf,mbuf);
			//printk("p:%08x\n",p);
			//printk("q:%08x\n",q);
			//printk("stack:%08x\n",stack);
			page_kfree(kfifo,sizeof(struct FIFO8));
			page_kfree(mfifo,sizeof(struct FIFO8));
			page_kfree(kbuf,32);
			page_kfree(mbuf,128);
			page_kfree((int)p, finfo->size);
			page_vfree((int)q, segsiz + 128 * 1024);
			page_vfree(stack-64*1024, 64*1024);
			print("\n");
		}
		else
		{
			// 未知的文件类型
			print("Isn't Powerint DOS 386 Execute File.\n\n");
		}
		page_kfree((int)name, 300); //将name字符指针所占用的内存释放
		return 1;
	}
	page_kfree((int)name, 300); //将name字符指针所占用的内存释放
	return 0;								//找不到文件
}
void mouseinput()
{
	char mousecopy[80];
	int i, mx1 = mx, my1 = my, bufx = mx * 8, bufy = my * 16;
	mouse_ready(&mdec);
	while (1)
	{
		if (fifo8_status(TaskGetMousefifo(NowTask())) == 0)
		{
			io_stihlt();
		}
		else
		{
			*(char *)(0xb8000 + my1 * 160 + mx1 * 2 + 1) = 0x07;
			*(char *)(0xb8000 + my * 160 + mx * 2 + 1) = 0x70;
			i = fifo8_get(TaskGetMousefifo(NowTask()));
			// io_sti();
			if (mouse_decode(&mdec, i) != 0)
			{
				mx1 = mx; // 更新mx1,my1
				my1 = my;
				bufx += mdec.x;
				bufy += mdec.y;
				mx = bufx / 8;	// 计算位置
				my = bufy / 16; // 计算位置
				if (bufx > 79 * 8)
				{
					bufx = 79 * 8;
				}
				else if (bufx < 0)
				{
					bufx = 0;
				}
				if (bufy > 24 * 16)
				{
					bufy = 24 * 16;
				}
				else if (bufy < 0)
				{
					bufy = 0;
				}
			}
		}
	}
	mouse_sleep(&mdec); //让鼠标暂时停止不向FIFO缓冲区写入数据
	mx = mx1;
	my = my1;
	return;
}

void cmd_tl()
{
	// tl：tasklist
	// 显示当前运行的任务
	extern int tasknum; //任务数量（定义在task.c）
	for (int i = 0; i != tasknum + 1; i++)
	{
		printf("Task %d: Name:%s,Level:%d,Sleep:%d,GDT address:%d*8\n", i, GetTask(i)->name, GetTask(i)->level, GetTask(i)->sleep, GetTask(i)->sel / 8);
	}
}
void save_all_floppy()
{
	// save_all_floppy：保存所有磁盘
	int t = 0;						  //磁道
	int h = 0;						  //磁头
	int s = 1;						  //扇区
	char *data = (char *)ADR_DISKIMG; //磁盘数据储存在的内存地址
	char *ing = "[          ]";		  //进度条
	for (; t < 80; t++)				  //软盘一共80个磁道
	{
		// 将光标xy设置为本行的开头
		gotoxy(0, get_y());
		printf("%s %d%%", ing, t * 5 / 4); // 显示进度条 以及百分比
		gotoxy(1, get_y());				   // 将光标xy设置为本行的开头第一个位置
		for (int i = 0; i != t * 5 / 40; i++)
		{
			printchar('#'); // 显示进度条
		}
		//文件的保存
		for (; h < 2; h++)
		{
			for (; s < 19; s++)
			{
				write_floppy_for_ths(t, h, s, data, 1); //写入磁盘
				data += 512;							// 指向下一个扇区
			}
			s = 1; //重定位S
		}
		h = 0; //重定位H
	}
}
void read_floppy_disk_all()
{
	unsigned char *buf = (unsigned char *)page_kmalloc(2880 * 512);
	int buf2 = buf;
	int t = 0;						  //磁道
	int h = 0;						  //磁头
	int s = 1;						  //扇区

	char *ing = "[          ]";		  //进度条
	for (; t < 80; t++)				  //软盘一共80个磁道
	{
		// 将光标xy设置为本行的开头
#ifdef __READ__ALL__
		gotoxy(0, get_y());
		printf("%s %d%%", ing, t * 5 / 4); // 显示进度条 以及百分比
		gotoxy(1, get_y());				   // 将光标xy设置为本行的开头第一个位置
		for (int i = 0; i != t * 5 / 40; i++)
		{
			printchar('#'); // 显示进度条
		}
#endif
#ifdef GRAPHICS_LOADING_UI
		extern int dblock;
		dblock = (t * 7 / 40);
#endif
		//软盘的读取
		for (; h < 2; h++)
		{
			for (; s < 19; s++)
			{
				if (fdc_rw_ths(t, h, s, buf, 1, 1) != 1)
				{
#ifdef GRAPHICS_LOADING_UI
					SwitchToText8025_BIOS();
#endif
					Panic_F("[ !!!!!! ] Read Error !!! ");
					Panic_K("[ !!!!!! ] Read Error !!! ");
					for (;;)
						;
				}
#ifdef __READ__ALL__
				gotoxy(strlen(ing) + 5, get_y());
				printf("T:%02d H:%02d S:%02d ADDRESS: %08x", t, h, s, buf);
#endif
				buf += 512; // 指向下一个扇区
			}
			s = 1; //重定位S
		}
		h = 0; //重定位H
	}
	extern int ADR_DISKIMG;
	ADR_DISKIMG = buf2;
	// 装载字库
    extern char *font;
    font = fopen("A:\\other\\font.bin");
    extern char *ascfont;
    ascfont = fopen("A:\\other\\font.bin");
    extern char *hzkfont;
    hzkfont = fopen("A:\\other\\hzk16");
}
