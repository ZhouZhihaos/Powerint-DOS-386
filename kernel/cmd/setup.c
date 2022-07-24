#include <dos.h>
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
	print("RNM Give me money!\n");
	return;
	int i;
	char c;
	struct FILEINFO *finfo,*finfo2,*finfo3;
	char *buf = (char *)page_malloc_one();	// 写入缓冲扇区
	// 将信息写入缓冲区
	finfo = Get_File_Address("A:\kernel.bin");
	finfo2 = Get_File_Address("A:\other\font.bin");
	finfo3 = Get_File_Address("A:\dosldr");
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
			return;
		}
		if (c == '1')
		{
			Maskirq(0);
			// 写入引导扇区
			print("1.Write Boot Sector...");
			drivers_idehdd_write(0, 1, fopen("A:\hdboot.bin"));
			print("done.\n\n");
			// 写入文件信息
			print("2.Write FILEINFO...");
			drivers_idehdd_write(19, 1, buf);
			print("done.\n\n");
			// 写入Dosldr内容
			print("3.Write DOSLDR...");
			for (int i = 0; i < (finfo3->size/512)+1; i++)
				drivers_idehdd_write(31+finfo3->clustno+i,1,fopen("A:\dosldr")+i*512);
			print("done.\n\n");
			// 写入Kernel.bin内容
			print("4.Write KERNEL.BIN...");
			for (int i = 0; i < (finfo->size/512)+1; i++)
				drivers_idehdd_write(31+finfo->clustno+i,1,fopen("A:\kernel.bin")+i*512);
			print("done.\n\n");
			// 写入Font.bin内容
			print("5.Write FONT.BIN...");
			for (int i = 0; i < (finfo2->size/512)+1; i++)
				drivers_idehdd_write(31+finfo2->clustno+i,1,fopen("A:\other\font.bin")+i*512);
			print("done.\n\n");
			ClearMaskIrq(0);
			return;
		}
	}
}
