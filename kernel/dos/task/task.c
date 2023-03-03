#define _TASK_C
#include <dos.h>
#define	SA_RPL_MASK	0xFFFC
#define	SA_TI_MASK	0xFFFB
#define	SA_TIL		4 // 设置此项，将从LDT中寻找
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3
#define GET_SEL(cs,rpl) ((cs & SA_RPL_MASK & SA_TI_MASK) | (rpl))
struct TIMER *mt_timer1, *mt_timer2, *mt_timer3;
int mt_tr1, mt_tr2, mt_tr3;
int taskctl;
int tasknum = 0;  // 0任务（GDT103*8）留给系统
int cg_flag = 0;
int cg_flag0 = 0;  // 需不需要调用C_G()
struct TASK* c_task = NULL;
#define EFLAGS_VM_MASK 0x00020000
void mt_init(void) {
  extern struct TASK normal;
  // tasknum = 0;
  // 对多任务调度的初始化
  strcpy(normal.name, "Normal");
  normal.sel = 103 * 8;
  normal.level = 1;
  normal.sleep = 1;
  normal.fifosleep = 1;
  normal.keyfifo = NULL;
  normal.mousefifo = NULL;
  mt_tr3 = 103 * 8;
  mt_tr2 = 103 * 8;
  mt_tr1 = 103 * 8;
  mt_timer1 = timer_alloc();
  timer_settime(mt_timer1, 1);
  mt_timer2 = timer_alloc();
  timer_settime(mt_timer2, 3);
  mt_timer3 = timer_alloc();
  timer_settime(mt_timer3, 8);
}
void mt_taskswitch1() {
  // printk("mt1\n");
  if (Get_Running_Task_Num() == 0) {
    printk("Start idle task.\n");
    WakeUp(GetTask(1));  // 启动idle task
    timer_settime(mt_timer1, 1);
    return;
  }
  if (mt_tr1 == 103 * 8) {
    if (cg_flag0) {
      io_cli();
      for (int i = 1; GetTask(i) != 0; i++) {
        // printk("Set %s Task %d To
        // %d\n",GetTask(i)->name,GetTask(i)->level,GetTask(i)->nl);
        GetTask(i)->level = GetTask(i)->nl;
      }
      mt_tr2 = 103 * 8;
      mt_tr3 = 103 * 8;
      taskctl = 103 * 8;
      cg_flag0 = 0;
      io_sti();
    }
  }
  if (Get_Running_Task_Num() == 1) {
    // 无优先级1的任务
    // printk("Let?\n");
    if (mt_tr1 == 103 * 8) {
      mt_tr1 = 104 * 8;
      taskctl = mt_tr1;
      timer_settime(mt_timer1, 1);
      farjmp(0, taskctl);
    }
    timer_settime(mt_timer1, 1);
    // 没必要切换了
    return;
  }
  for (int i = mt_tr1 / 8 - 103; i < tasknum + 1; i++) {
    // 优先级1的执行
    struct TASK* task = GetTask(mt_tr1 / 8 - 103);
    if (task->level == 1 && task->sleep == 0 && task->lock == 0 &&
        task->running) {  // 找到level=1的任务并且没有休眠
      // task->level = task->nl;
      if (taskctl == mt_tr1) {
        mt_tr1 += 8;
        timer_settime(mt_timer1, 1);
        return;
      }
      taskctl = mt_tr1;
      mt_tr1 += 8;
      timer_settime(mt_timer1, 1);
      farjmp(0, taskctl);
      return;
    }
    mt_tr1 += 8;
  }
  // 无优先级1的任务
  mt_tr1 = 103 * 8;  // mt_tr1复原
  timer_settime(mt_timer1, 1);
}
void mt_taskswitch2() {
  // printk("mt2\n");
  // asm volatile ("cli");
  if (Get_Running_Task_Num() <= 1) {
    timer_settime(mt_timer2, 3);
    timer_settime(mt_timer1, 1);
    return;
  }

  for (int i = mt_tr2 / 8 - 103; i < tasknum + 1; i++) {
    // 优先级2的执行
    struct TASK* task = GetTask(mt_tr2 / 8 - 103);
    if (task->level == 2 && task->sleep == 0 && task->lock == 0 &&
        task->running) {  // 找到level=2的任务并且没有休眠
                          // task->level = task->nl;
      if (taskctl == mt_tr2) {
        mt_tr2 += 8;
        timer_settime(mt_timer2, 1);
        return;
      }
      taskctl = mt_tr2;
      mt_tr2 += 8;
      //   printk("Level:2 Name:%s Task:%d*8
      //   EIP:%08x\n",task->name,taskctl/8,task->tss.eip);
      timer_settime(mt_timer2, 3);
      timer_settime(mt_timer1, 1);
      farjmp(0, taskctl);  // 跳转
      // asm volatile ("sti");
      // putchar('\0');
      return;
    }
    mt_tr2 += 8;
  }
  // 无优先级2的任务
  mt_tr2 = 103 * 8;  // mt_tr2复原
  timer_settime(mt_timer2, 3);
  timer_settime(mt_timer1, 1);
  // asm volatile ("sti");
}
void mt_taskswitch3() {
  // printk("mt3\n");
  extern int mt2flag;
  if (Get_Running_Task_Num() <= 1) {
    timer_settime(mt_timer3, 8);
    if (mt2flag == 1) {
      timer_settime(mt_timer2, 3);
    }
    timer_settime(mt_timer1, 1);
    return;
  }
  for (int i = mt_tr3 / 8 - 103; i < tasknum + 1; i++) {
    // 优先级3的执行
    struct TASK* task = GetTask(mt_tr3 / 8 - 103);
    if (task->level == 3 && task->sleep == 0 && task->lock == 0 &&
        task->running) {  // 找到level=3的任务并且没有休眠
                          // task->level = task->nl;
      if (taskctl == mt_tr3) {
        mt_tr3 += 8;
        timer_settime(mt_timer3, 1);
        return;
      }
      taskctl = mt_tr3;
      mt_tr3 += 8;
      //  printk("Level:3 Name:%s Task:%d*8
      //  EIP:%08x\n",task->name,taskctl/8,task->tss.eip);
      timer_settime(mt_timer3, 8);
      if (mt2flag == 1) {
        timer_settime(mt_timer2, 3);
      }
      timer_settime(mt_timer1, 1);
      farjmp(0, taskctl);  // 跳转
      return;
    }
    mt_tr3 += 8;
  }
  // 无优先级3的任务
  mt_tr3 = 103 * 8;  // mt_tr3复原
  timer_settime(mt_timer3, 8);
  if (mt2flag == 1) {
    timer_settime(mt_timer2, 3);
  }
  timer_settime(mt_timer1, 1);
}
int Get_Running_Task_Num() {
  int ret = 0;
  for (int i = 0; i <= tasknum; i++) {
    if (GetTask(i)->sleep == 0) {
      ret++;
    }
  }
  // printk("ret = %d\n",ret);
  return ret;
}
struct TASK* NowTask() {
  // while(taskctl != 103*8 && tasknum < 2);
  return GetTask(taskctl / 8 - 103);
}
void TaskSetFIFO(struct TASK* task,
                 struct FIFO8* keyfifo,
                 struct FIFO8* mousefifo) {
  task->keyfifo = keyfifo;
  task->mousefifo = mousefifo;
}
struct FIFO8* TaskGetKeyfifo(struct TASK* task) {
  return (struct FIFO8*)task->keyfifo;
}
struct FIFO8* TaskGetMousefifo(struct TASK* task) {
  return (struct FIFO8*)task->mousefifo;
}
static bool flags_once = false;
static char default_drive;
static unsigned int default_drive_number;
struct TASK*
AddTask(char* name, int level, int cs, int eip, int ds, int ss, int esp) {
  if (!flags_once) {
    if (*(unsigned char*)(0x7c00 + BS_DrvNum) >= 0x80) {
      default_drive_number =
          *(unsigned char*)(0x7c00 + BS_DrvNum) - 0x80 + 0x02;
    } else {
      default_drive_number = *(unsigned char*)(0x7c00 + BS_DrvNum);
    }
    default_drive = default_drive_number + 0x41;
    flags_once = true;
  }
  Maskirq(0);
  tasknum++;
  struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
  struct TASK* task = (struct TASK*)page_kmalloc(sizeof(struct TASK));
  // GDT 0~2 系统用
  // GDT 3~103 程序用
  // GDT 104 ~ TSS用
  set_segmdesc(gdt + 103 + tasknum, 103, (int)&task->tss, AR_TSS32);
  // 必填
  task->IPC_header.now = 0;
  for (int i = 0; i != MAX_IPC_MESSAGE; i++) {
    task->IPC_header.data[i] = 0;
    task->IPC_header.size[i] = 0;
    task->IPC_header.from_tid[i] = 0;
  }
  task->sel = (103 + tasknum) * 8;
  task->level = level;
  task->nl = level;
  task->sleep = 0;  // 这个任务没有休眠
  task->lock = 0;   // 这个任务没有被锁住
  task->fifosleep = 0;
  task->tss.eflags = 0x00001202;
  task->tss.cr3 = PDE_ADDRESS;  // PAE分页（二级分页
  task->tss.eax = 0;
  task->tss.ebx = 0;
  task->tss.ecx = 0;
  task->tss.edx = 0;
  task->tss.esp = esp;
  task->tss.ebp = 0;
  task->tss.esi = 0;
  task->tss.edi = 0;
  task->tss.ds = GET_SEL(ds,SA_RPL0);
  task->tss.es = GET_SEL(ds,SA_RPL0);
  task->tss.fs = GET_SEL(ds,SA_RPL0);
  task->tss.gs = GET_SEL(ds,SA_RPL0);
  task->tss.ss = GET_SEL(ss,SA_RPL0);
  task->tss.cs = GET_SEL(cs,SA_RPL0);
  task->tss.eip = eip;
  task->tss.ldtr = 0;
  task->tss.iomap = 0x40000000;
  task->running = 1;
  task->esp_start = esp;  // 为fork做准备
  task->eip_start = eip;  // 为clone task做准备
  task->CatchEIP = 0;
  task->DisableExpFlag = 0;
  task->flagOfexp = 0;
  task->drive_number = default_drive_number;
  task->drive = default_drive;
  task->directory = drive_ctl.drives[task->drive_number].root_directory;
  task->change_dict_times = 0;
  task->app = 0;
  // 应用程序选填
  task->ds_base = 0;
  task->cs_base = 0;
  task->alloc_addr = 0;
  task->memman = 0;
  task->esp0 = 0;
  task->esp1 = 0;
  task->ss1 = 0;
  task->is_child = 0;
  task->keyboard_press = NULL;
  task->keyboard_release = NULL;
  task->thread.father = NULL;
  task->mx = 0;
  task->my = 0;
  if (strlen(name) > 32) {
    strncpy(task->name, name, 32);
  } else {
    strcpy(task->name, name);
  }
  ClearMaskIrq(0);
  return task;
}
struct TASK*
AddUserTask(char* name, int level, int cs, int eip, int ds, int ss, int esp) {
  if (!flags_once) {
    if (*(unsigned char*)(0x7c00 + BS_DrvNum) >= 0x80) {
      default_drive_number =
          *(unsigned char*)(0x7c00 + BS_DrvNum) - 0x80 + 0x02;
    } else {
      default_drive_number = *(unsigned char*)(0x7c00 + BS_DrvNum);
    }
    default_drive = default_drive_number + 0x41;
    flags_once = true;
  }
  Maskirq(0);
  tasknum++;
  struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
  struct TASK* task = (struct TASK*)page_kmalloc(sizeof(struct TASK));
  // GDT 0~2 系统用
  // GDT 3~103 程序用
  // GDT 104 ~ TSS用
  set_segmdesc(gdt + 103 + tasknum, 103, (int)&task->tss, AR_TSS32);
  // 必填
  task->IPC_header.now = 0;
  for (int i = 0; i != MAX_IPC_MESSAGE; i++) {
    task->IPC_header.data[i] = 0;
    task->IPC_header.size[i] = 0;
    task->IPC_header.from_tid[i] = 0;
  }
  task->sel = (103 + tasknum) * 8;
  task->level = level;
  task->nl = level;
  task->sleep = 0;  // 这个任务没有休眠
  task->lock = 0;   // 这个任务没有被锁住
  task->fifosleep = 0;
  task->tss.eflags = 0x00000202;
  task->tss.cr3 = PDE_ADDRESS;  // PAE分页（二级分页
  task->tss.eax = 0;
  task->tss.ebx = 0;
  task->tss.ecx = 0;
  task->tss.edx = 0;
  task->tss.esp = esp;
  task->tss.ebp = 0;
  task->tss.esi = 0;
  task->tss.edi = 0;
  task->tss.ds = GET_SEL(ds,SA_RPL3);
  task->tss.es = GET_SEL(ds,SA_RPL3);
  task->tss.fs = GET_SEL(ds,SA_RPL3);
  task->tss.gs = GET_SEL(ds,SA_RPL3);
  task->tss.ss = GET_SEL(ss,SA_RPL3);
  task->tss.cs = GET_SEL(cs,SA_RPL3);
  task->tss.eip = eip;
  task->tss.ldtr = 0;
  task->tss.iomap = 0x40000000;
  task->running = 1;
  task->esp_start = esp;  // 为fork做准备
  task->eip_start = eip;  // 为clone task做准备
  task->CatchEIP = 0;
  task->DisableExpFlag = 0;
  task->flagOfexp = 0;
  task->drive_number = default_drive_number;
  task->drive = default_drive;
  task->directory = drive_ctl.drives[task->drive_number].root_directory;
  task->change_dict_times = 0;
  task->app = 0;
  // 之后填写
  task->ds_base = 0;
  task->cs_base = 0;
  task->alloc_addr = 0;
  task->memman = 0;
  task->esp0 = 0;
  task->esp1 = 0;
  task->ss1 = 0;
  task->is_child = 0;
  task->keyboard_press = NULL;
  task->keyboard_release = NULL;
  task->thread.father = NULL;
  task->mx = 0;
  task->my = 0;
  if (strlen(name) > 32) {
    strncpy(task->name, name, 32);
  } else {
    strcpy(task->name, name);
  }
  ClearMaskIrq(0);
  return task;
}
void SleepTask(struct TASK* task) {
  // printk("sleeptask\n");
  if (Get_Running_Task_Num() == 1)
    return;
  if (!task->is_child) {
    // 我是你爹，我睡了你也必须睡
    for (int i = 0; GetTask(i) != NULL; i++) {
      struct TASK* t = GetTask(i);
      if (t->is_child == 1 && t->thread.father == task) {
        t->sleep = 1;
        t->fifosleep = 1;
      }
    }
  }
  task->sleep = 1;  // 任务休眠
  task->fifosleep = 1;
}
void WakeUp(struct TASK* task) {
  if (!task->is_child) {
    // 同样的，我是你爸，我都起了你敢不起？
    if (!task->sleep && !task->fifosleep) {
      return;
    }
    for (int i = 0; GetTask(i) != NULL; i++) {
      struct TASK* t = GetTask(i);
      if (t->is_child == 1 && t->thread.father == task) {
        t->sleep = 0;
        t->fifosleep = 0;
      }
    }
    task->sleep = 0;
    task->fifosleep = 0;
  } else {
    task->sleep = 0;
    task->fifosleep = 0;
    task->thread.father->fifosleep =
        0;  // 我管你起没起，反正我吃饭你还得管
            // p.s. kfifo和mfifo 子线程和父线程用的是一套
  }
  return;
}
void SleepTaskFIFO(struct TASK* task) {
  /* 子线程和父线程共用FIFO */
  if (!task->is_child) {
    task->fifosleep = 1;
  } else {
    task->thread.father->fifosleep = 1;
  }
}
struct TASK* GetTask(int taskNum) {
  if (taskNum > tasknum)
    return 0;
  struct SEGMENT_DESCRIPTOR* gdt =
      (struct SEGMENT_DESCRIPTOR*)ADR_GDT + (103 + taskNum);

  //	printf("gdt->base_low:%04x\n",(unsigned short)gdt->base_low);
  //	printf("gdt->base_mid:%02x\n",(unsigned char)gdt->base_mid);
  // printf("gdt->base_high:%02x\n",(unsigned char)gdt->base_high);
  //将他们转换成32位的值
  unsigned char res1[4];

  //注意大小端
  res1[0] = ((unsigned char*)&gdt->base_low)[0];
  res1[1] = ((unsigned char*)&gdt->base_low)[1];
  res1[2] = (unsigned char)gdt->base_mid;
  res1[3] = (unsigned char)gdt->base_high;
  unsigned int res2 = *(unsigned int*)res1;
  return (struct TASK*)(res2 - 0xc);
}
struct TASK* GetTaskForName(char* taskname) {
  for (int i = 0; i <= tasknum; i++) {
    if (strcmp(GetTask(i)->name, taskname) == 0) {
      return GetTask(i);
    }
  }
  return 0;
}
struct TASK* GetTask_NoSafe(int taskNum) {
  // if (taskNum > tasknum)
  //   return 0;
  struct SEGMENT_DESCRIPTOR* gdt =
      (struct SEGMENT_DESCRIPTOR*)ADR_GDT + (103 + taskNum);

  //	printf("gdt->base_low:%04x\n",(unsigned short)gdt->base_low);
  //	printf("gdt->base_mid:%02x\n",(unsigned char)gdt->base_mid);
  // printf("gdt->base_high:%02x\n",(unsigned char)gdt->base_high);
  //将他们转换成32位的值
  unsigned char res1[4];

  //注意大小端
  res1[0] = ((unsigned char*)&gdt->base_low)[0];
  res1[1] = ((unsigned char*)&gdt->base_low)[1];
  res1[2] = (unsigned char)gdt->base_mid;
  res1[3] = (unsigned char)gdt->base_high;
  unsigned int res2 = *(unsigned int*)res1;
  return (struct TASK*)(res2 - 0xc);
}
void __SubTask(struct TASK* task) {
  // Maskirq(0);  // 关闭时钟中断 防止有任务来打扰
  // SleepTask(task);
  // struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
  // int bmpNum = tasknum;
  // int index = tasknum - (task->sel / 8 - 103) - 1;
  // tasknum -= index;
  // if (index == 0) {  // 最后一个
  //   // gdt描述符清空
  //   tasknum--;
  //   set_segmdesc(gdt + 103 + bmpNum, 0, 0, 0);
  //   ClearMaskIrq(0);
  //   return;
  // }
  // // printf("\nTASKSEL:%d*8 TASKNUM:%d INDEX:%d
  // // BMPNUM:%d\n",task->sel/8,tasknum,index,bmpNum);
  // page_kfree((int)task, sizeof(struct TASK));
  // for (int i = bmpNum - index + 1; i < bmpNum + index; i++) {
  //   page_kfree((int)GetTask_NoSafe(i), sizeof(struct TASK));
  //   struct TASK* ntask =
  //       AddTask(GetTask_NoSafe(i)->name, GetTask_NoSafe(i)->level,
  //               GetTask_NoSafe(i)->tss.cs, GetTask_NoSafe(i)->tss.eip,
  //               GetTask_NoSafe(i)->tss.ds, GetTask_NoSafe(i)->tss.ss,
  //               GetTask_NoSafe(i)->tss.esp);
  //   ntask->sleep = GetTask_NoSafe(i)->sleep;
  //   ntask->fifosleep = GetTask_NoSafe(i)->fifosleep;
  //   ntask->keyfifo = GetTask_NoSafe(i)->keyfifo;
  //   ntask->mousefifo = GetTask_NoSafe(i)->mousefifo;
  //   ntask->IPC_header = GetTask_NoSafe(i)->IPC_header;
  // }
  // tasknum -= 2;
  // ClearMaskIrq(0);
  io_cli();
  for (int i = 0; GetTask(i) != NULL; i++) {
    struct TASK* t = GetTask(i);
    if (t->is_child == 1 && t->thread.father == task) {
      __SubTask(t);
    }
  }
  struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
  int index = tasknum - (task->sel / 8 - 103) + 1;
  tasknum -= index;
  int bmpNum = tasknum;
  for (int i = task->sel / 8 - 103 + 1; i <= bmpNum + index; i++) {
    tasknum++;
    GetTask_NoSafe(i)->sel = (103 + tasknum) * 8;
    set_segmdesc(gdt + 103 + tasknum, 103, (int)&GetTask_NoSafe(i)->tss,
                 AR_TSS32);
  }
  extern struct PAGE_INFO* pages;
  for (int i = 0; i != memsize / (4 * 1024); i++) {
    if (pages[i].flag == 1 && pages[i].task_id == task->sel / 8 - 103) {
      int t, p;
      page2tpo(i, &t, &p);
      void* ptr = (void*)get_line_address(t, p, 0);
      // printk("SubTask Free a page:%08x\n",ptr);
      page_free_one(ptr);
    }
  }
  page_kfree((int)task, sizeof(struct TASK));
  io_sti();
}
void ChangeLevel(struct TASK* task, int nlevel) {
  if (task == NULL) {
    // 你找茬是吧，给个NULL进来
    return;
  }
  if (task->level == nlevel) {
    return;
  }
  if (GetTask(Get_Tid(task)) != task) {
    return;  // 防止修改一个不存在的进程
  }
  io_cli();
  task->nl = nlevel;
  cg_flag0 = 1;
  io_sti();
  while (task->level != nlevel)
    ;
}
void RunTask(struct TASK* task) {
  io_cli();
  switch (task->level) {
    case 1:
      mt_tr1 = task->sel;
      break;
    case 2:
      mt_tr2 = task->sel;
      break;
    case 3:
      mt_tr3 = task->sel;
      break;
  }
  io_sti();
}
struct TASK* _fork(int b) {
  int ESP;
  asm("movl %%ebp,%0" : "=g"(ESP));
  int* eipa = &b;
  ESP = (int)eipa;
  // printk("eipa=%08x\n", eipa[-1]);
  Maskirq(0);
  if (NowTask()->is_child) {
    io_sti();
    ClearMaskIrq(0);
    return NULL;  // 子进程返回0
  }

  int* eip = &b;
  int EIP = eip[-1];
  // printk("eip=%08x\n", EIP);
  int esp = NowTask()->tss.esp;
  (void)(esp);
  int sz = NowTask()->esp_start - ESP;
  uint8_t* stack = (uint8_t*)page_malloc(32 * 1024);  // 分配32KB内存用作栈空间
  stack += 32 * 1024;
  stack -= sz;
  // printk("stack=%08x\n", stack);
  memcpy(stack, (void*)ESP, sz);
  stack += sz;

  struct TASK* task =
      AddTask(NowTask()->name, NowTask()->level, NowTask()->tss.cs, EIP,
              NowTask()->tss.ds, 1 * 8, (int)((uint32_t)stack - sz));
  Maskirq(0);
  char* memman = (char*)page_kmalloc(4 * 1024);
  int alloc_addr = (int)page_kmalloc(512 * 1024);
  task->alloc_addr = alloc_addr;
  task->alloc_size = 512 * 1024;
  task->memman = memman;
  task->directory = NowTask()->directory;
  task->drive = NowTask()->drive;
  task->drive_number = NowTask()->drive_number;
  task->change_dict_times = NowTask()->change_dict_times;
  task->TTY = NowTask()->TTY;
  strcpy(task->path, NowTask()->path);
  char* kfifo = (char*)page_kmalloc(sizeof(struct FIFO8));
  char* mfifo = (char*)page_kmalloc(sizeof(struct FIFO8));
  char* kbuf = (char*)page_kmalloc(4096);
  char* mbuf = (char*)page_kmalloc(4096);
  fifo8_init((struct FIFO8*)kfifo, 4096, (unsigned char*)kbuf);
  fifo8_init((struct FIFO8*)mfifo, 4096, (unsigned char*)mbuf);
  TaskSetFIFO(task, (struct FIFO8*)kfifo, (struct FIFO8*)mfifo);
  change_page_task_id(task->sel / 8 - 103, stack - 32 * 1024, 32 * 1024);
  change_page_task_id(task->sel / 8 - 103, memman, 4 * 1024);

  task->is_child = 1;
  task->thread.father = NowTask();

  ClearMaskIrq(0);
  return task;
}
void SubTask(struct TASK* task) {
  int tid = Get_Tid(task);
  task->running = 0;   // 不在运行状态中
  WakeUp(GetTask(1));  // 叫醒idle task
  while (GetTask(tid))
    ;
}
void c_g() {
  if (!cg_flag0 && !cg_flag) {
    return;
  }
  io_cli();

  for (int i = 1; GetTask(i) != 0; i++) {
    // printk("Need.\n");
    // printf("Task #%d: Level %d -->
    // Level%d\n",i,GetTask(i)->level,GetTask(i)->nl);
    GetTask(i)->level = GetTask(i)->nl;
  }
  cg_flag0 = 0;
  io_sti();
}
static struct TASK* __clone_task(struct TASK* tk, int stack_sz) {
  Maskirq(0);
  void* stack = page_malloc(stack_sz);
  struct TASK* result = AddTask(tk->name, tk->level, tk->tss.cs, tk->eip_start,
                                tk->tss.ds, tk->tss.ss, (int)stack);
  change_page_task_id(Get_Tid(result), stack, stack_sz);
  ClearMaskIrq(0);
  return result;
}
struct TASK* clone_task(struct TASK* tk, int stack_sz) {
  if (tk->is_child) {
    // is a thread
    return NULL;  // thread还想clone？鬼！
  }
  io_cli();
  struct TASK* father = __clone_task(tk, stack_sz);
  for (int i = 1; i < tasknum + 1; i++) {
    if (GetTask(i)->is_child) {
      if (GetTask(i)->thread.father == tk) {
        struct TASK* ttask = __clone_task(GetTask(i), stack_sz);
        ttask->is_child = 1;
        ttask->thread.father = father;
      }
    }
  }
  io_sti();
  return father;
}
void TaskLock() {
  io_cli();                        // 保证原子操作
  if (NowTask()->is_child == 0) {  // 父进程
    for (int i = 1; GetTask(i) != 0; i++) {
      if (GetTask(i)->thread.father == NowTask()) {
        GetTask(i)->lock = 1;  // 锁住他，不让他运行，和此任务抢资源
      }
    }
  } else {
    for (int i = 1; GetTask(i) != 0; i++) {
      if (GetTask(i)->thread.father == NowTask()->thread.father ||
          GetTask(i) == NowTask()->thread.father) {
        if (GetTask(i) != NowTask()) {
          GetTask(i)->lock = 1;  // 锁住他，不让他运行，和此任务抢资源
        }
      }
    }
  }
  io_sti();
}
void TaskUnLock() {
  io_cli();                        // 保证原子操作
  if (NowTask()->is_child == 0) {  // 父进程
    for (int i = 1; GetTask(i) != 0; i++) {
      if (GetTask(i)->thread.father == NowTask()) {
        GetTask(i)->lock = 0;
      }
    }
  } else {
    for (int i = 1; GetTask(i) != 0; i++) {
      if (GetTask(i)->thread.father == NowTask()->thread.father ||
          GetTask(i) == NowTask()->thread.father) {
        if (GetTask(i) != NowTask()) {
          GetTask(i)->lock = 0;
        }
      }
    }
  }
  io_sti();
}