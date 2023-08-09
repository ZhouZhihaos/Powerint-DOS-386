#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H
#include <define.h>
// inthandler.c
void inthandler20(int cs,int* esp);
void inthandler21(int *esp);
void inthandler2c(int *esp);
// pic.c
void init_pic(void);
void send_eoi(int irq);
// irq.c
void irq_mask_clear(unsigned char irq);
void irq_mask_set(unsigned char irq);
#endif
