#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H
#include <define.h>
// inthandler.c
void inthandler20(int* esp);
void inthandler21(int *esp);
void inthandler2c(int *esp);
// pic.c
void init_pic(void);
void send_eoi(int irq);
// irq.c
void ClearMaskIrq(unsigned char irq);
void Maskirq(unsigned char irq);
#endif
