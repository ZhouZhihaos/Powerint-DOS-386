#include <dos.h>
#include <drivers.h>
extern struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cur, *sht_fm;
extern struct SHEET* sht;
extern struct SHTCTL* shtctl;
extern struct SHEET* sht_win_;
extern struct SHEET* sht_b_cur;
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47
char keytable[0x54] = {
    0,    0,    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',  '+',
    '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{',  '}',
    10,   0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
    0,    '|',  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,    '*',
    0,    ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
    0,    '7',  'D', '8', '-', '4', '5', '6', '+', '1', '2', '3', '0',  '.'};
char keytable1[0x54] = {
    0,    0,    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']',
    10,   0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,    '*',
    0,    ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
    0,    '7',  '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0',  '.'};
int e0_flag = 0;
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
extern int caps_lock, shift;
extern struct SHEET* sht_win;
extern struct SHTCTL* shtctl;
int getch() {
  unsigned char ch;
  ch = input_char_inSM();
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
int input_char_inSM() {
  int i;
  struct TASK* task = NowTask();
  while (1) {
    if ((fifo8_status(TaskGetKeyfifo(task)) == 0) ||
        (running_mode == POWERDESKTOP &&
         sht_win->height != shtctl->top - 1)) {  //没有输入
      io_stihlt();
    } else {
      i = fifo8_get(TaskGetKeyfifo(task));
      if (i != -1) {
        break;
      }
    }
  }
  return i;
}
int kbhit() {
  //printk("kbhit : %d\n", fifo8_status(NowTask()->keyfifo));
  return fifo8_status(NowTask()->keyfifo) != 0;
}
int sc2a(int sc) {
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

int caps_lock, shift;
void inthandler21(int* esp) {
  io_cli();
  extern int tasknum;
  unsigned char data, s[4];
  io_out8(PIC0_OCW2, 0x61);
  data = io_in8(PORT_KEYDAT);
  // printk("%02x\n", data);
  //printk("%02x\n", data);
  if (data == 0xe0) {
    // printk("Set e0_flag\n");
    e0_flag = 0x80;
    io_sti();
    return;
  }
  if (data == 0x2a || data == 0x36) {
    shift = 1;
    io_sti();
    return;
  }
  if (data == 0x3a) {  // Caps Lock
    caps_lock = caps_lock ^ 1;
    io_sti();
    return;
  }
  if (data == 0xaa || data == 0xb6) {  // shift
    shift = 0;
    io_sti();
    return;
  }
  // ctrl
  extern int app_task_num;
  if (data == 0x3b) {
    if (app_task_num != -1) {
      KILLAPP0(0xff, app_task_num);
    }
    io_sti();
    return;
  }
  if (data >= 0x80) {
    if (NowTask()->keyboard_release != NULL) {
      NowTask()->keyboard_release(data);
    }
    io_sti();
    return;
  }
  if (NowTask()->keyboard_press != NULL) {
    NowTask()->keyboard_press(data);
  }
  for (int i = 1; i < tasknum + 1; i++) {
    struct TASK* task = GetTask(i);
    if (task->sleep == 1 || task->fifosleep == 1)
      continue;
    // printk("e0_flag = %02x\n%02x + %02x = %02x\n",
    // e0_flag,data,e0_flag,data+e0_flag);
    fifo8_put(TaskGetKeyfifo(task), data + e0_flag);
    // printk(
    //     "TASKNUM:%d TASK NAME:%s TASK ID:%d TASK FIFO:%08x STATUS:%d
    //     DATA:%c\n", tasknum, task->name, Get_Tid(task), TaskGetKeyfifo(task),
    //     fifo8_status(TaskGetKeyfifo(task)), data);
  }
  e0_flag = 0;
  io_sti();
  return;
}
