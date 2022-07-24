#include <dos.h>
#include <drivers.h>

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
  char ch;
  ch = input_char_inSM();
  if (keytable[ch] == 0x00) {
    return 0;
  }
  if (ch == 0x48 || ch == 0x50 || ch == 0x4b || ch == 0x4d) {
    if (ch == 0x48) {
      return -1;
    } else if (ch == 0x50) {
      return -2;
    } else if (ch == 0x4b) {
      return -3;
    } else if (ch == 0x4d) {
      return -4;
    }
  } else if (shift == 0 && caps_lock == 0) {
    return keytable1[ch];
  } else if (shift == 1 || caps_lock == 1) {
    return keytable[ch];
  } else if (shift == 1 && caps_lock == 1) {
    return keytable1[ch];
  }
}
int input_char_inSM() {
  int i;
  while (1) {
    if (fifo8_status(TaskGetKeyfifo(NowTask())) == 0) {  //没有输入
      io_stihlt();
    } else {
      i = fifo8_get(TaskGetKeyfifo(NowTask()));
      if (i < 0x80) {
        break;
      } else {
        // printk("Not a key --> %02x\n", i);
      }
    }
  }
  return i;
}
int kbhit() {
 // printk("kbhit : %d\n", fifo8_status(NowTask()->keyfifo));
  return fifo8_status(NowTask()->keyfifo) != 0;
}
int sc2a(int sc) {
  int ch = sc;
  if (keytable[ch] == 0x00) {
    return 0;
  }
  if (ch == 0x48 || ch == 0x50 || ch == 0x4b || ch == 0x4d) {
    if (ch == 0x48) {
      return -1;
    } else if (ch == 0x50) {
      return -2;
    } else if (ch == 0x4b) {
      return -3;
    } else if (ch == 0x4d) {
      return -4;
    }
  } else if (shift == 0 && caps_lock == 0) {
    return keytable1[ch];
  } else if (shift == 1 || caps_lock == 1) {
    return keytable[ch];
  } else if (shift == 1 && caps_lock == 1) {
    return keytable1[ch];
  }
}

int caps_lock, shift;
void inthandler21(int* esp) {
  extern int tasknum;
  unsigned char data, s[4];
  io_out8(PIC0_OCW2, 0x61);
  data = io_in8(PORT_KEYDAT);
  
  if (data == 0x2a || data == 0x36) {
    shift = 1;
    return;
  }
  if (data == 0x3a) {  // Caps Lock
    caps_lock = caps_lock ^ 1;
    return;
  }
  if (data == 0xaa || data == 0xb6) {  // shift
    shift = 0;
    return;
  }
  // ctrl
  extern int app_task_num;
  if (data == 0x3b) {
    if (app_task_num != -1) {
      KILLAPP0(0xff, app_task_num);
    }
    return;
  }
  if (data >= 0x80) {
    return;
  }
  for (int i = 1; i < tasknum + 1; i++) {
    struct TASK* task = GetTask(i);
    if (task->sleep == 1 || task->fifosleep == 1)
      continue;
    fifo8_put(TaskGetKeyfifo(task), data);
    //printk("TASK NAME:%s TASK FIFO:%08x STATUS:%d DATA:%c\n",task->name,TaskGetKeyfifo(task),fifo8_status(TaskGetKeyfifo(task)),data);
  }
  return;
}
