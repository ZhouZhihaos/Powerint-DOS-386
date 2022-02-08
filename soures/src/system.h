#include <string.h>
#include <stdio.h>
// 常量定义
#define VERSION "0.5a" // Version of the program
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
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define AR_TSS32 0x0089
// naskfunc.nas
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void start_app(int eip, int cs, int ds);
void asm_inthandler36();
void asm_inthandler2c();
void asm_inthandler20();
void asm_inthandler21();
void io_cli(void);
void io_sti(void);
void io_out8(int port,int data);
void io_out16(int port,int data);
void io_out32(int port,int data);
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
void intreturn(int eax,int ebx,int ecx,int edx,int esi,int edi,int ebp);
void gensound(int notes,int dup);
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
int getlength(const char *str);
void print(const char *str);
void gotoxy(int x1, int y1);
int get_x();
int get_y();

// CASM.c
char *_Asm(const char *asm_code1,int addr);

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
void getCPUBrand(char *cBrand);
void shell(void);
int get_memsize();
void sleep(int s);
char *itoa(int n);
char ascii2num(char c);
char num2ascii(char c);
void input(char *ptr,int len);
char getch();
int input_char_inSM();
void mouseinput();
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
void inthandler36(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax);
char bcd2hex(char bcd);
char hex2bcd(char hex);
void init_gdtidt(void);
void inthandler21(int *esp);
void inthandler2c(int *esp);
int get_caps_lock();
int get_shift();
char *get_point_of_keytable();
char *get_point_of_keytable1();
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void init_pic(void);
int cmd_app(char *cmdline);
int run_bat(char *cmdline);
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
void ERROR(int CODE,char *TIPS);

// shell.c
void command_run(char *cmdline);
void cmd_dir();
void type_deal();
void mkfile(char *cmdline);
void del(char *cmdline);
void edit(char *cmdline);
void bmpview(char *filename);
void chview(char *filename);
void casm_shell();
void clean(char *s,int len);
void pcinfo();
void mem();
void pak(char *pakname,char *filenames);
void unpak(char *pakname);
void PUTCHINESE(int x, int y, char color, unsigned short CH);
void PrintChineseChar(int x, int y, char color, unsigned short cChar);
void PrintChineseStr(int x, int y, char color, unsigned char *str);
int GetCHorEN(unsigned char *str);
void beep(int point,int notes,int dup);

// file.c
struct FILEINFO
{
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
void file_readfat(int *fat, unsigned char *img);
char *fopen(char *name);
#define ADR_DISKIMG 0x00100000

// setup.c
void lba2chs(int lba, int cyline, int header, int sector);
int chs2lba(int cyline, int header, int sector);
void setup(void);

// memory.c
#define MEMMAN_FREES 4090 /* 大约是32KB*/
#define MEMMAN_ADDR 0x003c0000
struct FREEINFO
{ /* 可用信息 */
	unsigned int addr, size;
};
struct MEMMAN
{ /* 内存管理 */
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memtest_sub(unsigned int start, unsigned int end);

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
void timer_init(struct TIMER *timer, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);

// vga.c
void write_regs(unsigned char *regs);
void SwitchTo320X200X256();
void SwitchToText8025();
#define outportb io_out8 //因为我们只有io_out8，所以可以直接用io_out8
#define inportb io_in8	 //因为我们只有io_in8，所以可以直接用io_in8
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
#define VGA_CRTC_DATA 0x3D5	 /* 0x3B5 */
#define VGA_INSTAT_READ 0x3DA

#define VGA_NUM_SEQ_REGS 5
#define VGA_NUM_CRTC_REGS 25
#define VGA_NUM_GC_REGS 9
#define VGA_NUM_AC_REGS 21
#define VGA_NUM_REGS (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
#define _vmemwr(DS, DO, S, N) memcpy((char *)((DS)*16 + (DO)), S, N)
void Draw_Char(int x, int y, char c, int color);
void Draw_Str(int x,int y,char *str,int color);
void Draw_Box(int x, int y, int w, int h, int color);
void Draw_Px(int x, int y, int color);
static void set_plane(unsigned p);
void pokew(int setmentaddr, int offset, short value);
void pokeb(int setmentaddr, int offset, char value);
void set_palette(int start, int end, unsigned char *rgb);
static void vmemwr(unsigned dst_off, unsigned char *src, unsigned count);
static unsigned get_fb_seg(void);
void init_palette(void);

// hd.c
void drivers_idehdd_read(unsigned int LBA, unsigned int number, unsigned short *buffer);
void drivers_idehdd_write(unsigned int LBA, unsigned int number, unsigned short *buffer);
void drivers_idehdd_wait(void);
void drivers_idehdd_wait2(void);
void drivers_idehdd_wait1(void);


// acpi.c
struct ACPI_RSDP {
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
struct ACPISDTHeader {
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
struct ACPI_RSDT {
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
    struct   ACPISDTHeader h;
    unsigned int FirmwareCtrl;
    unsigned int Dsdt;
 
    // field used in ACPI 1.0; no longer in use, for compatibility only
    unsigned char  Reserved;
 
    unsigned char  PreferredPowerManagementProfile;
    unsigned short SCI_Interrupt;
    unsigned int SMI_CommandPort;
    unsigned char  AcpiEnable;
    unsigned char  AcpiDisable;
    unsigned char  S4BIOS_REQ;
    unsigned char  PSTATE_Control;
    unsigned int PM1aEventBlock;
    unsigned int PM1bEventBlock;
    unsigned int PM1aControlBlock;
    unsigned int PM1bControlBlock;
    unsigned int PM2ControlBlock;
    unsigned int PMTimerBlock;
    unsigned int GPE0Block;
    unsigned int GPE1Block;
    unsigned char  PM1EventLength;
    unsigned char  PM1ControlLength;
    unsigned char  PM2ControlLength;
    unsigned char  PMTimerLength;
    unsigned char  GPE0Length;
    unsigned char  GPE1Length;
    unsigned char  GPE1Base;
    unsigned char  CStateControl;
    unsigned short WorstC2Latency;
    unsigned short WorstC3Latency;
    unsigned short FlushSize;
    unsigned short FlushStride;
    unsigned char  DutyOffset;
    unsigned char  DutyWidth;
    unsigned char  DayAlarm;
    unsigned char  MonthAlarm;
    unsigned char  Century;
 
    // reserved in ACPI 1.0; used since ACPI 2.0+
    unsigned short BootArchitectureFlags;
 
    unsigned char  Reserved2;
    unsigned int Flags;
 
    // 12 byte structure; see below for details
    GenericAddressStructure ResetReg;
 
    unsigned char  ResetValue;
    unsigned char  Reserved3[3];
 
    // 64bit pointers - Available on ACPI 2.0+
    unsigned int                X_FirmwareControl[2];
    unsigned int                X_Dsdt[2];
 
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