#include <dosldr.h>
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR 0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1
void ClearMaskIrq(unsigned char irq) {
  unsigned short port;
  unsigned char value;

  if (irq < 8) {
    port = PIC0_IMR;
  } else {
    port = PIC1_IMR;
    irq -= 8;
  }
  value = io_in8(port) & ~(1 << irq);
  io_out8(port, value);
}
void Maskirq(unsigned char irq) {
  unsigned short port;
  unsigned char value;

  if (irq < 8) {
    port = PIC0_IMR;
  } else {
    port = PIC1_IMR;
    irq -= 8;
  }
  value = io_in8(port) | (1 << irq);
  io_out8(port, value);
}
void init_pic(void) {
  io_out8(PIC0_IMR, 0xff); /* 初始化，所有中断均被屏蔽 */
  io_out8(PIC1_IMR, 0xff);
  io_out8(PIC0_ICW1, 0x11);
  io_out8(PIC0_ICW2, 0x20);
  io_out8(PIC0_ICW3, 1 << 2);
  io_out8(PIC0_ICW4, 0x01);

  io_out8(PIC1_ICW1, 0x11);
  io_out8(PIC1_ICW2, 0x28);
  io_out8(PIC1_ICW3, 2);
  io_out8(PIC1_ICW4, 0x01);

  io_out8(PIC0_IMR, 0xfb);
  io_out8(PIC1_IMR, 0xff); /* 禁止所有中断 */
  return;
}
void send_eoi(int irq) {
  if (irq >= 8) {
    io_out8(PIC1_OCW2, 0x60 | (irq - 8));
    io_out8(PIC0_OCW2, 0x60 | 2);
  } else {
    io_out8(PIC0_OCW2, 0x60 | irq);
  }
}