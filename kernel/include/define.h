#ifndef _DEFINE_H
#define _DEFINE_H
#include <ctypes.h>
#include <stdarg.h>
#include <stddef.h>
typedef unsigned int vram_t;
typedef vram_t color_t;

/* dos.h */
#define VERSION "0.7b" // Version of the program
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
#define Panic_Print(func, info, ...)                                           \
  func("%s--PANIC: %s:%d Info:" info "\n", __FUNCTION__, __FILE__, __LINE__,   \
       ##__VA_ARGS__);
#define WARNING_Print(func, info, ...)                                         \
  func("%s--WARNING: %s:%d Info:" info "\n", __FUNCTION__, __FILE__, __LINE__, \
       ##__VA_ARGS__);
#define DEBUG_Print(func, info, ...)                                           \
  func("%s--DEBUG: %s:%d Info:" info "\n", __FUNCTION__, __FILE__, __LINE__,   \
       ##__VA_ARGS__);
#define Panic_K(info, ...) Panic_Print(printk, info, ##__VA_ARGS__)
#define WARNING_K(info, ...) WARNING_Print(printk, info, ##__VA_ARGS__)
#define DEBUG_K(info, ...) DEBUG_Print(printk, info, ##__VA_ARGS__)
#define Panic_F(info, ...) Panic_Print(printf, info, ##__VA_ARGS__)
#define WARNING_F(info, ...) WARNING_Print(printf, info, ##__VA_ARGS__)
#define DEBUG_F(info, ...) DEBUG_Print(printf, info, ##__VA_ARGS__)
#define get_tid(task) task->sel / 8 - 103
#define POWERINTDOS 0
#define POWERDESKTOP 1
#define HIGHTEXTMODE 2
extern struct MOUSE_DEC mdec;
extern int gmx, gmy;
extern unsigned char *font, *ascfont, *hzkfont;
extern struct TIMERCTL timerctl;
extern unsigned int memsize;
extern uint32_t running_mode;
struct PAGE_INFO {
  unsigned flag : 2;
  unsigned task_id : 6;
} __attribute__((packed));
#define FREE_MAX_NUM 4096
#define ERRNO_NOPE 0
#define ERRNO_NO_ENOGHT_MEMORY 1
#define ERRNO_NO_MORE_FREE_MEMBER 2
#define MEM_MAX(a, b) (a) > (b) ? (a) : (b)
typedef struct {
  uint32_t start;
  uint32_t end; // end和start都等于0说明这个free结构没有使用
} free_member;
typedef struct freeinfo freeinfo;
typedef struct freeinfo {
  free_member *f;
  freeinfo *next;
} freeinfo;
typedef struct {
  freeinfo *freeinf;
  int memerrno;
} memory;
struct SEGMENT_DESCRIPTOR {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
};
struct GATE_DESCRIPTOR {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
};
#define MAX_TIMER 500
struct TIMER {
  struct TIMER *next;
  unsigned int timeout, flags;
  struct FIFO8 *fifo;
  unsigned char data;
};
struct TIMERCTL {
  unsigned int count, next;
  struct TIMER *t0;
  struct TIMER timers0[MAX_TIMER];
};
struct TSS32 {
  int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};
#define MAX_IPC_MESSAGE 5 // 一次最多存放5个IPC_MESSAGE
#define synchronous 1
#define asynchronous 2
struct IPC_Header { // IPC头（在TASK结构体中的头）
  int now;
  void *data[MAX_IPC_MESSAGE];
  unsigned int size[MAX_IPC_MESSAGE];
  int from_tid[MAX_IPC_MESSAGE];
};
struct THREAD {
  struct TASK *father;
};
#define _packed __attribute__((packed))
enum {
  CR0_PE = 1 << 0, // Protection Enable 启用保护模式
  CR0_MP = 1 << 1, // Monitor Coprocessor
  CR0_EM = 1 << 2, // Emulation 启用模拟，表示没有 FPU
  CR0_TS = 1 << 3, // Task Switch 任务切换，延迟保存浮点环境
  CR0_ET = 1 << 3, // Extension Type 保留
  CR0_NE = 1 << 5, // Numeric Error 启用内部浮点错误报告
  CR0_WP =
      1 << 16, // Write Protect 写保护（禁止超级用户写入只读页）帮助写时复制
  CR0_AM = 1 << 18, // Alignment Mask 对齐掩码
  CR0_NW = 1 << 29, // Not Write-Through 不是直写
  CR0_CD = 1 << 30, // Cache Disable 禁用内存缓冲
  CR0_PG = 1 << 31, // Paging 启用分页
};
typedef struct fpu_t {
  uint16_t control;
  uint16_t RESERVED1;
  uint16_t status;
  uint16_t RESERVED2;
  uint16_t tag;
  uint16_t RESERVED3;
  uint32_t fip0;
  uint32_t fop0;
  uint32_t fdp0;
  uint32_t fdp1;
  uint8_t regs[80];
} _packed fpu_t;
struct TASK {
  int sel, sleep, level;
  struct TSS32 tss;
  char name[32];
  char running;
  struct tty *TTY;
  struct FIFO8 *keyfifo, *mousefifo; // 基本输入设备的缓冲区
  int fifosleep;
  int cs_base, ds_base;
  void *alloc_addr;
  memory *mm;
  int alloc_size;
  struct IPC_Header IPC_header;
  struct TIMER *timer;
  int esp_start; // 开始的esp
  int eip_start; // 开始的eip
  short cs_start;
  short ss_start;
  int is_child; // 是子线程吗
  int app;
  struct THREAD thread;
  int drive_number;
  char drive;
  char *line;
  void (*keyboard_press)(unsigned char data, uint32_t task);
  void (*keyboard_release)(unsigned char data, uint32_t task);
  int nl;
  int lock; // 被锁住了？
  char forever;
  int DisableExpFlag;
  uint32_t CatchEIP;
  char flagOfexp;
  int mx, my;
  fpu_t *fpu;
  struct vfs_t *nfs;
  int fpu_flag;
  struct FIFO8 *Pkeyfifo, *Ukeyfifo;
  uint32_t fpu_use;
  uint32_t *gdt_data;
} __attribute__((packed));
#define vfs_now current_task()->nfs
#define PG_P 1
#define PG_USU 4
#define PG_RWW 2
#define PDE_ADDRESS 0x400000
#define PTE_ADDRESS 0x401000
#define PAGE_END 0x801000
#define PAGE_MANNAGER 0x801000
struct FIFO8 {
  unsigned char *buf;
  int p, q, size, free, flags;
};
struct ListCtl {
  struct List *start;
  struct List *end;
  int all;
};
struct List {
  struct ListCtl *ctl;
  struct List *prev;
  uintptr_t val;
  struct List *next;
};
typedef struct List List;

/* cmd.h */
#define CHAT_SERVER_IP 0x761ff8d7
#define CHAT_SERVER_PROT 25565
#define CHAT_CLIENT_PROT 21538

/* fs.h */
extern uint32_t Path_Addr;
struct FAT_CACHE {
  unsigned int ADR_DISKIMG;
  struct FAT_FILEINFO *root_directory;
  struct List *directory_list;
  struct List *directory_clustno_list;
  int *fat;
  int FatMaxTerms;
  unsigned int ClustnoBytes;
  unsigned short RootMaxFiles;
  unsigned int RootDictAddress;
  unsigned int FileDataAddress;
  unsigned int imgTotalSize;
  unsigned short SectorBytes;
  unsigned int Fat1Address, Fat2Address;
  unsigned char *FatClustnoFlags;
  int type;
};
typedef struct {
  struct FAT_CACHE dm;
  struct FAT_FILEINFO *dir;
} fat_cache;
#define get_dm(vfs) ((fat_cache *)(vfs->cache))->dm
#define get_now_dir(vfs) ((fat_cache *)(vfs->cache))->dir
#define get_clustno(high, low) (high << 16) | (low & 0xffff)
#define clustno_end(type) 0xfffffff & ((((1 << (type - 1)) - 1) << 1) + 1)
typedef enum { FLE, DIR, RDO, HID, SYS } ftype;
typedef struct {
  char name[255];
  ftype type;
  unsigned int size;
  unsigned short year, month, day;
  unsigned short hour, minute;
} vfs_file;
typedef struct vfs_t {
  List *path;
  void *cache;
  char FSName[255];
  int disk_number;
  uint8_t drive; // 大写（必须）
  vfs_file *(*FileInfo)(struct vfs_t *vfs, char *filename);
  List *(*ListFile)(struct vfs_t *vfs, char *dictpath);
  bool (*ReadFile)(struct vfs_t *vfs, char *path, char *buffer);
  bool (*WriteFile)(struct vfs_t *vfs, char *path, char *buffer, int size);
  bool (*DelFile)(struct vfs_t *vfs, char *path);
  bool (*DelDict)(struct vfs_t *vfs, char *path);
  bool (*CreateFile)(struct vfs_t *vfs, char *filename);
  bool (*CreateDict)(struct vfs_t *vfs, char *filename);
  bool (*RenameFile)(struct vfs_t *vfs, char *filename, char *filename_of_new);
  bool (*Attrib)(struct vfs_t *vfs, char *filename, ftype type);
  bool (*Format)(uint8_t disk_number);
  void (*InitFs)(struct vfs_t *vfs, uint8_t disk_number);
  void (*DeleteFs)(struct vfs_t *vfs);
  bool (*Check)(uint8_t disk_number);
  bool (*cd)(struct vfs_t *vfs, char *dictName);
  int (*FileSize)(struct vfs_t *vfs, char *filename);
  void (*CopyCache)(struct vfs_t *dest, struct vfs_t *src);
  int flag;
} vfs_t;
#define BS_jmpBoot 0
#define BS_OEMName 3
#define BPB_BytsPerSec 11
#define BPB_SecPerClus 13
#define BPB_RsvdSecCnt 14
#define BPB_NumFATs 16
#define BPB_RootEntCnt 17
#define BPB_TotSec16 19
#define BPB_Media 21
#define BPB_FATSz16 22
#define BPB_SecPerTrk 24
#define BPB_NumHeads 26
#define BPB_HiddSec 28
#define BPB_TotSec32 32
#define BPB_FATSz32 36
#define BPB_ExtFlags 40
#define BPB_FSVer 42
#define BPB_RootClus 44
#define BPB_FSInfo 48
#define BPB_BkBootSec 50
#define BPB_Reserved 52
#define BPB_Fat32ExtByts 28
#define BS_DrvNum 36
#define BS_Reserved1 37
#define BS_BootSig 38
#define BS_VolD 39
#define BS_VolLab 43
#define BS_FileSysType 54
#define EOF -1
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
struct FAT_FILEINFO {
  unsigned char name[8], ext[3], type;
  char reserve;
  unsigned char create_time_tenth;
  unsigned short create_time, create_date, access_date, clustno_high;
  unsigned short update_time, update_date, clustno_low;
  unsigned int size;
};
#define rmfarptr2ptr(x) ((x).seg * 0x10 + (x).offset)
typedef struct FILE {
  unsigned int mode;
  unsigned int fileSize;
  unsigned char *buffer;
  unsigned int bufferSize;
  unsigned int p;
  char *name;
} FILE;
struct DLL_STRPICENV {
  int work[16384];
};
struct RGB {
  unsigned char b, g, r, t;
};
struct paw_info {
  unsigned char reserved[12]; // 12 bytes reserved(0xFF)
  char oem[3];                // PRA
  int xsize;                  // xsize
  int ysize;                  // ysize
};

/* interrupts.h */
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
typedef struct {
  unsigned short di, si, bp, sp, bx, dx, cx, ax;
  unsigned short gs, fs, es, ds, eflags;
} regs16_t;

/* io.h */
struct tty {
  int using1;                              // 使用标志
  void *vram;                              // 显存（也可以当做图层）
  int x, y;                                // 目前的 x y 坐标
  int xsize, ysize;                        // x 坐标大小 y 坐标大小
  int Raw_y;                               // 换行次数
  unsigned char color;                     // 颜色
  void (*putchar)(struct tty *res, int c); // putchar函数
  void (*MoveCursor)(struct tty *res, int x, int y);  // MoveCursor函数
  void (*clear)(struct tty *res);                     // clear函数
  void (*screen_ne)(struct tty *res);                 // screen_ne函数
  void (*gotoxy)(struct tty *res, int x, int y);      // gotoxy函数
  void (*print)(struct tty *res, const char *string); // print函数
  void (*Draw_Box)(struct tty *res, int x, int y, int x1, int y1,
                   unsigned char color); // Draw_Box函数
  unsigned int reserved[4];              // 保留项
};
struct Input_StacK {
  char **Stack;
  unsigned int Stack_Size;
  unsigned int free;
  unsigned int Now;
  unsigned int times;
};
#define MAX_SHEETS 256
struct SHEET {
  vram_t *buf;
  int bxsize, bysize, vx0, vy0, col_inv, height, flags;
  struct SHTCTL *ctl;
  struct TASK *task;
  void (*Close)(); // 为NULL表示没有关闭函数
  void *args;
};
struct SHTCTL {
  vram_t *vram;
  unsigned char *map;
  int xsize, ysize, top;
  struct SHEET *sheets[MAX_SHEETS];
  struct SHEET sheets0[MAX_SHEETS];
};

/* drivers.h */
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
typedef struct {
  unsigned char AddressSpace;
  unsigned char BitWidth;
  unsigned char BitOffset;
  unsigned char AccessSize;
  unsigned int Address[2];
} GenericAddressStructure;
struct ACPI_FADT {
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
} __attribute__((packed));
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
#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define MOUSE_ROLL_NONE 0
#define MOUSE_ROLL_UP 1
#define MOUSE_ROLL_DOWN 2
struct MOUSE_DEC {
  unsigned char buf[4], phase;
  int x, y, btn;
  int sleep;
  char roll;
};
struct PCI_CONFIG_SPACE_PUCLIC { //用64个字节描述
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
  unsigned int BaseAddr[6];
  unsigned int CardbusCIS;
  unsigned short SubVendorID;
  unsigned short SubSystemID;
  unsigned int ROMBaseAddr;
  unsigned char CapabilitiesPtr;
  unsigned char Reserved[3];
  unsigned int Reserved1;
  unsigned char InterruptLine;
  unsigned char InterruptPin;
  unsigned char MinGrant;
  unsigned char MaxLatency;
};
typedef struct {
  unsigned short offset;
  unsigned short seg;
} ReadModeFarPointer;
typedef struct {
  unsigned short attributes;
  unsigned char winA, winB;
  unsigned short granularity;
  unsigned short winsize;
  unsigned short segmentA, segmentB;
  /* In VBE Specification, this field should be
   * ReadModeFarPointer winPosFunc;
   * However, we overwrite this field in loader n*/
  unsigned short mode;
  unsigned short reserved2;
  unsigned short bytesPerLine;
  unsigned short width, height;
  unsigned char Wchar, Ychar, planes, bitsPerPixel, banks;
  unsigned char memory_model, bank_size, image_pages;
  unsigned char reserved0;
  unsigned char red_mask, red_position;
  unsigned char green_mask, green_position;
  unsigned char blue_mask, blue_position;
  unsigned char rsv_mask, rsv_position;
  unsigned char directcolor_attributes;
  unsigned int physbase; // your LFB (Linear Framebuffer) address ;)
  unsigned int offscreen;
  unsigned short offsize;

} __attribute__((packed)) VESAModeInfo;
typedef struct {
  unsigned char signature[4];
  unsigned short Version;
  ReadModeFarPointer oemString;
  unsigned int capabilities;
  ReadModeFarPointer videoModes;
  unsigned short totalMemory;
  unsigned short OEMVersion;
  ReadModeFarPointer vendor;
  ReadModeFarPointer product;
  ReadModeFarPointer revision;
  /* In VBE Specification, this field should be reserved.
   * However, we overwrite this field in loader */
  unsigned short modeCount;
  unsigned char reserved0[220];
  unsigned char oemUse[256];
  VESAModeInfo modeList[0];
} __attribute__((packed)) VESAControllerInfo;
struct VBEINFO {
  char res1[18];
  short xsize, ysize;
  char res2[18];
  int vram;
};
#define VBEINFO_ADDRESS 0x7e00
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
#define VGA_NUM_REGS                                                           \
  (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)
#define _vmemwr(DS, DO, S, N) memcpy((char *)((DS)*16 + (DO)), S, N)
#define SB16_IRQ 5
#define SB16_FAKE_TID -3
#define SB16_PORT_MIXER 0x224
#define SB16_PORT_DATA 0x225
#define SB16_PORT_RESET 0x226
#define SB16_PORT_READ 0x22A
#define SB16_PORT_WRITE 0x22C
#define SB16_PORT_READ_STATUS 0x22E
#define SB16_PORT_DSP_16BIT_INTHANDLER_IRQ 0x22F
#define COMMAND_DSP_WRITE 0x40
#define COMMAND_DSP_SOSR 0x41
#define COMMAND_DSP_TSON 0xD1
#define COMMAND_DSP_TSOF 0xD3
#define COMMAND_DSP_STOP8 0xD0
#define COMMAND_DSP_RP8 0xD4
#define COMMAND_DSP_STOP16 0xD5
#define COMMAND_DSP_RP16 0xD6
#define COMMAND_DSP_VERSION 0xE1
#define COMMAND_MIXER_MV 0x22
#define COMMAND_SET_IRQ 0x80
#define BUF_RDY_VAL 128
#define MAX_DRIVERS 256
#define DRIVER_USE 1
#define DRIVER_FREE 0
typedef struct driver *drv_t;
typedef int drv_type_t;
struct driver {
  struct TASK *drv_task; // 驱动程序的任务
  drv_type_t drv_type;   // 驱动程序类型
  int flags;             // 驱动程序的状态
};
struct driver_ctl {
  struct driver drivers[MAX_DRIVERS]; // 驱动程序数组
  int driver_num;                     // 驱动程序数量
};
struct arg_struct {
  int func_num;
  void *arg; // 参数(base=0x00)
  int tid;
};
struct InitializationBlock {
  // 链接器所迫 只能这么写了
  uint16_t mode;
  uint8_t reserved1numSendBuffers;
  uint8_t reserved2numRecvBuffers;
  uint8_t mac0, mac1, mac2, mac3, mac4, mac5;
  uint16_t reserved3;
  uint64_t logicalAddress;
  uint32_t recvBufferDescAddress;
  uint32_t sendBufferDescAddress;
} __attribute__((packed));

struct BufferDescriptor {
  uint32_t address;
  uint32_t flags;
  uint32_t flags2;
  uint32_t avail;
} __attribute__((packed));

struct IDEHardDiskInfomationBlock {
  char reserve1[2];
  unsigned short CylinesNum;
  char reserve2[2];
  unsigned short HeadersNum;
  unsigned short TrackBytes;
  unsigned short SectorBytes;
  unsigned short TrackSectors;
  char reserve3[6];
  char OEM[20];
  char reserve4[2];
  unsigned short BuffersBytes;
  unsigned short EECCheckSumLength;
  char Version[8];
  char ID[40];
};

/* gui.h */
struct Button {
  int x, y, w, h;    // 16
  struct SHEET *buf; // 20
  char *text;        // 24
  int cont;          // 28
  bool is_clicking;  // 32
  void (*OnClick)(); // 36
  struct TASK *task; // 40
  // NoFrameButton专有
  color_t bc;
  bool hide;
  bool clicking_first;
} __attribute__((packed));
typedef struct Button Button;
struct TextBox {
  int x, y, w, h;
  int Write_Pos_X;
  struct SHEET *buf;
  char *text;
  int len;
  bool is_clicking;
  int cont;
};
typedef struct TextBox TextBox;
#define CMDLINEXSIZE 650
#define CMDLINEYSIZE 429
#define FMXSIZE 350
#define FMYSIZE 355

#define COL_000000 0x00000000
#define COL_FF0000 0x00ff0000
#define COL_00FF00 0x0000ff00
#define COL_FFFF00 0x00ffff00
#define COL_0000FF 0x000000ff
#define COL_FF00FF 0x00ff00ff
#define COL_00FFFF 0x0000ffff
#define COL_C6C6C6 0x00c6c6c6
#define COL_848484 0x00848484
#define COL_840000 0x00840000
#define COL_008400 0x00008400
#define COL_848400 0x00848400
#define COL_000084 0x00000084
#define COL_840084 0x00840084
#define COL_008484 0x00008484
#define COL_FFFFFF 0x00ffffff
// 透明色
#define COL_TRANSPARENT 0x50ffffff

typedef struct listBox_t {
  int x;
  int y;
  int width;
  int height;
  int view_max;
  struct SHEET *sheet;
  int item_num;
  int now_min;
  int now_max;
  int cont;
  char **item;
  Button **btns;
  int itm_arr_sz;
  Button *button1; // 上
  Button *button2; // 下
} listBox_t;

typedef struct GUI_Position {
  uint32_t x;
  uint32_t y;
  uint32_t w;
  uint32_t h;
} GuiPosition;
typedef struct loadBox {
  GuiPosition position;
  // load进度
  uint8_t load_progress;
  uint32_t count; // 在链表中的位置

  struct SHEET *sheet;
} loadBox;

/* net */
#define swap32(x)                                                              \
  ((((x)&0xff000000) >> 24) | (((x)&0x00ff0000) >> 8) |                        \
   (((x)&0x0000ff00) << 8) | (((x)&0x000000ff) << 24))
#define swap16(x) ((((x)&0xff00) >> 8) | (((x)&0x00ff) << 8))
// 以太网帧
struct EthernetFrame_head {
  uint8_t dest_mac[6];
  uint8_t src_mac[6];
  uint16_t type;
} __attribute__((packed));
// 以太网帧--尾部
struct EthernetFrame_tail {
  uint32_t CRC; // 这里可以填写为0，网卡会自动计算
};
// ARP
#define ARP_PROTOCOL 0x0806
#define MAX_ARP_TABLE 256
#define ARP_WAITTIME 10
struct ARPMessage {
  uint16_t hardwareType;
  uint16_t protocol;
  uint8_t hardwareAddressSize;
  uint8_t protocolAddressSize;
  uint16_t command;
  uint8_t src_mac[6];
  uint32_t src_ip;
  uint8_t dest_mac[6];
  uint32_t dest_ip;
} __attribute__((packed));
// IPV4
#define IP_PROTOCOL 0x0800
#define MTU 1500
#define IP_MF 13
#define IP_DF 14
#define IP_OFFSET 0
struct IPV4Message {
  uint8_t headerLength : 4;
  uint8_t version : 4;
  uint8_t tos;
  uint16_t totalLength;
  uint16_t ident;
  uint16_t flagsAndOffset;
  uint8_t timeToLive;
  uint8_t protocol;
  uint16_t checkSum;
  uint32_t srcIP;
  uint32_t dstIP;
} __attribute__((packed));
// ICMP
#define ICMP_PROTOCOL 1
struct ICMPMessage {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t ID;
  uint16_t sequence;
} __attribute__((packed));
#define PING_WAITTIME 2000
#define PING_ID 0x0038
#define PING_SEQ 0x2115
#define PING_DATA 0x38
#define PING_SIZE 28
// UDP
#define UDP_PROTOCOL 17
struct UDPMessage {
  uint16_t srcPort;
  uint16_t dstPort;
  uint16_t length;
  uint16_t checkSum;
} __attribute__((packed));
// DHCP
#define DHCP_CHADDR_LEN 16
#define DHCP_SNAME_LEN 64
#define DHCP_FILE_LEN 128
struct DHCPMessage {
  uint8_t opcode;
  uint8_t htype;
  uint8_t hlen;
  uint8_t hops;
  uint32_t xid;
  uint16_t secs;
  uint16_t flags;
  uint32_t ciaddr;
  uint32_t yiaddr;
  uint32_t siaddr;
  uint32_t giaddr;
  uint8_t chaddr[DHCP_CHADDR_LEN];
  char bp_sname[DHCP_SNAME_LEN];
  char bp_file[DHCP_FILE_LEN];
  uint32_t magic_cookie;
  uint8_t bp_options[0];
} __attribute__((packed));
#define DHCP_BOOTREQUEST 1
#define DHCP_BOOTREPLY 2

#define DHCP_HARDWARE_TYPE_10_EHTHERNET 1

#define MESSAGE_TYPE_PAD 0
#define MESSAGE_TYPE_REQ_SUBNET_MASK 1
#define MESSAGE_TYPE_ROUTER 3
#define MESSAGE_TYPE_DNS 6
#define MESSAGE_TYPE_DOMAIN_NAME 15
#define MESSAGE_TYPE_REQ_IP 50
#define MESSAGE_TYPE_DHCP 53
#define MESSAGE_TYPE_PARAMETER_REQ_LIST 55
#define MESSAGE_TYPE_END 255

#define DHCP_OPTION_DISCOVER 1
#define DHCP_OPTION_OFFER 2
#define DHCP_OPTION_REQUEST 3
#define DHCP_OPTION_PACK 4

#define DHCP_SERVER_PORT 67
#define DHCP_CLIENT_PORT 68

#define DHCP_MAGIC_COOKIE 0x63825363
// DNS
#define DNS_Header_ID 0x2115

#define DNS_TYPE_A 1
#define DNS_TYPE_NS 2
#define DNS_TYPE_MD 3
#define DNS_TYPE_MF 4
#define DNS_TYPE_CNAME 5
#define DNS_TYPE_SOA 6
#define DNS_TYPE_MB 7
#define DNS_TYPE_MG 8
#define DNS_TYPE_MR 9
#define DNS_TYPE_NULL 10
#define DNS_TYPE_WKS 11
#define DNS_TYPE_PTR 12
#define DNS_TYPE_HINFO 13
#define DNS_TYPE_MINFO 14
#define DNS_TYPE_MX 15
#define DNS_TYPE_TXT 16
#define DNS_TYPE_ANY 255

#define DNS_CLASS_INET 1
#define DNS_CLASS_CSNET 2
#define DNS_CLASS_CHAOS 3
#define DNS_CLASS_HESIOD 4
#define DNS_CLASS_ANY 255

#define DNS_PORT 53
#define DNS_SERVER_IP 0x08080808
struct DNS_Header {
  uint16_t ID;
  uint8_t RD : 1;
  uint8_t AA : 1;
  uint8_t Opcode : 4;
  uint8_t QR : 1;
  uint8_t RCODE : 4;
  uint8_t Z : 3;
  uint8_t RA : 1;
  uint8_t TC : 1;
  uint16_t QDcount;
  uint16_t ANcount;
  uint16_t NScount;
  uint16_t ARcount;
} __attribute__((packed));
struct DNS_Question {
  uint16_t type;
  uint16_t Class;
} __attribute__((packed));
struct DNS_Answer {
  uint32_t name : 24;
  uint16_t type;
  uint16_t Class;
  uint32_t TTL;
  uint16_t RDlength;
  uint8_t RData[0];
} __attribute__((packed));
// TCP
#define TCP_PROTOCOL 6
#define TCP_CONNECT_WAITTIME 10000
#define MSS_Default 1460
#define TCP_SEG_WAITTIME 100
struct TCPPesudoHeader {
  uint32_t srcIP;
  uint32_t dstIP;
  uint16_t protocol;
  uint16_t totalLength;
} __attribute__((packed));
struct TCPMessage {
  uint16_t srcPort;
  uint16_t dstPort;
  uint32_t seqNum;
  uint32_t ackNum;
  uint8_t reserved : 4;
  uint8_t headerLength : 4;
  uint8_t FIN : 1;
  uint8_t SYN : 1;
  uint8_t RST : 1;
  uint8_t PSH : 1;
  uint8_t ACK : 1;
  uint8_t URG : 1;
  uint8_t ECE : 1;
  uint8_t CWR : 1;
  uint16_t window;
  uint16_t checkSum;
  uint16_t pointer;
  uint32_t options[0];
} __attribute__((packed));
// Socket
#define MAX_SOCKET_NUM 256
struct Socket {
  // 函数格式
  int (*Connect)(struct Socket *socket);                             // TCP
  void (*Disconnect)(struct Socket *socket);                         // TCP
  void (*Listen)(struct Socket *socket);                             // TCP
  void (*Send)(struct Socket *socket, uint8_t *data, uint32_t size); // TCP/UDP
  void (*Handler)(struct Socket *socket, void *base);                // TCP/UDP
  // TCP/UDP
  uint32_t remoteIP;
  uint16_t remotePort;
  uint32_t localIP;
  uint16_t localPort;
  uint8_t state;
  uint8_t protocol;
  // TCP
  uint32_t seqNum;
  uint32_t ackNum;
  uint16_t MSS;
} __attribute__((packed));
// UDP state
#define SOCKET_ALLOC -1
// UDP/TCP state
#define SOCKET_FREE 0
// TCP state
#define SOCKET_TCP_CLOSED 1
#define SOCKET_TCP_LISTEN 2
#define SOCKET_TCP_SYN_SENT 3
#define SOCKET_TCP_SYN_RECEIVED 4
#define SOCKET_TCP_ESTABLISHED 5
#define SOCKET_TCP_FIN_WAIT1 6
#define SOCKET_TCP_FIN_WAIT2 7
#define SOCKET_TCP_CLOSING 8
#define SOCKET_TCP_TIME_WAIT 9
#define SOCKET_TCP_CLOSE_WAIT 10
#define SOCKET_TCP_LAST_ACK 11
// Socket Server
#define SOCKET_SERVER_MAX_CONNECT 32
struct SocketServer {
  struct Socket *socket[SOCKET_SERVER_MAX_CONNECT];
  void (*Send)(struct SocketServer *server, uint8_t *data,
               uint32_t size); // TCP/UDP
};
// Http
typedef struct HTTPGetHeader {
  bool ok;
  char path[13];
} HTTPGetHeader;
// ntp
struct NTPMessage {
  uint8_t VN : 3;
  uint8_t LI : 2;
  uint8_t Mode : 3;
  uint8_t Startum;
  uint8_t Poll;
  uint8_t Precision;
  uint32_t Root_Delay;
  uint32_t Root_Difference;
  uint32_t Root_Identifier;
  uint64_t Reference_Timestamp;
  uint64_t Originate_Timestamp;
  uint64_t Receive_Timestamp;
  uint64_t Transmission_Timestamp;
} __attribute__((packed));
#define NTPServer1 0xA29FC87B
#define NTPServer2 0x727607A3
// ftp
#define FTP_PORT_MODE 1
#define FTP_PASV_MODE 2
#define FTP_SERVER_DATA_PORT 20
#define FTP_SERVER_COMMAND_PORT 21
struct FTP_Client {
  int (*Login)(struct FTP_Client *ftp_c_, uint8_t *user, uint8_t *pass);
  int (*TransModeChoose)(struct FTP_Client *ftp_c_, int mode);
  void (*Logout)(struct FTP_Client *ftp_c_);
  int (*Download)(struct FTP_Client *ftp_c_, uint8_t *path_pdos,
                  uint8_t *path_ftp, int mode);
  int (*Upload)(struct FTP_Client *ftp_c_, uint8_t *path_pdos,
                uint8_t *path_ftp, int mode);
  int (*Delete)(struct FTP_Client *ftp_c_, uint8_t *path_ftp);
  uint8_t *(*Getlist)(struct FTP_Client *ftp_c_);
  struct Socket *socket_cmd;
  struct Socket *socket_dat;
  bool using1;
  bool is_login;
  uint8_t *recv_buf_cmd;
  bool recv_flag_cmd;
  uint32_t reply_code;
  uint8_t *recv_buf_dat;
  bool recv_flag_dat;
  uint32_t recv_dat_size;
};
typedef struct {
  void (*Read)(char drive, unsigned char *buffer, unsigned int number,
               unsigned int lba);
  void (*Write)(char drive, unsigned char *buffer, unsigned int number,
                unsigned int lba);
  int flag;
  unsigned int size; // 大小
  char DriveName[50];
} vdisk;
#endif