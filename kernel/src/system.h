#include <string.h>
#include <stdio.h>
// 常量定义
#define VERSION "0.5c" // Version of the program
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR 0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1
#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x00280000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_DATA16_RW 0x0092
#define AR_CODE32_ER 0x409a
#define AR_CODE16_ER 0x009a
#define AR_INTGATE32 0x008e
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define AR_TSS32 0x0089
#define EOF -1
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define NULL ((void *)0)
#define Panic_K(info,...) printk("%s--PANIC: %s:%d Info:"info"\n",__FUNCTION__,__FILE__, __LINE__, ##__VA_ARGS__);
#define Panic_F(info,...) printf("%s--PANIC: %s:%d Info:"info"\n",__FUNCTION__,__FILE__, __LINE__, ##__VA_ARGS__);
extern char keytable[0x54];
extern char keytable1[0x54];
extern char mouse_cur_graphic[16][16];
struct TSS32
{
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int es, cs, ss, ds, fs, gs;
    int ldtr, iomap;
};
struct TASK
{
    int sel, sleep, level;
    struct TSS32 tss;
    char name[32];
    char running;
    struct tty *TTY;
	struct FIFO8 *keyfifo,*mousefifo;	// 基本输入设备的缓冲区
	int fifosleep;
};
struct pos
{
    int x, y;
};
struct tty
{
    int x, y;          // 显示位置
    int rx, ry;        // 真实位置
    struct pos cursor; // 光标位置
    char *vram;        // 屏幕的地址，或者说就是写入到哪里，要是到屏幕上，就是写入到0xB8000
};
struct SHEET
{
    unsigned char *buf;
    int bxsize, bysize, vx0, vy0, col_inv, height, flags;
    struct SHTCTL *ctl;
};
#define MAX_SHEETS 256
void Draw_Circle_32(int x, int y, int r, char Cr, char Cg, char Cb, int xsize, unsigned char *vram);
void BMPVIEW32(char *path, unsigned char *vram, int xsize);
struct SHTCTL
{
    unsigned char *vram, *map;
    int xsize, ysize, top;
    struct SHEET *sheets[MAX_SHEETS];
    struct SHEET sheets0[MAX_SHEETS];
};
extern int dictaddr;
// naskfunc.nas

typedef struct
{
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;
void INT(unsigned char intnum, regs16_t *regs);
void int32(unsigned char intnum, regs16_t *regs);
void null_inthandler();
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void asm_inthandler36();
void asm_inthandler2c();
void asm_inthandler20();
void asm_inthandler21();
void io_cli(void);
void io_sti(void);
void load_tr(int tr);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void loadregisters();
void saveregisters();
void intreturn(int eax, int ebx, int ecx, int edx, int esi, int edi, int ebp);
void gensound(int notes, int dup);
int get_cpu1();
int get_cpu2();
int get_cpu3();
int get_cpu4();
int get_cpu5();
int get_cpu6();
int get_cpu7();
void asm_error0();
void asm_error1();
void asm_error3();
void asm_error4();
void asm_error5();
void asm_error6();
void asm_error7();
void asm_error8();
void asm_error9();
void asm_error10();
void asm_error11();
void asm_error12();
void asm_error13();
void asm_error14();
void asm_error16();
void asm_error17();
void asm_error18();
void initialize_floppy_DMA();
void prepare_for_floppy_DMA_write();
void prepare_for_floppy_DMA_read();
// screen.c
void printf(const char *format, ...);
void clear(void);
void ASM_call(int a);
void Move_Cursor(short x, short y);
void putchar(char ch);
void putstr(const char *str, int length);
void printchar(char ch);
void outputchar_b800(int x, int y, char ch);
void screen_ne();
int Get_S_NE_T();
int Get_Raw_x();
int Get_Raw_y();
void init_s_ne_t();
int getlength(const char *str);
void print(const char *str);
void gotoxy(int x1, int y1);
int get_x();
int get_y();
void PUTCHINESE0(unsigned char *vram, int x, int y, char color, unsigned short CH, int xsize);
void Close_High_Text_Mode(void);
void SwitchToHighTextMode();
void HighPrint(char *string);
void HighPutStr(char *string, int length);
void Draw_Px32(unsigned int *vram, int x, int y, int color, int xsize);
// CASM.c
char *_Asm(const char *asm_code1, int addr);

// mouse.c
// 鼠标状态MOUSE_DEC
struct MOUSE_DEC
{
    unsigned char buf[3], phase;
    int x, y, btn;
    int sleep;
};
void wait_KBC_sendready(void);
void init_keyboard(void);
void enable_mouse(struct MOUSE_DEC *mdec);
void mouse_sleep(struct MOUSE_DEC *mdec);
void mouse_ready(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064

// fifo.c
struct FIFO8
{
    unsigned char *buf;
    int p, q, size, free, flags;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);
struct FIFO32
{
    int *buf;
    int p, q, size, free, flags;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);
// kernelc.c
struct SEGMENT_DESCRIPTOR
{
    short limit_low, base_low;
    char base_mid, access_right;
    char limit_high, base_high;
};
struct GATE_DESCRIPTOR
{
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};
void print_32bits_ascil(unsigned int n);
void shell(void);
unsigned int get_memsize();
int kbhit();
void sleep(int s);
void input(char *ptr, int len);
void input_No_El(char *ptr, int len);
int getch();
int input_char_inSM();
int get_caps_lock();
int get_shift();
char *get_point_of_keytable();
char *get_point_of_keytable1();
extern int mx;
extern int my;
extern char pro_memman[1024];
extern struct MOUSE_DEC mdec;
extern char *line;
extern int shift;
extern int caps_lock;
extern char *path;
void task_sr2();
void task_sr1();
// gdtidt.c
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void init_pic(void);
void init_gdtidt(void);
// shell.c
void command_run(char *cmdline);
void Copy(char *path, char *path1);
int run_bat(char *cmdline);
int cmd_app(char *cmdline);
void cmd_dir();
void type_deal();
void mkfile(char *cmdline);
void del(char *cmdline);
void edit(char *cmdline);
void bmpview(char *filename);
void chview(char *filename);
void casm_shell();
void clean(char *s, int len);
void pcinfo();
void mem();
void PUTCHINESE(int x, int y, char color, unsigned short CH);
void PrintChineseChar(int x, int y, char color, unsigned short cChar);
void PrintChineseStr(int x, int y, char color, unsigned char *str);
void read_floppy_disk_all();
// void HighPrint(char *str);

int GetCHorEN(unsigned char *str);
void beep(int point, int notes, int dup);
void mouseinput();
void EDIT_FILE(char *name, char *dest, int length);
// file.c
struct FILEINFO
{
    unsigned char name[8], ext[3], type;
    char reserve[10];
    unsigned short time, date, clustno;
    unsigned int size;
};
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef signed long LONG;
typedef unsigned int UINT;
typedef unsigned char UCHAR;
typedef short SWORD;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

/* VESA 信息块 */
typedef struct
{
    u16 offset;
    u16 seg;
} ReadModeFarPointer;
typedef struct
{
    u16 attributes;
    u8 winA, winB;
    u16 granularity;
    u16 winsize;
    u16 segmentA, segmentB;

    /* In VBE Specification, this field should be
     * ReadModeFarPointer winPosFunc;
     * However, we overwrite this field in loader n*/
    u16 mode;
    u16 reserved2;

    u16 bytesPerLine;

    u16 width, height;
    u8 Wchar, Ychar, planes, bitsPerPixel, banks;
    u8 memory_model, bank_size, image_pages;
    u8 reserved0;

    u8 red_mask, red_position;
    u8 green_mask, green_position;
    u8 blue_mask, blue_position;
    u8 rsv_mask, rsv_position;
    u8 directcolor_attributes;

    u32 physbase; // your LFB (Linear Framebuffer) address ;)
    u32 offscreen;
    u16 offsize;

} VESAModeInfo;
typedef struct
{
    u8 signature[4];
    u16 Version;
    ReadModeFarPointer oemString;
    u32 capabilities;
    ReadModeFarPointer videoModes;
    u16 totalMemory;
    u16 OEMVersion;
    ReadModeFarPointer vendor;
    ReadModeFarPointer product;
    ReadModeFarPointer revision;
    /* In VBE Specification, this field should be reserved.
     * However, we overwrite this field in loader */
    u16 modeCount;
    u8 reserved0[220];
    u8 oemUse[256];
    VESAModeInfo modeList[0];
} VESAControllerInfo;

#define rmfarptr2ptr(x) ((x).seg * 0x10 + (x).offset)
typedef struct FILE
{
    unsigned char *buf; // 文件缓冲区
    int size;           // 文件大小
    int p;              // 当前读写位置
} FILE;

struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
void file_readfat(int *fat, unsigned char *img);
char *fopen(char *name);
//#define ADR_DISKIMG 0x00100000
extern int ADR_DISKIMG;
int Get_dictaddr(char *path1);
FILE *fopen_for_FILE(char *path, char *mode);
int fgetc(FILE *fp);
char *fgets(char *s, int size, FILE *fp);
int fclose(FILE *fp);
int fseek(FILE *fp, int offset, int whence);
int fread(void *buf, int size, int count, FILE *fp);
// setup.c
void lba2chs(int lba, int cyline, int header, int sector);
int chs2lba(int cyline, int header, int sector);
void setup(void);

// timer.c
#define MAX_TIMER 500
struct timer
{
    int count;
};
struct TIMER
{
    struct TIMER *next;
    unsigned int timeout, flags;
    struct FIFO32 *fifo;
    int data;
};
struct TIMERCTL
{
    unsigned int count, next;
    struct TIMER *t0;
    struct TIMER timers0[MAX_TIMER];
};
extern struct TIMERCTL timerctl;
void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);
// void unmaskIRQ(unsigned char irq); //允许中断
//  vga.c
void write_regs(unsigned char *regs);
void SwitchTo320X200X256();
void SwitchToText8025();
void Set_Font(char *file);
void Gprint(char *string);
void Gprintf(char *string, ...);
#define outportb io_out8 //因为我们只有io_out8，所以可以直接用io_out8
#define inportb io_in8   //因为我们只有io_in8，所以可以直接用io_in8
#define VGA_AC_INDEX 0x3C0
#define VGA_AC_WRITE 0x3C0
#define VGA_AC_READ 0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9
#define VGA_MISC_READ 0x3CC
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX 0x3D4 /* 0x3B4 */
#define VGA_CRTC_DATA 0x3D5  /* 0x3B5 */
#define VGA_INSTAT_READ 0x3DA

#define VGA_NUM_SEQ_REGS 5
#define VGA_NUM_CRTC_REGS 25
#define VGA_NUM_GC_REGS 9
#define VGA_NUM_AC_REGS 21
#define VGA_NUM_REGS (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
#define _vmemwr(DS, DO, S, N) memcpy((char *)((DS)*16 + (DO)), S, N)
void Draw_Char(int x, int y, char c, int color);
void Draw_Str(int x, int y, char *str, int color);
void Draw_Box(int x, int y, int w, int h, int color);
void Draw_Px(int x, int y, int color);
static void set_plane(unsigned p);
void pokew(int setmentaddr, int offset, short value);
void pokeb(int setmentaddr, int offset, char value);
void set_palette(int start, int end, unsigned char *rgb);
static void vmemwr(unsigned dst_off, unsigned char *src, unsigned count);
void init_palette(void);
unsigned get_fb_seg(void);
extern unsigned char table_rgb[16 * 3];
// vbe.c
struct VBEINFO
{
    char res1[18];
    short xsize, ysize;
    char res2[18];
    int vram;
};
int SwitchVBEMode(int mode);
int check_vbe_mode(int mode, struct VBEINFO *vinfo);
void SwitchToText8025_BIOS();
void SwitchTo320X200X256_BIOS();
#define MODE_640X400X256 0x100
#define MODE_640X480X256 0x101
#define MODE_800X600X256 0x103
#define MODE_1024X768X256 0x105
#define MODE_1024X768XRGB 0x118
#define VBEINFO_ADDRESS 0x7e00
// hd.c
void drivers_idehdd_read(unsigned int LBA, unsigned int number, unsigned short *buffer);
void drivers_idehdd_write(unsigned int LBA, unsigned int number, unsigned short *buffer);
void drivers_idehdd_wait(void);
void drivers_idehdd_wait2(void);
void drivers_idehdd_wait1(void);
// acpi.c
struct ACPI_RSDP
{
    char Signature[8];
    unsigned char Checksum;
    char OEMID[6];
    unsigned char Revision;
    unsigned int RsdtAddress;
    unsigned int Length;
    unsigned int XsdtAddress[2];
    unsigned char ExtendedChecksum;
    unsigned char Reserved[3];
};
struct ACPISDTHeader
{
    char Signature[4];
    unsigned int Length;
    unsigned char Revision;
    unsigned char Checksum;
    char OEMID[6];
    char OEMTableID[8];
    unsigned int OEMRevision;
    unsigned int CreatorID;
    unsigned int CreatorRevision;
};
struct ACPI_RSDT
{
    struct ACPISDTHeader header;
    unsigned int Entry;
};
typedef struct
{
    unsigned char AddressSpace;
    unsigned char BitWidth;
    unsigned char BitOffset;
    unsigned char AccessSize;
    unsigned int Address[2];
} GenericAddressStructure;
struct ACPI_FADT
{
    struct ACPISDTHeader h;
    unsigned int FirmwareCtrl;
    unsigned int Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    unsigned char Reserved;

    unsigned char PreferredPowerManagementProfile;
    unsigned short SCI_Interrupt;
    unsigned int SMI_CommandPort;
    unsigned char AcpiEnable;
    unsigned char AcpiDisable;
    unsigned char S4BIOS_REQ;
    unsigned char PSTATE_Control;
    unsigned int PM1aEventBlock;
    unsigned int PM1bEventBlock;
    unsigned int PM1aControlBlock;
    unsigned int PM1bControlBlock;
    unsigned int PM2ControlBlock;
    unsigned int PMTimerBlock;
    unsigned int GPE0Block;
    unsigned int GPE1Block;
    unsigned char PM1EventLength;
    unsigned char PM1ControlLength;
    unsigned char PM2ControlLength;
    unsigned char PMTimerLength;
    unsigned char GPE0Length;
    unsigned char GPE1Length;
    unsigned char GPE1Base;
    unsigned char CStateControl;
    unsigned short WorstC2Latency;
    unsigned short WorstC3Latency;
    unsigned short FlushSize;
    unsigned short FlushStride;
    unsigned char DutyOffset;
    unsigned char DutyWidth;
    unsigned char DayAlarm;
    unsigned char MonthAlarm;
    unsigned char Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    unsigned short BootArchitectureFlags;

    unsigned char Reserved2;
    unsigned int Flags;

    // 12 byte structure; see below for details
    GenericAddressStructure ResetReg;

    unsigned char ResetValue;
    unsigned char Reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    unsigned int X_FirmwareControl[2];
    unsigned int X_Dsdt[2];

    GenericAddressStructure X_PM1aEventBlock;
    GenericAddressStructure X_PM1bEventBlock;
    GenericAddressStructure X_PM1aControlBlock;
    GenericAddressStructure X_PM1bControlBlock;
    GenericAddressStructure X_PM2ControlBlock;
    GenericAddressStructure X_PMTimerBlock;
    GenericAddressStructure X_GPE0Block;
    GenericAddressStructure X_GPE1Block;
};
void init_acpi(void);
int acpi_shutdown(void);
unsigned int acpi_find_table(char *Signature);
unsigned int *acpi_find_rsdp(void);
char checksum(unsigned char *addr, unsigned int length);

// Input_Stack.c
struct Input_StacK
{
    char **Stack;
    unsigned int Stack_Size;
    unsigned int free;
    unsigned int Now;
    unsigned int times;
};

void Input_Stack_Init(void);
void Input_Stack_Put(char *str);
int Get_Now();
char *Pop_Stack();
void Set_Now(int now);
int Get_times();
// other.c
void ERROR0();
void ERROR1();
void ERROR3();
void ERROR4();
void ERROR5();
void ERROR6();
void ERROR7();
void ERROR8();
void ERROR9();
void ERROR10();
void ERROR11();
void ERROR12();
void ERROR13();
void ERROR14();
void ERROR16();
void ERROR17();
void ERROR18();
void ERROR19();
void ERROR(int CODE, char *TIPS);
void KILLAPP(int ec);
void insert_char(char *str, int pos, char ch); // str:字符串，pos:位置，ch:字符
void delete_char(char *str, int pos);          // str:字符串，pos:位置
char bcd2hex(char bcd);
char hex2bcd(char hex);
void getCPUBrand(char *cBrand);
char ascii2num(char c);
char num2ascii(char c);
char *itoa(int n);
void inthandler36(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
void inthandler21(int *esp);
void inthandler2c(int *esp);
void strtoupper(char *str);
// task.c
struct TASK *AddTask(char *name, int level, int cs, int eip, int ds, int ss, int esp);
void mt_init(void);
void SleepTask(struct TASK *task);
void WakeUp(struct TASK *task);
struct TASK *GetTask(int taskNum);
void SubTask(struct TASK *task);
struct TASK *task_now(void);
struct TASK *GetTaskForName(char *taskname);
struct TASK *NowTask();
struct TASK *AddV86_Task(char *name, int level, int cs, int eip, int ds, int ss, int esp);
// irq.c
void ClearMaskIrq(unsigned char irq);
void Maskirq(unsigned char irq);

// dma.c
void dma_xfer(unsigned char channel, unsigned long address, unsigned int length, unsigned char read);

// floppy.c
int read_block(int block, unsigned char *blockbuff, unsigned long nosectors);
int write_block(int block, unsigned char *blockbuff, unsigned long nosectors);
int write_floppy_for_ths(int track, int head, int sec, unsigned char *blockbuff, unsigned long nosec);
void block2hts(int block, int *track, int *head, int *sector);
// init.c
void sysinit(void);
// sheet.c
void sheet_free(struct SHEET *sht);
void ctl_free(struct SHTCTL *ctl);
struct SHTCTL *shtctl_init(unsigned char *vram, int xsize, int ysize);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void SDraw_Box(unsigned char *vram, int x, int y, int x1, int y1, int color, int xsize);
// pci.c
void init_PCI(unsigned int adder_Base);

struct FILEINFO *Get_File_Address(char *path1);
int fdc_rw_ths(int track, int head, int sector, unsigned char *blockbuff, int read, unsigned long nosectors);
// jpeg.c
struct DLL_STRPICENV
{ /* 64KB */
    int work[64 * 1024 / 4];
};

struct RGB
{
    unsigned char b, g, r, t;
};

int info_JPEG(struct DLL_STRPICENV *env, int *info, int size, char *fp);
int decode0_JPEG(struct DLL_STRPICENV *env, int size, char *fp, int b_type, char *buf, int skip);
void jpgview32(char *path, unsigned char *vram, int xsize);
//com.c
int init_serial(void);
int serial_received();
char read_serial();
int is_transmit_empty();
void write_serial(char a);
//log.c
void kprint(char *str);
void printk(char *str, ...);
// page.c
#define	PG_P			1
#define	PG_USU			4
#define	PG_RWW			2
#define	PDE_ADDRESS		0x400000
#define	PTE_ADDRESS		0x401000
#define	PAGE_END		0x801000
#define	PAGE_MANNAGER	0x801000
void C_init_page();
int get_line_address(int t,int p,int o);
int get_page_from_line_address(int line_address);
void page2tpo(int page,int *t,int *p);
void tpo2page(int *page,int t,int p);
void *page_malloc_one();
int get_pageinpte_address(int t,int p);
void page_free_one(void *p);
int find_kpage(int line, int n);
void *page_vmalloc(int size);
void *page_kmalloc(int size);
void page_kfree(int p,int size);
void page_vfree(void *p,int size);
//cmos.c
#define BCD_HEX(n) ((n >> 4) * 10) + (n & 0xf)
#define HEX_BCD(n) ((n / 10) << 4) + (n % 10)
#define CMOS_CUR_SEC 0x0
#define CMOS_ALA_SEC 0x1
#define CMOS_CUR_MIN 0x2
#define CMOS_ALA_MIN 0x3
#define CMOS_CUR_HOUR 0x4
#define CMOS_ALA_HOUR 0x5
#define CMOS_WEEK_DAY 0x6
#define CMOS_MON_DAY 0x7
#define CMOS_CUR_MON 0x8
#define CMOS_CUR_YEAR 0x9
#define CMOS_DEV_TYPE 0x12
#define CMOS_CUR_CEN 0x32
#define cmos_index 0x70
#define cmos_data 0x71
unsigned char read_cmos(unsigned char p);
void write_cmos(unsigned char p, unsigned char data);
unsigned int get_hour_hex();
unsigned int get_min_hex();
unsigned int get_sec_hex();
unsigned int get_day_of_month();
unsigned int get_day_of_week();
unsigned int get_mon_hex();
unsigned int get_year();
void write_cmos_time(unsigned int year,unsigned char mon,unsigned char day,unsigned char hour, unsigned char min);
//tty.c
void clear_tty(struct tty *tty);
void screen_ne_tty(struct tty *tty);
void putstr_tty(const char *str, int length, struct tty *tty);
void print_tty(const char *str, struct tty *tty);
void putchar_tty(char ch, struct tty *tty);
void GotoXy_No_Safe_tty(int x1, int y1, struct tty *tty);
void gotoxy_tty(int x1, int y1, struct tty *tty);

struct PCI_CONFIG_SPACE_PUCLIC{ //用64个字节描述
    unsigned short VendorID;
    unsigned short DeviceID;
    unsigned short Command;
    unsigned short Status;
    unsigned char RevisionID;
    unsigned char ProgIF;
    unsigned char SubClass;
    unsigned char BaseClass;
    unsigned char CacheLineSize;
    unsigned char LatencyTimer;
    unsigned char HeaderType;
    unsigned char BIST;
    unsigned int  BaseAddr[6];
    unsigned int  CardbusCIS;
    unsigned short SubVendorID;
    unsigned short SubSystemID;
    unsigned int  ROMBaseAddr;
    unsigned char CapabilitiesPtr;
    unsigned char Reserved[3];
    unsigned int  Reserved1;
    unsigned char InterruptLine;
    unsigned char InterruptPin;
    unsigned char MinGrant;
    unsigned char MaxLatency;
};