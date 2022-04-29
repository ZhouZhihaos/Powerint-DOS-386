#include "system.h"
int check_vbe_mode(int mode,struct VBEINFO *vinfo)
{
	init_palette();
	regs16_t regs;
	regs.ax = 0x4f01;
	regs.cx = mode+0x4000;
	regs.es = (int)(vinfo) / 0x10;
	regs.di = (int)(vinfo) % 0x10;
	INT(0x10,&regs);
	if (regs.ax != 0x004f)
		return -1;
	return 0;
}
int SwitchVBEMode(int mode)
{
	init_palette();
	struct VBEINFO *vinfo = VBEINFO_ADDRESS;
	if (check_vbe_mode(mode,vinfo) != 0)
		return -1;
	regs16_t regs;
	regs.ax = 0x4f02;
	regs.bx = mode+0x4000;
	INT(0x10,&regs);
	return 0;
}
void SwitchToText8025_BIOS()
{
	regs16_t regs;
	regs.ax = 0x0003;
	INT(0x10,&regs);
	clear();
}
void SwitchTo320X200X256_BIOS()
{
	regs16_t regs;
	regs.ax = 0x0013;
	INT(0x10,&regs);
}