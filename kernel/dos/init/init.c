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
void socket_init();
void init_devfs();
void init_vfs();
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
  IVT = page_malloc(0x400);
  memcpy(IVT, 0x0, 0x400);
  init_tty();
  clear();
  init_pic();
  io_sti();
  irq_mask_clear(0);  // pit (timer)
  irq_mask_clear(1);  // keyboard
  irq_mask_clear(12); // mouse
  set_cr0(get_cr0() | CR0_EM | CR0_TS | CR0_NE);
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
  init_vdisk();
  init_vfs();
  init_floppy();
  init_devfs();
  Register_fat_fileSys();
  reg_pfs();
  memsize = memtest(0x00400000, 0xbfffffff);
  if (memsize / (1024 * 1024) < 32) {
    while (1) {
      beep(3, 7, 5);
      sleep(100);
    }
  }
  pf_set(memsize);
  init_acpi();
  disable_sb16();
  Input_Stack_Init();
  socket_init();
  init_driver();
  init_mount_disk();
  SetDrive((unsigned char *)"DISK_DRIVE");
  SetDrive((unsigned char *)"NETCARD_DRIVE");
  normal.tss.ldtr = 0;
  normal.tss.iomap = 0x40000000;
  set_segmdesc(gdt + 103, 103, (int)&normal.tss, AR_TSS32);
  load_tr(103 * 8);
  sr1 = register_task("System retention1", 1, 2 * 8, (int)task_sr1, 1 * 8,
                      1 * 8, (unsigned int)page_malloc(64 * 1024) + 64 * 1024);
  sr2 = register_task("System retention2", 1, 2 * 8, (int)task_sr2, 1 * 8,
                      1 * 8, (unsigned int)page_malloc(64 * 1024) + 64 * 1024);
  shell_task = register_task("Shell", 1, 2 * 8, (int)shell, 1 * 8, 1 * 8,
                             (unsigned int)page_malloc(64 * 1024) + 64 * 1024);
  //给每个任务设置FIFO
  task_set_fifo(shell_task, &keyfifo, &mousefifo);
  task_set_fifo(sr1, &keyfifo_sr1, &mousefifo_sr1);
  task_set_fifo(sr2, &keyfifo_sr2, &mousefifo_sr2);
  void *alloc_addr = (void *)page_malloc(512 * 1024);
  shell_task->alloc_addr = alloc_addr;
  shell_task->alloc_size = 512 * 1024;
  shell_task->mm = memory_init((uint32_t)alloc_addr, 512 * 1024);
  srand(time()); // Init random seed
  while (fifo8_status(task_get_key_fifo(shell_task)) != 0)
    fifo8_get(task_get_key_fifo(shell_task));
  while (fifo8_status(task_get_key_fifo(sr1)) != 0)
    fifo8_get(task_get_key_fifo(sr1));
  while (fifo8_status(task_get_key_fifo(sr2)) != 0)
    fifo8_get(task_get_key_fifo(sr2));
  PCI_ADDR_BASE = (unsigned int)page_malloc(1 * 1024 * 1024);
  init_PCI(PCI_ADDR_BASE);
  // printf("%08x\n", PCI_ADDR_BASE);
  init_float();

  mt_init();
  for (;;)
    ;
}
