#include <dos.h>
#include <upng.h>
struct TASK* shell_task;
struct TASK *sr1, *sr2;
struct TASK normal;
unsigned int memsize;
unsigned int PCI_ADDR_BASE;
unsigned int ADR_DISKIMG = 0x00100000;
struct tty tty_console[6];
struct MOUSE_DEC mdec;
char drive;
int autoexec = 0;
void sysinit(void) {
  static char fxsave_region[512] __attribute__((aligned(16)));
  asm volatile(" fxsave %0 " ::"m"(fxsave_region));
  struct FIFO8 keyfifo, mousefifo;
  struct FIFO8 keyfifo_sr1, keyfifo_sr2;
  struct FIFO8 mousefifo_sr1, mousefifo_sr2;
  char keybuf[32];
  char mousebuf[128];
  char keybuf_sr1[32];
  char mousebuf_sr1[128];
  char keybuf_sr2[32];
  char mousebuf_sr2[128];
  struct SEGMENT_DESCRIPTOR* gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
  drive = *(char*)(0x7d78);
  clear();
  init_page();  // 初始化分页
  init_gdtidt();
  init_pic();
  io_sti();
  ClearMaskIrq(0);   // pit (timer)
  ClearMaskIrq(1);   // keyboard
  ClearMaskIrq(12);  // mouse
  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 128, mousebuf);
  fifo8_init(&keyfifo_sr1, 32, keybuf_sr1);
  fifo8_init(&mousefifo_sr1, 128, mousebuf_sr1);
  fifo8_init(&keyfifo_sr2, 32, keybuf_sr2);
  fifo8_init(&mousefifo_sr2, 128, mousebuf_sr2);
  init_keyboard();
  enable_mouse(&mdec);
  mouse_sleep(&mdec);
  init_pit();
  init_rtc();
  memsize = memtest(0x00400000, 0xbfffffff);
  if (memsize / (1024 * 1024) < 32) {
    printf("\nMemory Size:%dMB\n", memsize / (1024 * 1024));
    printf("MEMORY SIZE TOO SMALL!\n");
    printf(
        "If you want to start Powerint DOS,must change memory size to 32MB "
        "or more.\n");
    if (memsize / (1024 * 1024) < 16) {
      while (1) {
        beep(3, 7, 5);
        sleep(100);
      }
    }
    printf("This is a warning,please wait 3 seconds to continue...");
    sleep(3000);
	autoexec = 1;	// 不执行AUTOEXEC.BAT
  }
  clear();
  init_acpi();
  init_floppy();
  disable_sb16();
  Input_Stack_Init();
  init_driver();
  normal.tss.ldtr = 0;
  normal.tss.iomap = 0x40000000;
  set_segmdesc(gdt + 103, 103, (int)&normal.tss, AR_TSS32);
  load_tr(103 * 8);
  sr1 = AddTask("System retention", 1, 2 * 8, (int)task_sr1, 1 * 8, 1 * 8,
                (unsigned int)page_kmalloc(64 * 1024) + 64 * 1024);
  sr2 = AddTask("System retention", 1, 2 * 8, (int)task_sr2, 1 * 8, 1 * 8,
                (unsigned int)page_kmalloc(64 * 1024) + 64 * 1024);
  shell_task = AddTask("Shell", 1, 2 * 8, (int)shell, 1 * 8, 1 * 8,
                       (unsigned int)page_kmalloc(128 * 1024) + 128 * 1024);
  // printk("task_sr1:%08x\n",sr1);
  //给每个任务设置FIFO
  TaskSetFIFO(shell_task, &keyfifo, &mousefifo);
  TaskSetFIFO(sr1, &keyfifo_sr1, &mousefifo_sr1);
  TaskSetFIFO(sr2, &keyfifo_sr2, &mousefifo_sr2);
  // 给Shell设置alloc_addr和memman配合lessthan4kb
  int alloc_addr = (int)page_kmalloc(512 * 1024);
  char* memman = (char*)page_kmalloc(4 * 1024);
  shell_task->alloc_addr = alloc_addr;
  shell_task->memman = memman;
  srand(time());  // Init random seed
  // while(1) printk("%08x\n",rand());
  clear();
  printf("Loading Powerint DOS...\n");
#ifdef __READ__ALL__
  // sleep(1000);
  read_floppy_disk_all();  //系统初始化的最后一步，读取所有软盘的数据
#endif
  while (fifo8_status(TaskGetKeyfifo(shell_task)) != 0)
    fifo8_get(TaskGetKeyfifo(shell_task));
  while (fifo8_status(TaskGetKeyfifo(sr1)) != 0)
    fifo8_get(TaskGetKeyfifo(sr1));
  while (fifo8_status(TaskGetKeyfifo(sr2)) != 0)
    fifo8_get(TaskGetKeyfifo(sr2));
  PCI_ADDR_BASE = (unsigned int)page_kmalloc(1 * 1024 * 1024);
  init_PCI(PCI_ADDR_BASE);
  // printf("%08x\n", PCI_ADDR_BASE);
  init_float();
  mt_init();
  for (;;)
    ;
}
void save_all_floppy() {
  // save_all_floppy：保存所有磁盘
  Maskirq(0);                       // 暂时不需要任务切换
  int t = 0;                        //磁道
  int h = 0;                        //磁头
  int s = 1;                        //扇区
  char* data = (char*)ADR_DISKIMG;  //磁盘数据储存在的内存地址
  char* ing = "[          ]";       //进度条
  for (; t < 80; t++)               //软盘一共80个磁道
  {
    // 将光标xy设置为本行的开头
    gotoxy(0, get_y());
    printf("%s %d%%", ing, t * 5 / 4);  // 显示进度条 以及百分比
    gotoxy(1, get_y());  // 将光标xy设置为本行的开头第一个位置
    for (int i = 0; i != t * 5 / 40; i++) {
      printchar('#');  // 显示进度条
    }
    //文件的保存
    for (; h < 2; h++) {
      for (; s < 19; s++) {
        write_floppy_for_ths(t, h, s, data, 1);  //写入磁盘
        data += 512;                             // 指向下一个扇区
      }
      s = 1;  //重定位S
    }
    h = 0;  //重定位H
  }
  ClearMaskIrq(0);
}
void read_floppy_disk_all() {
  unsigned char* buf = (unsigned char*)page_kmalloc(2880 * 512);
  int buf2 = buf;
  int t = 0;  //磁道
  int h = 0;  //磁头
  int s = 1;  //扇区

  char* ing = "[          ]";  //进度条
  for (; t < 80; t++)          //软盘一共80个磁道
  {
    // 将光标xy设置为本行的开头
#ifdef __READ__ALL__
    gotoxy(0, get_y());
    printf("%s %d%%", ing, t * 5 / 4);  // 显示进度条 以及百分比
    gotoxy(1, get_y());  // 将光标xy设置为本行的开头第一个位置
    for (int i = 0; i != t * 5 / 40; i++) {
      printchar('#');  // 显示进度条
    }
#endif
    //软盘的读取
    for (; h < 2; h++) {
      for (; s < 19; s++) {
        if (fdc_rw_ths(t, h, s, buf, 1, 1) != 1) {
          Panic_F("[ !!!!!! ] Read Error !!! ");
          Panic_K("[ !!!!!! ] Read Error !!! ");
          for (;;)
            ;
        }
#ifdef __READ__ALL__
        gotoxy(strlen(ing) + 5, get_y());
        printf("T:%02d H:%02d S:%02d ADDRESS: %08x", t, h, s, buf);
#endif
        buf += 512;  // 指向下一个扇区
      }
      s = 1;  //重定位S
    }
    h = 0;  //重定位H
  }
  ADR_DISKIMG = buf2;
}