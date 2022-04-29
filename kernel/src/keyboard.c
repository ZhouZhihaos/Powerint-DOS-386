#include "system.h"
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47
extern struct TASK *shell_task;
void wait_KBC_sendready(void)
{
	/* 等待键盘控制电路准备完毕 */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* 初始化键盘控制电路 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}
int getch()
{
	char ch;
	ch = input_char_inSM();
	if (keytable[ch] == 0x00)
	{
		return 0;
	}
	if (ch == 0x48 || ch == 0x50 || ch == 0x4b || ch == 0x4d)
	{
		if (ch == 0x48)
		{
			return -1;
		}
		else if (ch == 0x50)
		{
			return -2;
		}
		else if (ch == 0x4b)
		{
			return -3;
		}
		else if (ch == 0x4d)
		{
			return -4;
		}
	}
	else if (shift == 0 && caps_lock == 0)
	{
		return keytable1[ch];
	}
	else if (shift == 1 || caps_lock == 1)
	{
		return keytable[ch];
	}
	else if (shift == 1 && caps_lock == 1)
	{
		return keytable1[ch];
	}
}
int input_char_inSM()
{
	int i;
	while (1)
	{
		io_cli();
		if (fifo8_status(TaskGetKeyfifo(NowTask())) == 0)
		{ //没有输入
			io_stihlt();
		}
		else
		{
			i = fifo8_get(TaskGetKeyfifo(NowTask()));
			io_sti();
			if (i < 0x80)
			{
				break;
			}
		}
	}
	return i;
}