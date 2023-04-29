#include <dos.h>
#include <drivers.h>
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47
static int caps_lock, shift, e0_flag = 0;
char keytable[0x54] = {  // 按下Shift
    0,    0x01, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',  '+',
    '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{',  '}',
    10,   0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
    0,    '|',  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,    '*',
    0,    ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
    0,    '7',  'D', '8', '-', '4', '5', '6', '+', '1', '2', '3', '0',  '.'};
char keytable1[0x54] = {  // 未按下Shift
    0,    0x01, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']',
    10,   0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,    '*',
    0,    ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
    0,    '7',  '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0',  '.'};
void wait_KBC_sendready(void) {
  /* 等待键盘控制电路准备完毕 */
  for (;;) {
    if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
      break;
    }
  }
  return;
}

void init_keyboard(void) {
  /* 初始化键盘控制电路 */
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
  return;
}
int getch() {
  unsigned char ch;
  ch = input_char_inSM();  // 扫描码
  if (ch > 0x80) {         // keytable之外的键（↑,↓,←,→）
    ch -= 0x80;
    if (ch == 0x48) {  // ↑
      return -1;
    } else if (ch == 0x50) {  // ↓
      return -2;
    } else if (ch == 0x4b) {  // ←
      return -3;
    } else if (ch == 0x4d) {  // →
      return -4;
    }
  }
  // 返回扫描码（keytable之内）对应的ASCII码
  if (keytable[ch] == 0x00) {
    return 0;
  }
  if (shift == 0 && caps_lock == 0) {
    return keytable1[ch];
  } else if (shift == 1 || caps_lock == 1) {
    return keytable[ch];
  } else if (shift == 1 && caps_lock == 1) {
    return keytable1[ch];
  }
}
int input_char_inSM() {
  int i;
  struct TASK* task = NowTask();
  struct SHEET* sht_win = (struct SHEET*)task->TTY->reserved[1];
  extern struct SHTCTL* shtctl;
  while (1) {
    if ((fifo8_status(TaskGetKeyfifo(NowTask())) == 0) ||
        (running_mode == POWERDESKTOP && NowTask()->app && NowTask()->forever &&
         shtctl->sheets[shtctl->top - 1]->task != NowTask()) ||
        (NowTask()->TTY != now_tty() && NowTask()->TTY->using1 == 1 &&
         !NowTask()->forever)) {
      // 不返回扫描码的情况
      // 1.没有输入
      // 2.窗口未处于顶端
      // 3.正在运行的控制台并不是函数发起的控制台（TTY）
      if (fifo8_status(TaskGetKeyfifo(NowTask())) != 0) {
        fifo8_get(
            TaskGetKeyfifo(NowTask()));  // tnnd 都到这里了还想走？给你拦截了
      }
      io_stihlt();
    } else {
      // 返回扫描码
      i = fifo8_get(TaskGetKeyfifo(NowTask()));  // 从FIFO缓冲区中取出扫描码
      if (i != -1) {
        break;
      }
    }
  }
  return i;
}
int kbhit() {
  // printk("kbhit : %d\n", fifo8_status(NowTask()->keyfifo));
  return fifo8_status(NowTask()->keyfifo) != 0;  // 进程的键盘FIFO缓冲区是否为空
}
int sc2a(int sc) {
  // 扫描码转化ASCII码
  // 逻辑与getch函数大同小异
  int ch = sc;
  if (ch > 0x80) {
    ch -= 0x80;
    if (ch == 0x48) {
      return -1;
    } else if (ch == 0x50) {
      return -2;
    } else if (ch == 0x4b) {
      return -3;
    } else if (ch == 0x4d) {
      return -4;
    }
  }
  if (keytable[ch] == 0x00) {
    return 0;
  }
  if (shift == 0 && caps_lock == 0) {
    return keytable1[ch];
  } else if (shift == 1 || caps_lock == 1) {
    return keytable[ch];
  } else if (shift == 1 && caps_lock == 1) {
    return keytable1[ch];
  }
}

void inthandler21(int* esp) {
  // 键盘中断处理函数
  io_cli();
  extern int tasknum;
  unsigned char data, s[4];
  io_out8(PIC0_OCW2, 0x61);
  data = io_in8(PORT_KEYDAT);  // 从键盘IO口读取扫描码
  // printk("%02x\n", data);
  // 特殊键处理
  if (data == 0xe0) {
    // printk("Set e0_flag\n");
    e0_flag = 0x80;
    io_sti();
    return;
  }
  if (data == 0x2a || data == 0x36) {  // Shift按下
    shift = 1;
    io_sti();
    return;
  }
  if (data == 0x3a) {  // Caps Lock按下
    caps_lock = caps_lock ^ 1;
    io_sti();
    return;
  }
  if (data == 0xaa || data == 0xb6) {  // Shift松开
    shift = 0;
    io_sti();
    return;
  }
  // 快捷键处理
  extern int app_task_num;  // 现在运行的程序（POWERDESKTOP/HIGHTEXTMODE模式下）
  if (data == 0x3b && !shift) {
    // 仅仅按下F1
    if (running_mode == POWERINTDOS) {
      for (int i = 1; GetTask(i) != 0; i++) {
        if (GetTask(i)->TTY->vram == 0xB8000 && GetTask(i)->app == 1) {
          // printf("Break EIP:%08x",esp[1]);
          //  POWERINTDOS模式下找到现在运行的程序（掌握TTY屏幕主导权 & 是程序）
          KILLAPP0(0xff, i);  // 0xff是快捷键结束程序的便条
          break;
        }
      }
    } else {
      if (app_task_num != -1) {  // 有程序在运行
        KILLAPP0(0xff, app_task_num);
      }
    }
    io_sti();
    return;
  } else if (data == 0x3c && !shift) {
    if (running_mode == POWERDESKTOP) {
      AddShell_GraphicMode();
      io_sti();
      return;
    }
  } else if (data >= 0x3b && data <= 0x47 && shift) {
    // 按下F1 ~ F12 & Shift
    if (running_mode == POWERINTDOS) {
      SwitchShell_TextMode(data - 0x3b + 1);  // 换到1~12号控制台
      io_sti();
      return;
    } else if (running_mode == HIGHTEXTMODE) {
      SwitchShell_HighTextMode(data - 0x3b + 1);
      io_sti();
      return;
    }
  }
  // 普通键处理
  if (data >= 0x80) {
    //printk("press\n");
    for (int i = 1; i < tasknum + 1; i++) {
      if (GetTask(i)->keyboard_release != NULL) {
        // TASK结构体中有对松开键特殊处理的
        GetTask(i)->keyboard_release(data,i);  // 处理松开键
      }
    }

    io_sti();
    return;
  }
  for (int i = 1; i < tasknum + 1; i++) {
   // printk("up\n");
    if (GetTask(i)->keyboard_press != NULL) {
      // TASK结构体中有对按下键特殊处理的
      GetTask(i)->keyboard_press(data,i);  // 处理按下键
    }
  }
  for (int i = 1; i < tasknum + 1; i++) {
    // 按下键通常处理
    struct TASK* task = GetTask(i);  // 每个进程都处理一遍
    if (task->sleep == 1 || task->fifosleep)
      // 如果进程正在休眠
      continue;
    // 一般进程
    // printk("Send packet to %s ID %d\n",task->name,Get_Tid(task));
    fifo8_put(TaskGetKeyfifo(task), data + e0_flag);
    // printk(
    //    "TASKNUM:%d TASK NAME:%s TASK ID:%d TASK FIFO:%08x STATUS:%d
    //    DATA:%c\n", tasknum, task->name, Get_Tid(task), TaskGetKeyfifo(task),
    //    fifo8_status(TaskGetKeyfifo(task)), data);
  }
  e0_flag = 0;
  io_sti();
  return;
}
