// 杂项函数
// Copyright (C) 2021-2022 zhouzhihao & min0911_
// ------------------------------------------------
#include <dos.h>
int DisableExpFlag = 0;
uint32_t CatchEIP = 0;
char flagOfexp = 0;
char public_catch = 0;
int st_task = 0;
void SwitchPublic() {
  public_catch = 1;
}
void SwitchPrivate() {
  public_catch = 0;
}
void disableExp() {
  if (public_catch) {
    DisableExpFlag = 1;
  } else {
    NowTask()->DisableExpFlag = 1;
  }
}
void EnableExp() {
  if (public_catch) {
    DisableExpFlag = 0;
  } else {
    NowTask()->DisableExpFlag = 0;
  }
}
char GetExpFlag() {
  // printk("Get.\n");
  if (public_catch) {
    return flagOfexp;
  } else {
    return NowTask()->flagOfexp;
  }
}
void ClearExpFlag() {
  if (public_catch) {
    flagOfexp = 0;
  } else {
    NowTask()->flagOfexp = 0;
  }
}
void SetCatchEip(uint32_t eip) {
  // printk("eip = %08x\n",eip);
  if (public_catch) {
    CatchEIP = eip;
  } else {
    NowTask()->CatchEIP = eip;
  }
}
void print_32bits_ascil(unsigned int n);
//保护模式调用BIOS中断的驱动
void INT(unsigned char intnum, regs16_t* regs) {
  extern unsigned char* IVT;
  struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
  set_segmdesc(gdt + 1000, 0xffffffff, 0, AR_CODE32_ER);  // CODE32
  set_segmdesc(gdt + 1001, 0xfffff, 0, AR_CODE16_ER);     // CODE16
  set_segmdesc(gdt + 1002, 0xfffff, 0, AR_DATA16_RW);     // DATA16
  memcpy(0, IVT, 0x400);
  int32(intnum, regs);
  set_segmdesc(gdt + 1000, 0, 0, 0);  // 临时GDT清零
  set_segmdesc(gdt + 1001, 0, 0, 0);
  set_segmdesc(gdt + 1002, 0, 0, 0);
}
void insert_char(char* str, int pos, char ch) {
  int i;
  for (i = strlen(str); i >= pos; i--) {
    str[i + 1] = str[i];
  }
  str[pos] = ch;
}
void delete_char(char* str, int pos) {
  int i;
  for (i = pos; i < strlen(str); i++) {
    str[i] = str[i + 1];
  }
}
void ERROR0(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();  // loadregisters();
  ERROR(0, "#DE");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR1(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(1, "#DB");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR3(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(3, "#BP");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR4(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(4, "#OF");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR5(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(5, "#BR");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR6(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(6, "#UD");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
int dflag = 0;
bool has_fpu_error() {
  dflag = 0;
  uint16_t status_word;
  asm("fnstsw %0" : "=m"(status_word));
  dflag = 1;
  return (status_word & 0x1F) != 0;
}
void ERROR7(uint32_t eip) {
  if (dflag) {
    // printk("1\n");
    return;
  }
  if (has_fpu_error()) {
    asm volatile("fnclex");
    return;
  }

  if (NowTask()->fpu_use == 1) {
    // Maskirq(0);
    // dflag = 1;
    // if (NowTask()->fpu_use == 1 && NowTask()->app == 1) {
    //  // printk("switch %s\n", NowTask()->name);
    //   asm volatile("frstor %0" ::"m"(NowTask()->fxsave_region));
    //   NowTask()->fpu_use = 0;
    // }
    // dflag = 0;
    // ClearMaskIrq(0);
    extern int dflag;
    dflag = 1;
    // printk("switch %s\n",NowTask()->name);
    asm volatile("frstor %0" ::"m"(NowTask()->fxsave_region));
    NowTask()->fpu_use = 0;
    dflag = 0;
    return;
  }
  st_task = Get_Tid(NowTask());
  if (st_task) {
    asm volatile("fnsave %0" ::"m"(GetTask(st_task)->fxsave_region));
    st_task = 0;
    NowTask()->fpu_use = 1;
  }
}
void ERROR8(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(8, "#DF");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR9(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(9, "#MF");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR10(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(10, "#TS");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR11(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(11, "#NP");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR12(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(12, "#SS");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR13(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(13, "#GP");
  if (public_catch) {
    printk("eip = %08x Catch EIP = %08x\n", eip, CatchEIP);
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR16(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(16, "#MF");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR17(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(17, "#AC");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR18(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(18, "#MC");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR19(uint32_t eip) {
  uint32_t* esp = &eip;
  saveregisters();
  ERROR(19, "#XF");
  if (public_catch) {
    *esp = CatchEIP;
  } else {
    *esp = NowTask()->CatchEIP;
  }
  loadregisters();  // 恢复寄存器状态
}
void ERROR(int CODE, char* TIPS) {
  if (public_catch) {
    flagOfexp = 1;
  } else {
    NowTask()->flagOfexp = 1;
  }
  printk("DisableExpFlag = %d\n",
         public_catch ? DisableExpFlag : NowTask()->DisableExpFlag);
  if (public_catch) {
    if (DisableExpFlag) {
      return;
    }
  } else {
    if (NowTask()->DisableExpFlag) {
      return;
    }
  }
  io_cli();
  Maskirq(0);
  printk("%s\n", TIPS);
  printk("Error Code: %d\n", CODE);
  SwitchToText8025_BIOS();
  clear();
  int i, j;
  for (i = 0; i < 160; i++) {
    for (j = 0; j < 25; j++) {
      //将屏幕背景色改为蓝底白字
      if (i % 2 == 1) {
        *(char*)(0xb8000 + j * 160 + i) = 0x1f;
      }
    }
  }
  NowTask()->TTY->color = 0x1f;
  gotoxy(0, 0);
  // beep(2, 7, 8);
  printf("Sorry!Your computer has some problem!\n");
  printf(
      "Please check your computer and try again.Or Restart your computer.\n");
  printf("Now You just look at the Error Message.\n");
  printf("You can rebuild PowerintDOS 386 too.\n");
  printf("The System Version:%s\n", VERSION);
  printf("Error Code:%08x\n", CODE);
  printf("Error Message:%s\n", TIPS);

  // loadregisters();

  printf("Task sel=%d\n", NowTask()->sel);
  io_cli();
  for (;;) {
  }
}
void KILLAPP(int eip, int ec) {
  struct TASK* task = NowTask();
  if (task->is_child) {
    task = task->thread.father;  // 找你家长，乱搞！
  }
  if (ec == 0xff) {  // 返回系统快捷键
    printf("\nSystem Protect:Break Key(F1).\n");
  } else {
    printf("\nSystem Protect:The program name:%s TASK ID:%d EC:%x EIP:%08x\n",
           task->name, task->sel / 8 - 103, ec, eip);
  }
  SleepTask(task);
  task->running = 0;
  WakeUp(GetTask(1));  // 别睡了，起来帮我杀下进程
  // 下半部会帮助我们结束程序
  for (;;)
    ;
}
void KILLAPP0(int ec, int tn) {
  struct TASK* task = GetTask(tn);
  struct tty* t = task->TTY;
  t = tty_set(NowTask(), t);
  if (ec == 0xff) {  // 返回系统快捷键
    printf("\n(%s)System Protect:Break Key(F1).\n", task->name);
  } else {
    printf("\nSystem Protect:The program name:%s TASK ID:%d EC:%x,EIP:%08x\n",
           task->name, task->sel / 8 - 103, ec, task->tss.eip);
  }
  tty_set(NowTask(), t);
  // SleepTask(task);
  extern uint32_t app_num;
  app_num--;
  task->running = 0;
  WakeUp(GetTask(1));  // 别睡了，起来帮我杀下进程
  io_sti();
  ClearMaskIrq(0);

  for (;;) {
    // printk("Wait.\n");
  }
}
char bcd2hex(char bcd) {
  char i;
  if (bcd > 0x10 || bcd == 0x10) {
    i = (bcd / 0x10) * 6;
    i = i + bcd - (bcd / 0x10) * 0x10;
    return i;
  } else {
    return bcd;
  }
}
char hex2bcd(char hex) {
  char i;
  if (hex > 0x0a || hex == 0x0a) {
    i = (hex / 0x0a) * 0x10;
    i = i + hex - (hex / 0x0a) * 0x0a;
    return i;
  } else {
    return hex;
  }
}
void Print_Hex(unsigned x) {
  //  printk("MEM TEST:%dKB", x / 1024);
  // for(int i = 0;i<0x03000000;i++);
}
void Clear_A_Line() {
  // printk("\n");
}
void getCPUBrand(char* cBrand) {
  print_32bits_ascil(get_cpu4(0x80000002));
  print_32bits_ascil(get_cpu5(0x80000002));
  print_32bits_ascil(get_cpu6(0x80000002));
  print_32bits_ascil(get_cpu7(0x80000002));
  print_32bits_ascil(get_cpu4(0x80000003));
  print_32bits_ascil(get_cpu5(0x80000003));
  print_32bits_ascil(get_cpu6(0x80000003));
  print_32bits_ascil(get_cpu7(0x80000003));
  print_32bits_ascil(get_cpu4(0x80000004));
  print_32bits_ascil(get_cpu5(0x80000004));
  print_32bits_ascil(get_cpu6(0x80000004));
  print_32bits_ascil(get_cpu7(0x80000004));
  printf("\n");
}
char ascii2num(char c) {
  if (c > 'A' - 1 && c < 'F' + 1) {
    c = c - 0x37;
  } else if (c > 'a' - 1 && c < 'f' + 1) {
    c = c - 0x57;
  } else if (c > '0' - 1 && c < '9' + 1) {
    c = c - 0x30;
  }
  return c;
}
char num2ascii(char c) {
  if (c > 0x9 && c < 0x10) {
    c = c + 0x37;
  } else if (c < 0x0a) {
    c = c + 0x30;
  }
  return c;
}
void strtoupper(char* str) {
  while (*str != '\0') {
    if (*str >= 'a' && *str <= 'z') {
      *str -= 32;
    }
    str++;
  }
}
int GetCHorEN(unsigned char* str) {
  //获取这个字符是中文全角还是英文半角
  if (str[0] > 0x80 && str[1] > 0x80) {
    return 1;
  } else if (str[0] > 0x80 && str[1] < 0x80) {
    return 0;
  } else {
    return 0;
  }
}
void clean(char* s, int len) {
  //清理某个内存区域（全部置0）
  int i;
  for (i = 0; i != len; i++) {
    s[i] = 0;
  }
  return;
}
void print_32bits_ascil(unsigned int n) {
  char str[32];
  int i = 0;
  for (i = 0; i < 4; i++) {
    str[i] = n & 0xff;
    n >>= 8;
  }

  for (i = 0; i < 4; i++) {
    printchar(str[i]);
  }
}