#include "system.h"
struct TIMER *mt_timer1,*mt_timer2,*mt_timer3;
int mt_tr1,mt_tr2,mt_tr3;
int taskctl;
int tasknum = 0; // 0任务（GDT103*8）留给系统
#define EFLAGS_VM_MASK 0x00020000
void mt_init(void)
{
	extern struct TASK normal;
	// 对多任务调度的初始化
	strcpy(normal.name, "Normal");
	normal.sel = 103 * 8;
	normal.level = 1;
	normal.sleep = 1;
	normal.fifosleep = 1;
    mt_tr3 = 103*8;
	mt_tr2 = 103*8;
	mt_tr1 = 103*8;
	mt_timer1 = timer_alloc();
	timer_settime(mt_timer1,1);
	mt_timer2 = timer_alloc();
	timer_settime(mt_timer2,3);
	mt_timer3 = timer_alloc();
	timer_settime(mt_timer3,8);
}
void mt_taskswitch1()
{
	if (tasknum == 0) {
		timer_settime(mt_timer1,1);
		return;
	}
	for (int i = mt_tr1/8-103; i < tasknum+1; i++) {
	// 优先级1的执行
		struct TASK *task = GetTask(mt_tr1/8-103);
		if (task->level == 1 && task->sleep == 0) {	// 找到level=1的任务并且没有休眠
			taskctl = mt_tr1;
			mt_tr1 += 8;
//			printk("Level:1 Task:%d*8\n",taskctl/8);
			timer_settime(mt_timer1,1);
			farjmp(0,taskctl);
			return;
		}
		mt_tr1 += 8;
	}
	// 无优先级1的任务
	mt_tr1 = 103*8;	// mt_tr1复原
	timer_settime(mt_timer1,1);
}
void mt_taskswitch2()
{
	if (tasknum == 0) {
		timer_settime(mt_timer2,3);
		timer_settime(mt_timer1,1);
		return;
	}
	for (int i = mt_tr2/8-103; i < tasknum+1; i++) {
	// 优先级2的执行
		struct TASK *task = GetTask(mt_tr2/8-103);
		if (task->level == 2 && task->sleep == 0) {	// 找到level=2的任务并且没有休眠
			taskctl = mt_tr2;
			mt_tr2 += 8;
//			printk("Level:2 Task:%d*8\n",taskctl/8);
			timer_settime(mt_timer2,3);
			timer_settime(mt_timer1,1);
			farjmp(0,taskctl);	// 跳转
			return;
		}
		mt_tr2 += 8;
	}
	// 无优先级2的任务
	mt_tr2 = 103*8;	// mt_tr2复原
	timer_settime(mt_timer2,3);
	timer_settime(mt_timer1,1);
}
void mt_taskswitch3()
{
	extern int mt2flag;
	if (tasknum == 0) {
		timer_settime(mt_timer3,8);
		if (mt2flag == 1) {
			timer_settime(mt_timer2,3);
		}
		timer_settime(mt_timer1,1);
		return;
	}
	for (int i = mt_tr3/8-103; i < tasknum+1; i++) {
	// 优先级3的执行
		struct TASK *task = GetTask(mt_tr3/8-103);
		if (task->level == 3 && task->sleep == 0) {	// 找到level=3的任务并且没有休眠
			taskctl = mt_tr3;
			mt_tr3 += 8;
//			printk("Level:3 Task:%d*8\n",taskctl/8);
			timer_settime(mt_timer3,8);
			if (mt2flag == 1) {
				timer_settime(mt_timer2,3);
			}
			timer_settime(mt_timer1,1);
			farjmp(0,taskctl);	// 跳转
			return;
		}
		mt_tr3 += 8;
	}
	// 无优先级3的任务
	mt_tr3 = 103*8;	// mt_tr3复原
	timer_settime(mt_timer3,8);
	if (mt2flag == 1) {
		timer_settime(mt_timer2,3);
	}
	timer_settime(mt_timer1,1);
}
int Get_Running_Task_Num()
{
	int ret = 0;
	for (int i = 0; i <= tasknum; i++)
	{
		if (GetTask(i)->sleep == 0)
		{
			ret++;
		}
	}
	return ret;
}
struct TASK *NowTask()
{
	return GetTask(taskctl/8-103);
}
void TaskSetFIFO(struct TASK *task, struct FIFO8 *keyfifo, struct FIFO8 *mousefifo)
{
	task->keyfifo = keyfifo;
	task->mousefifo = mousefifo;
}
struct FIFO8 *TaskGetKeyfifo(struct TASK *task)
{
	return (struct FIFO8 *)task->keyfifo;
}
struct FIFO8 *TaskGetMousefifo(struct TASK *task)
{
	return (struct FIFO8 *)task->mousefifo;
}
struct TASK *AddTask(char *name, int level, int cs, int eip, int ds, int ss, int esp)
{
	Maskirq(0);
	tasknum++;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	struct TASK *task = (struct TASK *)page_kmalloc(sizeof(struct TASK));
	// GDT 0~2 系统用
	// GDT 3~103 程序用
	// GDT 104 ~ TSS用
	set_segmdesc(gdt + 103 + tasknum, 103, (int)&task->tss, AR_TSS32);
	task->sel = (103 + tasknum) * 8;
	task->level = level;
	task->sleep = 0; // 这个任务没有休眠
	task->fifosleep = 0;
	task->tss.eflags = 0x00000202;
	task->tss.cr3 = PDE_ADDRESS;	// PAE分页（二级分页
	task->tss.eax = 0;
	task->tss.ebx = 0;
	task->tss.ecx = 0;
	task->tss.edx = 0;
	task->tss.esp = esp;
	task->tss.ebp = 0;
	task->tss.esi = 0;
	task->tss.edi = 0;
	task->tss.ds = ds;
	task->tss.es = ds;
	task->tss.fs = ds;
	task->tss.gs = ds;
	task->tss.ss = ss;
	task->tss.cs = cs;
	task->tss.eip = eip;
	task->tss.ldtr = 0;
	task->tss.iomap = 0x40000000;
	task->running = 1;
	if (strlen(name) > 32)
	{
		strncpy(task->name, name, 32);
	}
	else
	{
		strcpy(task->name, name);
	}
	ClearMaskIrq(0);
	return task;
}
void SleepTask(struct TASK *task)
{
	if (Get_Running_Task_Num() == 1)
		return;
	task->sleep = 1; // 任务休眠
	task->fifosleep = 1;
}
void WakeUp(struct TASK *task)
{
	task->sleep = 0;
	task->fifosleep = 0;
	return;
}
void SleepTaskFIFO(struct TASK *task)
{
	task->fifosleep = 1;
}
struct TASK *GetTask(int taskNum)
{
	if (taskNum > tasknum)
		return 0;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT + (103 + taskNum);

//	printf("gdt->base_low:%04x\n",(unsigned short)gdt->base_low);
//	printf("gdt->base_mid:%02x\n",(unsigned char)gdt->base_mid);
	//printf("gdt->base_high:%02x\n",(unsigned char)gdt->base_high);
	//将他们转换成32位的值
	unsigned char res1[4];

	//注意大小端
	res1[0] = ((unsigned char *)&gdt->base_low)[0];
	res1[1] = ((unsigned char *)&gdt->base_low)[1];
	res1[2] = (unsigned char)gdt->base_mid;
	res1[3] = (unsigned char)gdt->base_high;
	unsigned int res2 = *(unsigned int *)res1;
	return (struct TASK *)(res2-0xc);
}
struct TASK *GetTaskForName(char *taskname)
{
	for(int i = 0; i <= tasknum; i++)
	{
		if(strcmp(GetTask(i)->name, taskname) == 0)
		{
			return GetTask(i);
		}
	}
	return 0;
}
struct TASK *GetTask_NoSafe(int taskNum)
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT + (103 + taskNum);
	return (int)gdt->base_low + (gdt->base_mid << 16) + (gdt->base_high << 24) - 12;
}
void SubTask(struct TASK *task)
{
	Maskirq(0);	// 关闭时钟中断 防止有任务来打扰
	SleepTask(task);
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	int bmpNum = tasknum;
	int index = tasknum - (task->sel / 8 - 103) - 1;
	tasknum-=index;
	if (index == 0) {	// 最后一个
		// gdt描述符清空
		tasknum--;
		set_segmdesc(gdt + 103 + bmpNum, 0, 0, 0);
		ClearMaskIrq(0);
		return;
	}
	//printf("\nTASKSEL:%d*8 TASKNUM:%d INDEX:%d BMPNUM:%d\n",task->sel/8,tasknum,index,bmpNum);
	page_kfree((int)task, sizeof(struct TASK));
	for (int i = bmpNum - index + 1; i < bmpNum + index; i++)
	{
		page_kfree((int)GetTask_NoSafe(i),sizeof(struct TASK));
		struct TASK *ntask = AddTask(GetTask_NoSafe(i)->name, GetTask_NoSafe(i)->level, 
		GetTask_NoSafe(i)->tss.cs, GetTask_NoSafe(i)->tss.eip, 
		GetTask_NoSafe(i)->tss.ds, GetTask_NoSafe(i)->tss.ss, 
		GetTask_NoSafe(i)->tss.esp);
		ntask->sleep = GetTask_NoSafe(i)->sleep;
		ntask->fifosleep = GetTask_NoSafe(i)->fifosleep;
		ntask->keyfifo = GetTask_NoSafe(i)->keyfifo;
		ntask->mousefifo = GetTask_NoSafe(i)->mousefifo;
	}
	tasknum-=2;
	set_segmdesc(gdt + 103 + bmpNum, 0, 0, 0);
	ClearMaskIrq(0);
}
