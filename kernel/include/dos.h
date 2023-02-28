#ifndef _DOS_H
#define _DOS_H
#include <cmd.h>
#include <define.h>
#include <drivers.h>
#include <fs.h>
#include <gui.h>
#include <interrupts.h>
#include <io.h>
#include <net.h>
#include <stdio.h>
#include <string.h>

// gdtidt.c
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base,
                  int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void init_pic(void);
void init_gdtidt(void);
void register_intr_handler(int num, int addr);
// timer.c
void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);
// task.c
void mt_init(void);
void mt_taskswitch1();
void mt_taskswitch2();
void mt_taskswitch3();
int Get_Running_Task_Num();
struct TASK *NowTask();
void TaskSetFIFO(struct TASK *task, struct FIFO8 *keyfifo,
                 struct FIFO8 *mousefifo);
struct FIFO8 *TaskGetKeyfifo(struct TASK *task);
struct FIFO8 *TaskGetMousefifo(struct TASK *task);
struct TASK *AddTask(char *name, int level, int cs, int eip, int ds, int ss,
                     int esp);
void SleepTask(struct TASK *task);
void WakeUp(struct TASK *task);
void SleepTaskFIFO(struct TASK *task);
struct TASK *GetTask(int taskNum);
struct TASK *GetTaskForName(char *taskname);
struct TASK *GetTask_NoSafe(int taskNum);
void __SubTask(struct TASK *task);
void SubTask(struct TASK *task);
void ChangeLevel(struct TASK *task, int nlevel);
void RunTask(struct TASK *task);
struct TASK *_fork(int b);
#ifndef _TASK_C
#define fork() _fork(0)
#else

#endif
struct TASK *clone_task(struct TASK *tk, int stack_sz);
// page.c
void C_init_page();
void pf_set(unsigned int memsize);
int get_line_address(int t, int p, int o);
int get_page_from_line_address(int line_address);
void page2tpo(int page, int *t, int *p);
void tpo2page(int *page, int t, int p);
void *page_malloc_one();
int get_pageinpte_address(int t, int p);
void page_free_one(void *p);
int find_kpage(int line, int n);
void *page_kmalloc(int size);
void page_kfree(int p, int size);
void page_map(void *target, void *start, void *end);
void *page_malloc_lessthan4kb(int size);
void page_free_lessthan4kb(void *p, int size);
void *page_malloc(int size);
void page_free(void *p, int size);
void change_page_task_id(int task_id, void *p, unsigned int size);
void PF(uint32_t eax);
// nasmfunc.asm
void int32(unsigned char intnum, regs16_t *regs);
void floppy_int(void);
int get_eip();
void farjmp(int eip, int cs);
void farcall(int eip, int cs);
void asm_inthandler36();
void asm_gui_api();
void asm_net_api();
void asm_inthandler2c();
void asm_inthandler20();
void asm_inthandler21();
void asm_rtc_handler();
void asm_ide_irq();
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
void null_inthandler();
void PCNET_ASM_INTHANDLER(void);
void asm_sb16_handler();
void asm_rtc_handler();
void __init_PIT();
void init_float();
void switch_stack_app();
void switch_stack_sys();

// other.c
void INT(unsigned char intnum, regs16_t *regs);
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
void ERROR16();
void ERROR17();
void ERROR18();
void ERROR19();
void ERROR(int CODE, char *TIPS);
void KILLAPP(int ec, int eip);
void KILLAPP0(int ec, int tn);
void insert_char(char *str, int pos, char ch); // str:字符串，pos:位置，ch:字符
void delete_char(char *str, int pos);          // str:字符串，pos:位置
char bcd2hex(char bcd);
char hex2bcd(char hex);
void getCPUBrand(char *cBrand);
char ascii2num(char c);
char num2ascii(char c);
void strtoupper(char *str);
int GetCHorEN(unsigned char *str);
// syscall.c
void inthandler36(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx,
                  int eax);
// fifo.c
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);
// list.c
void AddVal(int val, struct List *Obj);
struct List *FindForCount(int count, struct List *Obj);
void DeleteVal(int count, struct List *Obj);
struct List *NewList();
void Change(int count, struct List *Obj, int val);
int GetLastCount(struct List *Obj);
// init.c
void sysinit();
bool SetDrive(unsigned char *name);
unsigned int GetDriveCode(unsigned char *name);
bool DriveSemaphoreTake(unsigned int drive_code);
void DriveSemaphoreGive(unsigned int drive_code);
int disk_Size(char drive);
void Disk_Read(unsigned int lba, unsigned int number, void *buffer, char drive);
void Disk_Write(unsigned int lba, unsigned int number, void *buffer,
                char drive);
bool DiskReady(char drive);
// kernelc.c
void shell(void);
void shell_handler();
struct tty *now_tty();
void task_sr1();
void task_sr2();
// mem.c
unsigned int memtest(unsigned int start, unsigned int end);
void *memcpy(void *s, const void *ct, size_t n);
void *malloc(int size);
void free(void *p);
// ipc.c
int SendIPCMessage(int to_tid, void *data, unsigned int size, char type);
int SendIPCMessageForName(char *tname, void *data, unsigned int size,
                          char type);
int GetIPCMessage(void *data, int from_tid);
int GetIPCMessageForName(void *data, char *tname);
int IPCMessageStatus();
unsigned int IPCMessageLength(int from_tid);
int SendIPCMessageTID(int to_tid,        // 收信人
                      int y_tid,         // 发信人
                      void *data,        // 数据
                      unsigned int size, // 大小
                      char type /* 类型 */);
// arg.c
int Get_Arg(char *Arg, char *CmdLine, int Count);
int Get_Argc(char *CmdLine);
// time.c
unsigned time(void);
// rand.c
unsigned int rand(void);
void srand(unsigned long seed);
// md5.c
void md5s(char *hexbuf, int read_len, char *result);
void md5f(char *filename, unsigned char *result);
// lock.c
bool cas(int *ptr, int old, int New);
void lock();
void unlock();
#endif
