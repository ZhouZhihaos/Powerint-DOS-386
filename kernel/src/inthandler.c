/**
 * Powerint DOS 386 中断处理程序
 * @author: min0911 & zhouzhihao
 * @date: 2022.4.5
 * NOTE: 这里存放Powerint DOS除iRQ0、IRQ6以外的所有中断处理程序
 * NOTE: IRQ0在kernel/src/timer.c中实现
 * NOTE: IRQ6在kernel/src/floppy.c中实现(flint函数)
*/
#include "system.h"
extern int tasknum;
void inthandler21(int *esp)
{
    extern int pressed;
	unsigned char data, s[4];
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(PORT_KEYDAT);
	if (data < 0x80)
	{
		pressed = 1;
	}
	if (data == 0x2a || data == 0x36)
	{
		shift = 1;
		return;
	}
	if (data == 0x3a)
	{ // Caps Lock
		caps_lock = caps_lock ^ 1;
		return;
	}
	if (data == 0xaa || data == 0xb6)
	{ // shift
		shift = 0;
		return;
	}
	// ctrl
	extern int app_task_num;
	if (data == 0x3b)
	{
		if (app_task_num != -1)
		{
			KILLAPP0(0xff, app_task_num);
		}
		return;
	}
	for (int i = 1; i < tasknum+1; i++) {
		struct TASK *task = GetTask(i);
		if (task->sleep == 1 || task->fifosleep == 1)
			continue;
		fifo8_put(TaskGetKeyfifo(task),data);
		//printf("TASK NAME:%s TASK FIFO:%08x STATUS:%d DATA:%c\n",task->name,TaskGetKeyfifo(task),fifo8_status(TaskGetKeyfifo(task)),data);
	}

	return;
}
void inthandler2c(int *esp)
{
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);
	io_out8(PIC0_OCW2, 0x62);
	data = io_in8(PORT_KEYDAT);
	if (mdec.sleep == 0)
	{
		for (int i = 1; i < tasknum+1; i++) {
			struct TASK *task = GetTask(i);
			if (task->sleep == 1 || task->fifosleep == 1)
				continue;
			fifo8_put(TaskGetMousefifo(task),data);
			//printf("TASK NAME:%s TASK FIFO:%08x STATUS:%d DATA:%c\n",task->name,TaskGetMousefifo(task),fifo8_status(TaskGetMousefifo(task)),data);
		}
	}
	return;
}