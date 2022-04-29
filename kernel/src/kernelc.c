// Powerint DOS 386
// Copyright (C) 2021-2022 zhouzhihao & min0911
// -------------------------------------------------------------
#include "system.h"
/*globals*/
char keytable[0x54] = {
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 10, 0, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, '7', 'D', '8', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'};
char keytable1[0x54] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 10, 0, 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
	'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.'};
char mouse_cur_graphic[16][16] = {
	"*...............",
	"**..............",
	"*O*.............",
	"*OO*............",
	"*OOO*...........",
	"*OOOO*..........",
	"*OOOOO*.........",
	"*OOOOOO*........",
	"*OOOOOOO*.......",
	"*OOOO*****......",
	"*OO*O*..........",
	"*O*.*O*.........",
	"**..*O*.........",
	"*....*O*........",
	".....*O*........",
	"......*........."};
struct MOUSE_DEC mdec;
struct timer Timer;
int caps_lock = 0, shift = 0;
unsigned int memsize;
int mx = 39, my = 12;
char *line;
char *path;
struct tty tty_console[6];
int PCI_ADDR_BASE;
int pressed = 0; // 按键是否按下
char pro_memman[1024] = {0}; // 1bit = 16bytes
int ADR_DISKIMG = 0x00100000;
int autoexecflag = 0;
#ifdef GRAPHICS_LOADING_UI
int readallflag = 1;
int dblock = 0;
void ReadTask()
{
	read_floppy_disk_all();
	readallflag = 0;
	while (1);
}
#endif
void shell(void)
{
	clear();
	int i;
	path = (char *)page_kmalloc(256);
	clean(path, 256);
	strcpy(path, "");
	line = (char *)page_kmalloc(1024);
	if (autoexecflag == 1)
		goto whilef;
	else
		autoexecflag = 1;
#ifdef GRAPHICS_LOADING_UI
	struct VBEINFO *vbeinfo = (struct VBEINFO *)VBEINFO_ADDRESS;
	SwitchVBEMode(0x115); //800x600x16.5M(真彩色模式的800x600显示模式)
	jpgview32("A:\\load.jpg", vbeinfo->vram, 800);
	unsigned char *stack = (unsigned char *)page_kmalloc(128*1024);
	AddTask("Read", 1, 2 * 8, (int)ReadTask, 1 * 8, 1 * 8, stack+128*1024);
	Draw_Box32(vbeinfo->vram, 800,269,545,269+14*18+13-1,545+15,0xff,0xff,0xff);
	while (readallflag) {
		Draw_Box32(vbeinfo->vram,800,269,545,269+dblock*18+dblock-2+18,545+14,0x00,162,232);
	}
	SubTask(GetTaskForName("Read"));
	page_kfree((int)stack,128*1024);
#endif
	SwitchToText8025_BIOS();
	if(*(unsigned char *)(ADR_DISKIMG) != 0xeb) //校验软盘数据
	{
		//正常的Powerint DOS启动盘的第一个字节是0xeb
		//所以如果不是0xeb，说明是一个非法的Powerint DOS启动盘（可能读取出差错了）
		clear();
		printf("[Read Err]This floppy disk first byte is not 0xeb\n");
		for(;;);
	}
	Set_Font("A:\\other\\font.bin");
#ifndef __DEBUG__
	SwitchToHighTextMode();
	HighPrint("感谢您使用Powerint DOS 386，现在系统初始化已经完成，即将进入登录页面\n请按任意键进入登录（注册）页面...");
	getch();
	Close_High_Text_Mode();
	SwitchToText8025_BIOS();
	Set_Font("A:\\other\\font.bin");
	char *user = fopen("A:\\USER.INI");
	char *password = fopen("A:\\PASSWORD.INI");
	if (user == 0 || password == 0)
	{
		clear();
		if (fopen("A:\\CREUS.bin") == 0)
		{
			printf("System Start Error: No CREUS.bin\n");
			for (;;)
				;
		}
		cmd_app("creus");
	}
	else
	{
		char inpass[32];
		char inusr[32];
		printf("Login:\n");
	rei:
		printf("User:");

		input(inusr, 32);
		printf("Password:");

		input(inpass, 32);
		if (strcmp(user, inusr) != 0 || strcmp(password, inpass) != 0)
		{
			sleep(100);
			printf("User or Password Error\n");
			clean(inusr, 32);
			clean(inpass, 32);
			goto rei;
		}
	}
	printf("----Login Or register Successful done!----\n");
#endif
	char *autoexec = fopen("autoexec.bat");
	if (autoexec == 0)
	{
		printf("Boot Warning:No AUTOEXEC.BAT in Drive ");
		printchar(*(char *)(0x7dfd));
		print("\n");
	}
	else
	{
		run_bat("autoexec.bat");
	}
	// end
whilef:
	while (1)
	{
		printchar(*(char *)(0x7dfd));
		print(":\\");
		print(path);
		print(">");
		int i;
		clean(line, 1024);
		input(line, 1024);
		command_run(line);
	}
}
void pci_list()
{
	unsigned char *pci_drive = PCI_ADDR_BASE;
	//输出PCI表的内容

	for (;; pci_drive += 0x110)
	{
		if (pci_drive[0] == 0xff)
		{
			PCI_ClassCode_Print((struct PCI_CONFIG_SPACE_PUCLIC *)(pci_drive+12));
		}
		else
		{
			break;
		}
	}
}
void task_sr1()
{
	// 简单的下半部
	extern int tasknum;
	while(1) {
	re:
		for (int i = 1; i != tasknum+1; i++) {
			if (GetTask(i)->running == 0) {	// 进程没有运行
				SubTask(GetTask(i));	// 就把它结束
				goto re;
			}
		}
	}
}
void task_sr2()
{
	while (1);
}
unsigned int get_memsize()
{
	return memsize;
}
int kbhit()
{
	if (pressed)
	{
		pressed = 0;
		return 1;
	}
	return 0;
}
int get_caps_lock()
{
	return caps_lock;
}
int get_shift()
{
	return shift;
}
char *get_point_of_keytable()
{
	return keytable;
}
char *get_point_of_keytable1()
{
	return keytable1;
}
