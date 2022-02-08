#include "system.h"
#include <stdio.h>
void lba2chs(int lba, int cyline, int header, int sector)
{
	cyline = lba / 36;
	header = (lba / 18) % 2;
	sector = (lba % 18) + 1;
	return;
}
int chs2lba(int cyline, int header, int sector)
{
	return ((cyline * 2 + header) * 18 + sector - 1);
}
void setup(void)
{
	int i;
	char c;
	struct FILEINFO *finfo,*finfo2;
	char buf[512]={0};	// 写入缓冲扇区
	// Kernel.bin信息定义
	finfo = file_search("kernel.bin", (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	finfo2 = file_search("font.bin", (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	memcpy(&buf,finfo,32);	// 将Kernel.bin信息和font.bin信息写入缓冲区
	finfo2->clustno=3+finfo->size/512;
	memcpy(&buf[32],finfo2,32);
	print("Sorry,this setup has a problem,please wait it has done.\n\n");
	return;
	print("Welcome install Powerint DOS 386!\n");
	print("Install options:\n");
	print("1.Install All(default)\n");
	print("e|E.Exit setup\n");
	print("Press Input number of options:");
	while (1)
	{
		c = getch();
		if (c == 'e' || c == 'E')
		{
			print("\n");
			return;
		}
		if (c == '1')
		{
			print("\n\nWriting hard disk,please wait...\n");
			// 写入引导扇区
			print("1.Write Boot Sector...");
			drivers_idehdd_write(0, 1, fopen("hdboot.bin"));
			sleep(1);
			print("done.\n\n");
			// 写入文件信息
			print("2.Write KERNEL.BIN & FONT.BIN info...");
			drivers_idehdd_write(19, 1, &buf);
			sleep(1);
			print("done.\n\n");
			// 写入Kernel.bin内容
			print("3.Write KERNEL.BIN...");
			drivers_idehdd_write(33,((finfo->size/512)+1),fopen("kernel.bin"));
			sleep(1);
			print("done.\n\n");
			// 写入Font.bin内容
			print("4.Write FONT.BIN...");
			drivers_idehdd_write(34+finfo->size/512,((finfo2->size/512)+1),fopen("font.bin"));
			sleep(1);
			print("done.\n\n");
			return;
		}
	}
}
