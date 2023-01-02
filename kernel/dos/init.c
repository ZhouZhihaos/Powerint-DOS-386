#include <dos.h>
extern struct ide_device {
  unsigned char Reserved;       // 0 (Empty) or 1 (This Drive really exists).
  unsigned char Channel;        // 0 (Primary Channel) or 1 (Secondary Channel).
  unsigned char Drive;          // 0 (Master Drive) or 1 (Slave Drive).
  unsigned short Type;          // 0: ATA, 1:ATAPI.
  unsigned short Signature;     // Drive Signature
  unsigned short Capabilities;  // Features.
  unsigned int CommandSets;     // Command Sets Supported.
  unsigned int Size;            // Size in Sectors.
  unsigned char Model[41];      // Model in string.
} ide_devices[4];
struct TASK* shell_task;
struct TASK *sr1, *sr2;
struct TASK normal;
unsigned int memsize;
unsigned int PCI_ADDR_BASE;
struct tty tty_console[6];
struct MOUSE_DEC mdec;
void sysinit(void) {
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
  if (memsize / (1024 * 1024) < 64) {
    printf("\nMemory Size:%dMB\n", memsize / (1024 * 1024));
    printf("MEMORY SIZE TOO SMALL!\n");
    printf(
        "If you want to start Powerint DOS,must change memory size to 64MB "
        "or more.\n");
    if (memsize / (1024 * 1024) < 32) {
      while (1) {
        beep(3, 7, 5);
        sleep(100);
      }
    }
    printf("This is a warning,please wait 3 seconds to continue...");
    sleep(3000);
  }
  pf_set(memsize);
  clear();
  init_acpi();
  init_floppy();
  disable_sb16();
  Input_Stack_Init();
  init_driver();
  init_vdisk();
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
void Disk_Read(unsigned int lba,
               unsigned int number,
               void* buffer,
               char drive) {
  if (drive == 'A') {
    for (int i = 0; i != number; i++) {
      fdc_rw(lba + i, (unsigned char*)(buffer + i * 512), 1, 1);
    }
  } else if (drive != 'B') {
    unsigned char drive1 = drive;
    if (DiskReady(drive1)) {
      ide_read_sectors(drive1 - 'C', number, lba, 8, buffer);
    } else if (have_vdisk(drive1)) {
      rw_vdisk(drive1, lba, buffer, number, 1);
    } else {
      printk("Disk Not Ready.\n");
    }
  }
}
int disk_Size(char drive) {
  if (drive == 'A') {
    return 2880*512;
  } else if (drive != 'B') {
    unsigned char drive1 = drive;
    if (DiskReady(drive1)) {
      return ide_devices[drive1-'C'].Size / 2 * 1024;
    } else if (have_vdisk(drive1)) {
      extern vdisk vdisk_ctl[255];
      int indx = drive1 - ('C' + getReadyDisk());
      return vdisk_ctl[indx].size;
    } else {
      printk("Disk Not Ready.\n");
      return 0;
    }
  }
}
bool DiskReady(char drive) {
  unsigned char drive1 = drive;
  drive1 -= 'C';
  // printk("Drive1=%d\n", drive1);
  if (drive1 > 3 || ide_devices[drive1].Reserved == 0) {
    return false;
  }
  return true;
}
int getReadyDisk() {
  for (int i = 0; i < 4; i++) {
    if (ide_devices[i].Reserved == 0) {
      return i;
    }
  }
}
void Disk_Write(unsigned int lba,
                unsigned int number,
                void* buffer,
                char drive) {
  if (drive == 'A') {
    for (int i = 0; i != number; i++) {
      fdc_rw(lba + i, (unsigned char*)(buffer + i * 512), 0, 1);
    }

  } else if (drive != 'B') {
    unsigned char drive1 = drive;
    if (DiskReady(drive1)) {
      ide_write_sectors(drive1 - 'C', number, lba, 8, buffer);
    } else if (have_vdisk(drive1)) {
      rw_vdisk(drive1, lba, buffer, number, 0);
    } else {
      printk("Disk Not Ready.\n");
    }
  }
}
