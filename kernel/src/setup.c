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

	print("RNM Give you money\n");
	return;
	int i;
	char c;
	struct FILEINFO *finfo,*finfo2,*finfo3;
	char buf[512]={0};	// 写入缓冲扇区
	// 将信息写入缓冲区
	int tmp = dictaddr;
	dictaddr = 0x2600;	// 切换成根目录
	finfo = file_search("kernel.bin", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
	finfo2 = file_search("font.bin", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
	finfo3 = file_search("dosldr", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
	finfo3->clustno=2;
	memcpy(&buf,finfo3,32);
	finfo->clustno=1+finfo3->clustno+finfo3->size/512;
	memcpy(&buf[32],finfo,32);
	finfo2->clustno=1+finfo->clustno+finfo->size/512;
	memcpy(&buf[64],finfo2,32);
	//print("Sorry,this setup has a problem,please wait it has done.\n\n");
	//return;
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
			dictaddr = tmp;
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
			print("2.Write FILEINFO...");
			drivers_idehdd_write(19, 1, &buf);
			sleep(1);
			print("done.\n\n");
			// 写入Dosldr内容
			print("3.Write DOSLDR...");
			drivers_idehdd_write(31+finfo3->clustno,((finfo3->size/512)+1),fopen("dosldr"));
			sleep(1);
			print("done.\n\n");
			// 写入Kernel.bin内容
			print("3.Write KERNEL.BIN...");
			drivers_idehdd_write(31+finfo->clustno,((finfo->size/512)+1),fopen("kernel.bin"));
			sleep(1);
			print("done.\n\n");
			// 写入Font.bin内容
			print("4.Write FONT.BIN...");
			drivers_idehdd_write(31+finfo2->clustno,((finfo2->size/512)+1),fopen("font.bin"));
			sleep(1);
			print("done.\n\n");
			dictaddr = tmp;
			return;
		}
	}
}
