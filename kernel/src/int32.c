#include "system.h"
//保护模式调用BIOS中断的驱动
void INT(unsigned char intnum, regs16_t *regs)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	set_segmdesc(gdt + 1000, 0xffffffff, 0, AR_CODE32_ER); // CODE32
	set_segmdesc(gdt + 1001, 0xfffff, 0, AR_CODE16_ER);	   // CODE16
	set_segmdesc(gdt + 1002, 0xfffff, 0, AR_DATA16_RW);	   // DATA16
	int32(intnum, regs);
	set_segmdesc(gdt + 1000, 0, 0, 0); // 临时GDT清零
	set_segmdesc(gdt + 1001, 0, 0, 0);
	set_segmdesc(gdt + 1002, 0, 0, 0);
}