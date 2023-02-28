#ifndef __DOSLDR__H
#define __DOSLDR__H
#define page_kmalloc page_malloc
#define page_kfree   page_free
#define page_malloc_one() page_malloc(4)
typedef unsigned int size_t;
#define NULL 0
#define bool int
#define true 1
#define false 0
#define _INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, v) (ap = (va_list)&v + _INTSIZEOF(v))
#define va_arg(ap, t) (*(t*)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)))
#define va_end(ap) (ap = (va_list)0)
#define va_copy(d, s) __builtin_va_copy((d), (s))
#define va_list char*
typedef unsigned int size_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long long uint64_t;
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
void io_hlt(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
#define ADR_IDT 0x0026f800
#define LIMIT_IDT 0x000007ff
#define ADR_GDT 0x00270000
#define LIMIT_GDT 0x0000ffff
#define ADR_BOTPAK 0x100000
#define LIMIT_BOTPAK 0x0007ffff
#define AR_DATA32_RW 0x4092
#define AR_DATA16_RW 0x0092
#define AR_CODE32_ER 0x409a
#define AR_CODE16_ER 0x009a
#define AR_INTGATE32 0x008e
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
#define MEMMAN_FREES 4090

#define MEMMAN_ADDR 0x0005c0000
struct FREEINFO {
  unsigned int addr, size;
};
struct MEMMAN {
  int frees, maxfrees, lostsize, losts;
  struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN* man);
unsigned int memman_total(struct MEMMAN* man);
unsigned int memman_alloc(struct MEMMAN* man, unsigned int size);
int memman_free(struct MEMMAN* man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN* man, unsigned int size);
int memman_free_4k(struct MEMMAN* man, unsigned int addr, unsigned int size);
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
struct DRIVE_MEMBER {
  unsigned int ADR_DISKIMG;
  struct FILEINFO* root_directory;
  struct LIST* directory_list;
  struct LIST* directory_clustno_list;
  int* fat;
  unsigned int ClustnoBytes;
  unsigned short RootMaxFiles;
  unsigned int RootDictAddress;
  unsigned int FileDataAddress;
  unsigned int imgTotalSize;
  unsigned short SectorBytes;
  unsigned int Fat1Address, Fat2Address;
  unsigned char* FatClustnoFlags;
};
#define MAX_DRIVE_NUM 26
struct DRIVE_CTL {
  struct DRIVE_MEMBER drives[MAX_DRIVE_NUM];
};
extern struct DRIVE_CTL drive_ctl;
struct FILEINFO {
  unsigned char name[8], ext[3], type;
  char reserve[10];
  unsigned short time, date, clustno;
  unsigned int size;
};
struct TASK {
  int drive_number;
  char drive;
  struct FILEINFO* directory;
  int change_dict_times;
  char path[256];
} __attribute__((packed));
struct ListCtl {
  struct List* start;
  struct List* end;
  int all;
};
struct List {
  struct ListCtl* ctl;
  struct List* prev;
  int val;
  struct List* next;
};
typedef struct List List;
typedef struct FILE {
  unsigned char* buf;  // 文件缓冲区
  int size;            // 文件大小
  int p;               // 当前读写位置
  int realloc;         // 内存区域大小
  char* path;          // 文件名
} FILE;
int printf(const char* format, ...);
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
struct TASK* NowTask();
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
FILE *fopen(char *path, char *mode);
int fputc(int c, FILE *fp);
int fgetc(FILE *fp);
char *fgets(char *s, int size, FILE *fp);
int fseek(FILE *fp, int offset, int whence);
int fclose(FILE *fp);
int fread(void *buf, int size, int count, FILE *fp);
void EDIT_FILE(char *name, char *dest, int length, int offset);
void longName28dot3(char *result, char *src_in);
int fsz(char *filename);
int ftell(FILE *fp);
void rewind(FILE *fp);
#endif