#include "system.h"


#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(struct MOUSE_DEC *mdec)
{
	/* 激活鼠标 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 1;
	return; /* 顺利的话，键盘控制器会返回ACK(0xfa) */
}

void mouse_sleep(struct MOUSE_DEC *mdec)
{
	mdec->sleep = 1;
	return;
}

void mouse_ready(struct MOUSE_DEC *mdec)
{
	mdec->sleep = 0;
	return;
}
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 1)
	{
		if (dat == 0xfa)
		{ // ACK
			return 0;
		}
		mdec->buf[0] = dat;
		mdec->phase = 2;
		return 0;
	}
	else if (mdec->phase == 2)
	{
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	else if (mdec->phase == 3)
	{
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07; // 锟斤拷锟斤拷锟斤拷取锟斤拷锟斤拷锟斤拷位锟斤拷
		mdec->x = mdec->buf[1];			 // x
		mdec->y = mdec->buf[2];			 // y
		if ((mdec->buf[0] & 0x10) != 0)
		{
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0)
		{
			mdec->y |= 0xffffff00;
		}
		mdec->y = -mdec->y; // y取锟斤拷
		return 1;
	}
	return -1;
}