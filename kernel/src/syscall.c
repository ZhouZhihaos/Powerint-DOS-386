#include "system.h"
int buf[32] = {0};
int s[32]={0};
int keybuf[32]={0};
int mousebuf[32]={0};
int keyfifo[32]={0};
int mousefifo[32]={0};
void inthandler36(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	// PowerintDOS API
	struct TASK *task = NowTask();
	int cs_base = *((int *)(0xfe8));
	int ds_base = *((int *)(0xfec));
	int alloc_addr = *((int *)(0xff0)); // malloc地址
	if (eax == 0x01)
	{
		intreturn(eax, ebx, ecx, 0x302e3563, esi, edi, ebp); // 0.5c
	}
	else if (eax == 0x02)
	{
		printchar((edx & 0x000000ff));
	}
	else if (eax == 0x03)
	{
		if (ebx == 0x01)
		{
			SwitchToText8025();
		}
		else if (ebx == 0x02)
		{
			SwitchTo320X200X256();
		}
		else if (ebx == 0x03)
		{
			Draw_Char(ecx, edx, esi, edi);
		}
		else if (ebx == 0x04)
		{
			PrintChineseChar(ecx, edx, edi, esi);
		}
		else if (ebx == 0x05)
		{
			Draw_Box(ecx, edx, esi, edi, ebp);
		}
		else if (ebx == 0x06)
		{
			Draw_Px(ecx, edx, esi);
		}
		else if (ebx == 0x07)
		{
			Draw_Str(ecx, edx, (char *)esi + ds_base, edi);
		}
		else if (ebx == 0x08)
		{
			PrintChineseStr(ecx, edx, edi, (char *)esi + ds_base);
		}
	}
	else if (eax == 0x04)
	{
		gotoxy(edx, ecx);
	}
	else if (eax == 0x05)
	{
		print((char *)edx + ds_base);
	}
	else if (eax == 0x06)
	{
		sleep(edx);
	}
	else if (eax == 0x08)
	{
		int i, len;
		ecx = (ecx + 128) / 128;
		if (ecx > 1024)
		{
			intreturn(eax, ebx, ecx, 0, esi, edi, ebp);
		}
		for (len = ecx, i = 0; i < 1024; i++)
		{
			if (len == 0)
			{
				for (len = 0; len < ecx; len++)
				{
					pro_memman[i - 1 - len] = 0xff;
				}
				intreturn(eax, ebx, ecx, (alloc_addr - ds_base) + (i - ecx) * 128, esi, edi, ebp);
				i = 0;
				break;
			}
			if (pro_memman[i] == 0x00)
			{
				len--;
			}
			else if (pro_memman[i] == 0xff && len != 0)
			{
				len = ecx;
			}
		}
		if (i == 1023)
		{
			intreturn(eax, ebx, ecx, 0, esi, edi, ebp);
		}
	}
	else if (eax == 0x09)
	{
		int i;
		ecx = (ecx + 128) / 128;
		int ad = edx;
		edx = (edx + ds_base - alloc_addr) / 128;
		for (i = edx; i < ecx + edx; i++)
		{
			pro_memman[i] = 0x00;
			clean((char *)(ad + ds_base),128);
		}
	}
	else if (eax == 0x0c)
	{
		int i, j;
		for (i = ebx; i < edx; i++)
		{
			for (j = ecx; j < esi; j++)
			{
				*(char *)(0xb8000 + i * 160 + j * 2 + 1) = (char)(edi & 0x000000ff);
			}
		}
	}
	else if (eax == 0x0e)
	{
		intreturn(eax, ebx, get_y(), get_x(), esi, edi, ebp);
	}
	else if (eax == 0x0d)
	{
		beep(ebx, ecx, edx);
	}
	else if (eax == 0x0f)
	{
		int i, mx1 = mx, my1 = my, bufx = mx * 8, bufy = my * 16;
		int bx = mx1;
		int by = my1;
		int bmp = *(char *)(0xb8000 + by * 160 + bx * 2 + 1);
		mouse_ready(&mdec);
		for (;;)
		{
			// bmp = *(char *)(0xb8000 + my1 * 160 + mx1 * 2+1);

			if (fifo8_status(TaskGetMousefifo(task)) == 0)
			{
				io_stihlt();
			}
			else
			{

				i = fifo8_get(TaskGetMousefifo(task));
				io_sti();
				if (mouse_decode(&mdec, i) != 0)
				{
					if ((mdec.btn & 0x01) != 0)
					{
						intreturn(eax, ebx, mx, my, 1, edi, ebp);
						break;
					}
					else if ((mdec.btn & 0x02) != 0)
					{
						intreturn(eax, ebx, mx, my, 2, edi, ebp);
						break;
					}
					else if ((mdec.btn & 0x04) != 0)
					{
						intreturn(eax, ebx, mx, my, 3, edi, ebp);
						break;
					}
					mx1 = mx;
					my1 = my;
					bufx += mdec.x;
					bufy += mdec.y;
					//�ı�ģʽÿ���ַ��ǣ�8 * 16��������

					mx = bufx / 8;
					my = bufy / 16;

					if (bufx > 79 * 8)
					{
						bufx = 79 * 8;
					}
					else if (bufx < 0)
					{
						bufx = 0;
					}
					if (bufy > 24 * 16)
					{
						bufy = 24 * 16;
					}
					else if (bufy < 0)
					{
						bufy = 0;
					}
					*(char *)(0xb8000 + my1 * 160 + mx1 * 2 + 1) = bmp;
					bmp = *(char *)(0xb8000 + my * 160 + mx * 2 + 1);
					*(char *)(0xb8000 + my * 160 + mx * 2 + 1) = 0x70;
				}
			}
		}
		mouse_sleep(&mdec);
		*(char *)(0xb8000 + my * 160 + mx * 2 + 1) = bmp;
		mx = mx1;
		my = my1;
	}
	else if (eax == 0x10)
	{
		int gmx = 152, gmy = 92;
		int i;
		int x, y;
		mouse_ready(&mdec);
		for (;;)
		{
			for (y = 0; y < 16; y++)
			{
				for (x = 0; x < 16; x++)
				{
					if (mouse_cur_graphic[y][x] == '*')
					{
						*(char *)(0xa0000 + (gmy + y) * 320 + (gmx + x)) = 0;
					}
					else if (mouse_cur_graphic[y][x] == 'O')
					{
						*(char *)(0xa0000 + (gmy + y) * 320 + (gmx + x)) = 7;
					}
				}
			}
			if (fifo8_status(TaskGetMousefifo(task)) == 0)
			{
				io_stihlt();
			}
			else
			{
				i = fifo8_get(TaskGetMousefifo(task));
				io_sti();
				if (mouse_decode(&mdec, i) != 0)
				{
					for (y = 0; y < 16; y++)
					{
						for (x = 0; x < 16; x++)
						{
							*(char *)(0xa0000 + (gmy + y) * 320 + (gmx + x)) = 14;
						}
					}
					if ((mdec.btn & 0x01) != 0)
					{
						intreturn(eax, ebx, gmx, gmy, 1, edi, ebp);
						break;
					}
					else if ((mdec.btn & 0x02) != 0)
					{
						intreturn(eax, ebx, gmx, gmy, 2, edi, ebp);
						break;
					}
					else if ((mdec.btn & 0x04) != 0)
					{
						intreturn(eax, ebx, gmx, gmy, 3, edi, ebp);
						break;
					}
					gmx += mdec.x;
					gmy += mdec.y;
					if (gmx > 310)
					{
						gmx = 310;
					}
					else if (gmx < 0)
					{
						gmx = 0;
					}
					if (gmy > 184)
					{
						gmy = 184;
					}
					else if (gmy < 0)
					{
						gmy = 0;
					}
				}
			}
		}
		mouse_sleep(&mdec);
	}
	else if (eax == 0x16)
	{
		if (ebx == 0x01)
		{
			intreturn(eax, ebx, ecx, getch(), esi, edi, ebp);
		}
		else if (ebx == 0x02)
		{
			intreturn(eax, ebx, ecx, input_char_inSM(), esi, edi, ebp);
		}
		else if (ebx == 0x03)
		{
			input((char *)(edx + ds_base), ecx);
		}
	}
	else if (eax == 0x17)
	{
		if (ebx == 0x01)
		{
			drivers_idehdd_read(ecx, edx, (unsigned short *)esi + ds_base);
		}
		else if (ebx == 0x02)
		{
			drivers_idehdd_write(ecx, edx, (unsigned short *)esi + ds_base);
		}
	}
	else if (eax == 0x18)
	{
		bmpview((char *)(edx + ds_base));
	}
	else if (eax == 0x19)
	{
		command_run((char *)(edx + ds_base));
	}
	else if (eax == 0x1a)
	{
		if (ebx == 0x01)
		{
			struct FILEINFO *finfo;
			finfo = Get_File_Address((char *)(ds_base + edx));
			if (finfo != 0)
			{
				intreturn(eax, ebx, ecx, finfo->size, esi, edi, ebp);
			}
			else
			{
				intreturn(eax, ebx, ecx, -1, esi, edi, ebp);
			}
			// intreturn(eax, ebx, ecx, finfo->size, esi, edi, ebp);
		}
		else if (ebx == 0x02)
		{
			struct FILEINFO *finfo;
			finfo = Get_File_Address((char *)(ds_base + edx));
			char *p = fopen((char *)(ds_base + edx));
			char *q = (char *)ds_base + esi;
			int i;
			if (p != 0)
			{
				for (i = 0; i != finfo->size; i++)
				{
					q[i] = p[i];
				}
				intreturn(1, ebx, ecx, edx, esi, edi, ebp);
			}
			else
			{
				intreturn(0, ebx, ecx, edx, esi, edi, ebp);
			}
		}
		else if (ebx == 0x03)
		{
			char *FilePath = (char *)(ds_base + edx);
			mkfile(FilePath);
		}
		else if (ebx == 0x04)
		{
			char *FilePath = (char *)(ds_base + edx);
			mkdir(FilePath);
		}
		else if (ebx == 0x05)
		{
			char *FilePath = (char *)(ds_base + edx);
			char *Ptr = (char *)ds_base + esi;
			int length = ecx;
			EDIT_FILE(FilePath, Ptr, length);
			
		}
	}
	else if (eax == 0x1b)
	{
		int i;
		char *bes = (char *)(edx + ds_base);
		for (i = 0; i < strlen(line); i++)
		{
			bes[i] = line[i];
		}
		bes[i] = 0;
	}
	else if (eax == 0x1c)
	{
		Copy((char *)(edx + ds_base), (char *)(esi + ds_base));
	}
	else if (eax == 0x1d)
	{
		intreturn(kbhit(), ebx, ecx, edx, esi, edi, ebp);
	}
	else if (eax == 0x1e)
	{
		SleepTask(task);
		task->running = 0;
	}
	else if (eax == 0x1f)
	{
		//软盘驱动API
		/**
		 * 由于读取软盘有bug，所以暂时不提供
		 */
		if (ebx == 0x01)
		{
			write_floppy_for_ths(edx, esi, edi, ebp, ecx);
		}
	}
	else if (eax == 0x20)
	{
		// VBE驱动API
		if (ebx == 0x01)
		{
			intreturn(SwitchVBEMode(ecx), ebx, ecx, edx, esi, edi, ebp);
		}
		else if (ebx == 0x02)
		{
			intreturn(check_vbe_mode(ecx, (struct VBEINFO *)VBEINFO_ADDRESS), ebx, ecx, edx, esi, edi, ebp);
		}else if(ebx == 0x03) //切换到高分辨率文本显示模式
		{
			SwitchToHighTextMode();
		}
		else if(ebx == 0x1e)
		{
			//打印字符串在高分辨率文本模式下
			HighPrint((char *)(edx + ds_base));
		}
		else if(ebx == 0x04)
		{
			//从高分辨率文本模式返回到正常的文本模式
			Close_High_Text_Mode();
			SwitchToText8025_BIOS();
			Set_Font("A:\\other\\font.bin");
			clear();
		}
	}
	else if (eax == 0x21)
	{
		if (ebx == 0x01) {
			SwitchToText8025_BIOS();
		} else if (ebx == 0x02) {
			SwitchTo320X200X256_BIOS();
			Set_Font("A:\\other\\font.bin");
		}
	}
	else if (eax == 0x22)
	{
		//任务API
		if (ebx == 0x01) {
			Maskirq(0);
			int bp = 0;
			for (; bp != 32;) {
				if (buf[bp] == 0 && s[bp] == 0 && keyfifo[bp] == 0 && mousefifo[bp] == 0)
					break;
				else
					bp++;
			}
			if (bp == 31) {
				intreturn(eax, ebx, -1, edx, esi, edi, ebp);
				io_sti();
				return;
			}
			s[bp] = page_vmalloc(32 * 1024) + 32 * 1024;
			buf[bp] = (int)AddTask((char *)(edx + ds_base), 3, 3 * 8, ecx+27, 4 * 8, 1 * 8, s[bp]);
			keybuf[bp] = page_kmalloc(32);
			mousebuf[bp] = page_kmalloc(128);
			keyfifo[bp] = page_kmalloc(sizeof(struct FIFO8));
			mousefifo[bp] = page_kmalloc(sizeof(struct FIFO8));
			fifo8_init((struct FIFO8 *)keyfifo[bp],32,keybuf[bp]);
			fifo8_init((struct FIFO8 *)mousefifo[bp],128,mousebuf[bp]);
			TaskSetFIFO((struct TASK *)buf[bp],
			(struct FIFO8 *)keyfifo[bp],(struct FIFO8 *)mousefifo[bp]);
			intreturn(eax, ebx, bp, edx, esi, edi, ebp);	// 返回子进程ID号
			ClearMaskIrq(0);
		} else if (ebx == 0x02) {
			Maskirq(0);
			if (ecx > 31) {
				return;
			}
			struct TASK *ttask = (struct TASK *)buf[ecx];
			page_kfree(buf[ecx],sizeof(struct TASK));
			page_vfree(s[ecx]-32*1024,32*1024);
			page_kfree(keybuf[ecx],32);
			page_kfree(mousebuf[ecx],128);
			page_kfree(keyfifo[ecx],sizeof(struct FIFO8));
			page_kfree(mousefifo[ecx],sizeof(struct FIFO8));
			buf[ecx] = 0;
			s[ecx] = 0;
			keyfifo[ecx] = 0;
			mousefifo[ecx] = 0;
			ClearMaskIrq(0);
			SleepTask(ttask);
			ttask->running = 0;
		//	while(GetTaskForName(ttask->name)!=0);	
		}
	}
	return;
}
