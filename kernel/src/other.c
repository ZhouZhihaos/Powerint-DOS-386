// 杂项函数
// Copyright (C) 2021-2022 zhouzhihao & min0911_
// ------------------------------------------------
#include "system.h"
void insert_char(char *str, int pos, char ch)
{
	int i;
	for (i = strlen(str); i >= pos; i--)
	{
		str[i + 1] = str[i];
	}
	str[pos] = ch;
}
void delete_char(char *str, int pos)
{
	int i;
	for (i = pos; i < strlen(str); i++)
	{
		str[i] = str[i + 1];
	}
}
void ERROR0()
{
	saveregisters(); // 锟斤拷应loadregisters();
	ERROR(0, "#DE");
}
void ERROR1()
{
	saveregisters();
	ERROR(1, "#DB");
}
void ERROR3()
{
	saveregisters();
	ERROR(3, "#BP");
}
void ERROR4()
{
	saveregisters();
	ERROR(4, "#OF");
}
void ERROR5()
{
	saveregisters();
	ERROR(5, "#BR");
}
void ERROR6()
{
	saveregisters();
	ERROR(6, "#UD");
}
void ERROR7()
{
	saveregisters();
	ERROR(7, "#NM");
}
void ERROR8()
{
	saveregisters();
	ERROR(8, "#DF");
}
void ERROR9()
{
	saveregisters();
	ERROR(9, "#MF");
}
void ERROR10()
{
	saveregisters();
	ERROR(10, "#TS");
}
void ERROR11()
{
	saveregisters();
	ERROR(11, "#NP");
}
void ERROR12()
{
	saveregisters();
	ERROR(12, "#SS");
}
void ERROR13()
{
	saveregisters();
	ERROR(13, "#GP");
}
void ERROR14()
{
	saveregisters();
	ERROR(14, "#PF");
}
void ERROR16()
{
	saveregisters();
	ERROR(16, "#MF");
}
void ERROR17()
{
	saveregisters();
	ERROR(17, "#AC");
}
void ERROR18()
{
	saveregisters();
	ERROR(18, "#MC");
}
void ERROR19()
{
	saveregisters();
	ERROR(19, "#XF");
}
void ERROR(int CODE, char *TIPS)
{
	SwitchToText8025();
	clear();
	int i, j;
	for (i = 0; i < 160; i++)
	{
		for (j = 0; j < 25; j++)
		{
			//将屏幕背景色改为蓝底白字
			if (i % 2 == 1)
			{
				*(char *)(0xb8000 + j * 160 + i) = 0x4f;
			}
		}
	}
	gotoxy(0, 0);
	beep(2, 7, 8);
	printf("Sorry!Your computer has some problem!\n");
	printf("Please check your computer and try again.Or Restart your computer.\n");
	printf("Now You just look at the Error Message.\n");
	printf("You can rebuild PowerintDOS 386 too.\n");
	printf("The System Version:%s\n", VERSION);
	printf("Error Code:%08x\n", CODE);
	printf("Error Message:%s\n", TIPS);

	loadregisters();
					
	int eax = NowTask()->tss.eax;
	printf("eax=%08x\n", eax);
	int ebx = NowTask()->tss.ebx;
	printf("ebx=%08x\n", ebx);
	int ecx = NowTask()->tss.ecx;
	printf("ecx=%08x\n", ecx);
	int edx = NowTask()->tss.edx;
	printf("edx=%08x\n", edx);
	int esi = NowTask()->tss.esi;
	printf("esi=%08x\n", esi);
	int edi = NowTask()->tss.edi;
	printf("edi=%08x\n", edi);
	int ebp = NowTask()->tss.ebp;
	printf("ebp=%08x\n", ebp);
	int esp=NowTask()->tss.esp;
	printf("esp=%08x\n", esp);
	printf("eip=%08x\n", NowTask()->tss.eip);
	//段寄存器
	int cs = NowTask()->tss.cs;
	printf("cs=%04x\n", cs);
	int ds = NowTask()->tss.ds;
	printf("ds=%04x\n", ds);
	int es=NowTask()->tss.es;
	printf("es=%04x\n", es);
	int fs=NowTask()->tss.fs;
	printf("fs=%04x\n", fs);
	int gs = NowTask()->tss.gs;
	printf("gs=%04x\n", gs);
	int ss = NowTask()->tss.ss;
	printf("ss=%04x\n", ss);
	for (;;)
	{
	}
}
void KILLAPP(int ec)
{
	
	struct TASK *task = NowTask();
	if (ec == 0xff) {	// 返回系统快捷键
		printf("\nSystem Protect:Break Key(F1).\n");
	} else {
		printf("\nSystem Protect:The program name:%s TASK ID:%d EC:%x EIP:%08x\n",task->name,task->sel/8-103,ec,task->tss.eip);
	}
	SleepTask(task);
	task->running = 0;
	// 下半部会帮助我们结束程序
}
void KILLAPP0(int ec,int tn)
{
	struct TASK *task =GetTask(tn);
	if (ec == 0xff) {	// 返回系统快捷键
		printf("\nSystem Protect:Break Key(F1).\n");
	} else {
		printf("\nSystem Protect:The program name:%s TASK ID:%d EC:%x,EIP:%08x\n",task->name,task->sel/8-103,ec,task->tss.eip);
	}
	SleepTask(task);
	task->running = 0;
}
char bcd2hex(char bcd)
{
	char i;
	if (bcd > 0x10 || bcd == 0x10)
	{
		i = (bcd / 0x10) * 6;
		i = i + bcd - (bcd / 0x10) * 0x10;
		return i;
	}
	else
	{
		return bcd;
	}
}
char hex2bcd(char hex)
{
	char i;
	if (hex > 0x0a || hex == 0x0a)
	{
		i = (hex / 0x0a) * 0x10;
		i = i + hex - (hex / 0x0a) * 0x0a;
		return i;
	}
	else
	{
		return hex;
	}
}
void Print_Hex(unsigned x)
{
	#ifndef __DEBUG__
	printf("MEM TEST:%dKB", x / 1024);
	#endif
}
void Clear_A_Line()
{
	gotoxy(0, 0);
}
void getCPUBrand(char *cBrand)
{
	print_32bits_ascil(get_cpu4(0x80000002));
	print_32bits_ascil(get_cpu5(0x80000002));
	print_32bits_ascil(get_cpu6(0x80000002));
	print_32bits_ascil(get_cpu7(0x80000002));
	print_32bits_ascil(get_cpu4(0x80000003));
	print_32bits_ascil(get_cpu5(0x80000003));
	print_32bits_ascil(get_cpu6(0x80000003));
	print_32bits_ascil(get_cpu7(0x80000003));
	print_32bits_ascil(get_cpu4(0x80000004));
	print_32bits_ascil(get_cpu5(0x80000004));
	print_32bits_ascil(get_cpu6(0x80000004));
	print_32bits_ascil(get_cpu7(0x80000004));
	printf("\n");
}
char ascii2num(char c)
{
	if (c > 'A' - 1 && c < 'F' + 1)
	{
		c = c - 0x37;
	}
	else if (c > 'a' - 1 && c < 'f' + 1)
	{
		c = c - 0x57;
	}
	else if (c > '0' - 1 && c < '9' + 1)
	{
		c = c - 0x30;
	}
	return c;
}
char num2ascii(char c)
{
	if (c > 0x9 && c < 0x10)
	{
		c = c + 0x37;
	}
	else if (c < 0x0a)
	{
		c = c + 0x30;
	}
	return c;
}
char *itoa(int n)
{
	char *res;
	sprintf(res, "%d", n);
	return res;
}
void strtoupper(char *str)
{
	while (*str != '\0')
	{
		if (*str >= 'a' && *str <= 'z')
		{
			*str -= 32;
		}
		str++;
	}
}
int GetCHorEN(unsigned char *str)
{
	//获取这个字符是中文全角还是英文半角
	if (str[0] > 0x80 && str[1] > 0x80)
	{
		return 1;
	}
	else if (str[0] > 0x80 && str[1] < 0x80)
	{
		return 0;
	}
	else
	{
		return 0;
	}
}
void clean(char *s, int len)
{
	//清理某个内存区域（全部置0）
	int i;
	for (i = 0; i != len; i++)
	{
		s[i] = 0;
	}
	return;
}
void print_32bits_ascil(unsigned int n)
{
	char str[32];
	int i = 0;
	for (i = 0; i < 4; i++)
	{
		str[i] = n & 0xff;
		n >>= 8;
	}

	for (i = 0; i < 4; i++)
	{
		printchar(str[i]);
	}
}
