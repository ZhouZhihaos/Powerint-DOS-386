#include "system.h"
struct TASK *shell_task;
struct TASK *sr1, *sr2;
struct TASK normal;
void sysinit(void)
{
	struct FIFO8 keyfifo, mousefifo;
	struct FIFO8 keyfifo_sr1,keyfifo_sr2;
	struct FIFO8 mousefifo_sr1,mousefifo_sr2;
    extern int PCI_ADDR_BASE;
    extern struct tty tty_console[6];
    extern struct MOUSE_DEC mdec;
    extern struct timer Timer;
    extern int caps_lock, shift;
    extern unsigned int memsize;
    extern int mx, my;
    extern char *line;
    extern char *path;
    char keybuf[32];
    char mousebuf[128];
    char keybuf_sr1[32];
    char mousebuf_sr1[128];
    char keybuf_sr2[32];
    char mousebuf_sr2[128];
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
    clear();
	init_page();	// 初始化分页
    init_gdtidt();
    init_pic();
    io_sti();
    ClearMaskIrq(0);  // pit (timer)
    ClearMaskIrq(1);  // keyboard
    ClearMaskIrq(12); // mouse
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
    memsize = memtest(0x00400000, 0xbfffffff);
    if (memsize / (1024 * 1024) < 16)
    {
        printf("\nMemory Size:%dMB\n", memsize / (1024 * 1024));
        printf("MEMORY SIZE TOO SMALL!\n");
        printf("If you want to start Powerint DOS,must change memory size to 16MB or more.\n");
        if (memsize / (1024 * 1024) < 9)
        {
            while (1)
            {
                beep(3, 7, 5);
                sleep(10);
            }
        }
        printf("This is a warning,press any key to start system...");
        getch();
    }
    init_acpi();
    clear();
    init_floppy();
    Input_Stack_Init();
    normal.tss.ldtr = 0;
    normal.tss.iomap = 0x40000000;
    set_segmdesc(gdt + 103, 103, (int)&normal.tss, AR_TSS32);
    load_tr(103 * 8);
    sr1 = AddTask("System retention", 1, 2 * 8, (int)task_sr1, 1 * 8, 1 * 8, (unsigned int)page_kmalloc(64 * 1024) + 64 * 1024);
    sr2 = AddTask("System retention", 1, 2 * 8, (int)task_sr2, 1 * 8, 1 * 8, (unsigned int)page_kmalloc(64 * 1024) + 64 * 1024);
    shell_task = AddTask("Shell", 1, 2 * 8, (int)shell, 1 * 8, 1 * 8, (unsigned int)page_kmalloc(128 * 1024) + 128 * 1024);
	//给每个任务设置FIFO
	TaskSetFIFO(shell_task,&keyfifo,&mousefifo);
	TaskSetFIFO(sr1,&keyfifo_sr1,&mousefifo_sr1);
	TaskSetFIFO(sr2,&keyfifo_sr2,&mousefifo_sr2);
	clear();
    printf("Loading Powerint DOS...\n");
    #ifdef __READ__ALL__
    read_floppy_disk_all(); //系统初始化的最后一步，读取所有软盘的数据
    #endif
    while (fifo8_status(TaskGetKeyfifo(shell_task)) != 0)
        fifo8_get(TaskGetKeyfifo(shell_task));
    while (fifo8_status(TaskGetKeyfifo(sr1)) != 0)
        fifo8_get(TaskGetKeyfifo(sr1));
    while (fifo8_status(TaskGetKeyfifo(sr2)) != 0)
        fifo8_get(TaskGetKeyfifo(sr2));
    PCI_ADDR_BASE = (unsigned int)page_kmalloc(1*1024*1024);
    init_PCI(PCI_ADDR_BASE);
    //printf("%08x\n", PCI_ADDR_BASE);
	mt_init();
	for(;;);
}