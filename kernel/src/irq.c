/*
    irq.c
    IRQ handler
*/
#include "system.h"
#define PIC1		0x20
#define PIC2		0xA0
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

void ClearMaskIrq(unsigned char irq)
{
    unsigned short port;
    unsigned char value;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = io_in8(port) & ~(1 << irq);
    io_out8(port, value);
}
void Maskirq(unsigned char irq)
{
    unsigned short port;
    unsigned char value;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = io_in8(port) | (1 << irq);
    io_out8(port, value);
}