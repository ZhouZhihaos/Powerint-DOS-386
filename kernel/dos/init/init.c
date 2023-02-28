#include <dos.h>
extern struct ide_device {
  unsigned char Reserved;      // 0 (Empty) or 1 (This Drive really exists).
  unsigned char Channel;       // 0 (Primary Channel) or 1 (Secondary Channel).
  unsigned char Drive;         // 0 (Master Drive) or 1 (Slave Drive).
  unsigned short Type;         // 0: ATA, 1:ATAPI.
  unsigned short Signature;    // Drive Signature
  unsigned short Capabilities; // Features.
  unsigned int CommandSets;    // Command Sets Supported.
  unsigned int Size;           // Size in Sectors.
  unsigned char Model[41];     // Model in string.
} ide_devices[4];
struct TASK *shell_task;
struct TASK *sr1, *sr2;
struct TASK normal;
unsigned int memsize;
unsigned int PCI_ADDR_BASE;
struct MOUSE_DEC mdec;
extern unsigned char *IVT;
void init_page(void);
void disable_sb16(void);
void init_mount_disk(void);
int getReadyDisk();
void Socket_all_init();
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
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  init_page(); // 初始化分页
  init_gdtidt();
  IVT = page_kmalloc(0x400);
  memcpy(IVT, 0x0, 0x400);
  init_tty();
  clear();
  init_pic();
  io_sti();
  ClearMaskIrq(0);  // pit (timer)
  ClearMaskIrq(1);  // keyboard
  ClearMaskIrq(12); // mouse
  fifo8_init(&keyfifo, 32, (unsigned char *)keybuf);
  fifo8_init(&mousefifo, 128, (unsigned char *)mousebuf);
  fifo8_init(&keyfifo_sr1, 32, (unsigned char *)keybuf_sr1);
  fifo8_init(&mousefifo_sr1, 128, (unsigned char *)mousebuf_sr1);
  fifo8_init(&keyfifo_sr2, 32, (unsigned char *)keybuf_sr2);
  fifo8_init(&mousefifo_sr2, 128, (unsigned char *)mousebuf_sr2);
  init_keyboard();
  enable_mouse(&mdec);
  mouse_sleep(&mdec);
  init_pit();
  init_rtc();
  memsize = memtest(0x00400000, 0xbfffffff);
  if (memsize / (1024 * 1024) < 32) {
    while (1) {
      beep(3, 7, 5);
      sleep(100);
    }
  }
  pf_set(memsize);
  init_acpi();
  init_floppy();
  disable_sb16();
  Input_Stack_Init();
  Socket_all_init();
  init_driver();
  init_vdisk();
  init_mount_disk();
  SetDrive((unsigned char *)"FLOPPY_DRIVE");
  SetDrive((unsigned char *)"IDE_DRIVE");
  SetDrive((unsigned char *)"VDISK_DRIVE");
  SetDrive((unsigned char *)"NETCARD_DRIVE");
  normal.tss.ldtr = 0;
  normal.tss.iomap = 0x40000000;
  set_segmdesc(gdt + 103, 103, (int)&normal.tss, AR_TSS32);
  load_tr(103 * 8);
  sr1 = AddTask("System retention1", 1, 2 * 8, (int)task_sr1, 1 * 8, 1 * 8,
                (unsigned int)page_kmalloc(64 * 1024) + 64 * 1024);
  sr2 = AddTask("System retention2", 1, 2 * 8, (int)task_sr2, 1 * 8, 1 * 8,
                (unsigned int)page_kmalloc(64 * 1024) + 64 * 1024);
  shell_task = AddTask("Shell", 1, 2 * 8, (int)shell, 1 * 8, 1 * 8,
                       (unsigned int)page_kmalloc(128 * 1024) + 128 * 1024);
  //给每个任务设置FIFO
  TaskSetFIFO(shell_task, &keyfifo, &mousefifo);
  TaskSetFIFO(sr1, &keyfifo_sr1, &mousefifo_sr1);
  TaskSetFIFO(sr2, &keyfifo_sr2, &mousefifo_sr2);
  // 给Shell设置alloc_addr和memman配合lessthan4kb
  int alloc_addr = (int)page_kmalloc(512 * 1024);
  char *memman = (char *)page_kmalloc(4 * 1024);
  shell_task->alloc_addr = alloc_addr;
  shell_task->alloc_size = 512 * 1024;
  shell_task->memman = memman;
  srand(time()); // Init random seed
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

static unsigned char *drive_name[16] = {NULL, NULL, NULL, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL, NULL, NULL,
                                        NULL, NULL, NULL, NULL};
static struct FIFO8 drive_fifo[16];
static unsigned char drive_buf[16][256];
bool SetDrive(unsigned char *name) {
  for (int i = 0; i != 16; i++) {
    if (drive_name[i] == NULL) {
      drive_name[i] = name;
      fifo8_init(&drive_fifo[i], 256, drive_buf[i]);
      return true;
    }
  }
  return false;
}
unsigned int GetDriveCode(unsigned char *name) {
  for (int i = 0; i != 16; i++) {
    if (strcmp((char *)drive_name[i], (char *)name) == 0) {
      return i;
    }
  }
  return 16;
}

bool DriveSemaphoreTake(unsigned int drive_code) {
  if (drive_code >= 16) {
    return true;
  }
  fifo8_put(&drive_fifo[drive_code], Get_Tid(NowTask()));
  // printk("FIFO: %d PUT: %d STATUS: %d\n", drive_code, Get_Tid(NowTask()),
  //        fifo8_status(&drive_fifo[drive_code]));
  while (drive_buf[drive_code][drive_fifo[drive_code].q] != Get_Tid(NowTask()))
    ;
  return true;
}
void DriveSemaphoreGive(unsigned int drive_code) {
  if (drive_code >= 16) {
    return;
  }
  if (drive_buf[drive_code][drive_fifo[drive_code].q] != Get_Tid(NowTask())) {
    // 暂时先不做处理 一般不会出现这种情况
    return;
  }
  fifo8_get(&drive_fifo[drive_code]);
  // printk("FIFO: %d GET: %d STATUS: %d\n", drive_code, Get_Tid(NowTask()),
  //        fifo8_status(&drive_fifo[drive_code]));
}

void Disk_Read(unsigned int lba, unsigned int number, void *buffer,
               char drive) {
  if (drive == 'A') {
    if (DriveSemaphoreTake(GetDriveCode((unsigned char *)"FLOPPY_DRIVE"))) {
      for (int i = 0; i != number; i++) {
        fdc_rw(lba + i, (unsigned char *)(buffer + i * 512), 1, 1);
      }
      DriveSemaphoreGive(GetDriveCode((unsigned char *)"FLOPPY_DRIVE"));
    }
  } else if (drive != 'B') {
    unsigned char drive1 = drive;
    if (DiskReady(drive1)) {
      if (DriveSemaphoreTake(GetDriveCode((unsigned char *)"IDE_DRIVE"))) {
        ide_read_sectors(drive1 - 'C', number, lba, 8, (unsigned int)buffer);
        DriveSemaphoreGive(GetDriveCode((unsigned char *)"IDE_DRIVE"));
      }
    } else if (have_vdisk(drive1)) {
      if (DriveSemaphoreTake(GetDriveCode((unsigned char *)"VDISK_DRIVE"))) {
        rw_vdisk(drive1, lba, buffer, number, 1);
        DriveSemaphoreGive(GetDriveCode((unsigned char *)"VDISK_DRIVE"));
      }
    } else {
      printk("Disk Not Ready.\n");
    }
  }
}
int disk_Size(char drive) {
  if (drive == 'A') {
    return 2880 * 512;
  } else if (drive != 'B') {
    unsigned char drive1 = drive;
    if (DiskReady(drive1)) {
      return ide_devices[drive1 - 'C'].Size / 2 * 1024;
    } else if (have_vdisk(drive1)) {
      extern vdisk vdisk_ctl[255];
      int indx = drive1 - ('C' + getReadyDisk());
      return vdisk_ctl[indx].size;
    } else {
      printk("Disk Not Ready.\n");
      return 0;
    }
  }
  return 0;
}
bool DiskReady(char drive) {
  unsigned char drive1 = drive;
  drive1 -= 'C';
  // printk("Drive1=%d ide_devices[drive1].Reserved = %d\n",
  // drive1,ide_devices[drive1].Reserved);
  if (drive1 > 3) {
    //  printk("false\n");
    return false;
  }
  if (ide_devices[drive1].Reserved == 0) {
    return false;
  }
  // printk("true\n");
  return true;
}
int getReadyDisk() {
  for (int i = 0; i < 4; i++) {
    if (ide_devices[i].Reserved == 0) {
      return i;
    }
  }
  return 0;
}
void Disk_Write(unsigned int lba, unsigned int number, void *buffer,
                char drive) {
  if (drive == 'A') {
    if (DriveSemaphoreTake(GetDriveCode((unsigned char *)"FLOPPY_DRIVE"))) {
      for (int i = 0; i != number; i++) {
        fdc_rw(lba + i, (unsigned char *)(buffer + i * 512), 0, 1);
      }
      DriveSemaphoreGive(GetDriveCode((unsigned char *)"FLOPPY_DRIVE"));
    }
  } else if (drive != 'B') {
    unsigned char drive1 = drive;
    if (DiskReady(drive1)) {
      if (DriveSemaphoreTake(GetDriveCode((unsigned char *)"IDE_DRIVE"))) {
        ide_write_sectors(drive1 - 'C', number, lba, 8, (unsigned int)buffer);
        DriveSemaphoreGive(GetDriveCode((unsigned char *)"IDE_DRIVE"));
      }
    } else if (have_vdisk(drive1)) {
      if (DriveSemaphoreTake(GetDriveCode((unsigned char *)"VDISK_DRIVE"))) {
        rw_vdisk(drive1, lba, buffer, number, 0);
        DriveSemaphoreGive(GetDriveCode((unsigned char *)"VDISK_DRIVE"));
      }
    } else {
      printk("Disk Not Ready.\n");
    }
  }
}
