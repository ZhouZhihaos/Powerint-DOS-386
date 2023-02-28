#include <drivers.h>

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4
typedef unsigned char byte;
void mouse_wait(byte a_type)  // unsigned char
{
  unsigned int _time_out = 100000;  // unsigned int
  if (a_type == 0) {
    while (_time_out--)  // Data
    {
      if ((io_in8(0x64) & 1) == 1) {
        return;
      }
    }
    return;
  } else {
    while (_time_out--)  // Signal
    {
      if ((io_in8(0x64) & 2) == 0) {
        return;
      }
    }
    return;
  }
}

void mouse_write(byte a_write)  // unsigned char
{
  // Wait to be able to send a command
  mouse_wait(1);
  // Tell the mouse we are sending a command
  io_out8(0x64, 0xD4);
  // Wait for the final part
  mouse_wait(1);
  // Finally write
  io_out8(0x60, a_write);
}

byte mouse_read() {
  // Get's response from mouse
  mouse_wait(0);
  return io_in8(0x60);
}
void mouse_reset() {
  mouse_write(0xff);
}
void enable_mouse(struct MOUSE_DEC* mdec) {
  /* 激活鼠标 */
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
  mdec->phase = 1;
  mouse_write(0xf3);
  mouse_write(200);
  mouse_write(0xf3);
  mouse_write(100);
  mouse_write(0xf3);
  mouse_write(80);
  mouse_write(0xf2);
  printk("mouseId=%d\n", mouse_read());
  return; /* 顺利的话，键盘控制器会返回ACK(0xfa) */
}

void mouse_sleep(struct MOUSE_DEC* mdec) {
  mdec->sleep = 1;
  return;
}

void mouse_ready(struct MOUSE_DEC* mdec) {
  mdec->sleep = 0;
  return;
}
int mouse_decode(struct MOUSE_DEC* mdec, unsigned char dat) {
  if (mdec->phase == 1) {
    if (dat == 0xfa) {  // ACK
      return 0;
    }
    mdec->buf[0] = dat;
    mdec->phase = 2;
    return 0;
  } else if (mdec->phase == 2) {
    mdec->buf[1] = dat;
    mdec->phase = 3;
    return 0;
  } else if (mdec->phase == 3) {
    mdec->buf[2] = dat;
    mdec->phase = 4;
    return 0;
  } else if (mdec->phase == 4) {
    // printk("已经收集了四个字节\n");
    mdec->buf[3] = dat;
    mdec->phase = 1;
    mdec->btn = mdec->buf[0] & 0x07;
    mdec->x = mdec->buf[1];  // x
    mdec->y = mdec->buf[2];  // y
    if ((mdec->buf[0] & 0x10) != 0) {
      mdec->x |= 0xffffff00;
    }
    if ((mdec->buf[0] & 0x20) != 0) {
      mdec->y |= 0xffffff00;
    }
    mdec->y = -mdec->y;  //
    if (mdec->buf[3] == 0xff) {
      mdec->roll = MOUSE_ROLL_UP;
    } else if (mdec->buf[3] == 0x01) {
      mdec->roll = MOUSE_ROLL_DOWN;
    } else {
      mdec->roll = MOUSE_ROLL_NONE;
    }
    return 1;
  }
  return -1;
}
// int a = 1;
void inthandler2c(int* esp) {
  io_cli();
  extern int tasknum;
  unsigned char data;
  io_out8(PIC1_OCW2, 0x64);
  io_out8(PIC0_OCW2, 0x62);
  data = io_in8(PORT_KEYDAT);
  //printk("data=%02x\n", data);
  if (mdec.sleep == 0) {
    for (int i = 1; i < tasknum + 1; i++) {
      struct TASK* task = GetTask(i);
      if (task->sleep == 1 || task->fifosleep == 1)
        continue;
      fifo8_put(TaskGetMousefifo(task), data);
    }
  }
  io_sti();
  return;
}
