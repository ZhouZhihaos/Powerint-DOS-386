#include "system.h"
// Copyright (C) zhouzhihao 2022
// 本人自创的PAK文件格式

// PAK：多文件打包成一个文件
// 0x0000 ~ 0x0004
// 3字节"PAK"，2字节文件数量
// 0x0005 ~ 0x000F 保留
// 0x0010 ~ 0x0010+C*16（C=打包文件个数，每个文件信息头大小为16bytes）
// 每个文件信息头结构：
//   0~7 文件名
//   8~11 后缀
//   12~13 文件在PAK文件内的偏移
//   14~15 文件长度
void pak(char *pakname, char *filenames)
{
	// 打包
	// pakname：要打包成的PAK文件名
	// filenames：要打包的文件名s
	char filename[16] = {0};
	int i;
	for (i = 0; pakname[i] != ' '; i++)
	{
		filename[i] = pakname[i];
	}
	mkfile(filename);	// 新建PAK
	// 再打开它
	struct FILEINFO *pakfinfo = Get_File_Address(filename);
	struct FILEINFO *finfo;
	char *pak = fopen(filename);
	char *file;
	int filenum;
	int len = 0;
	int point;
	// filenames：filename1 filename2 filename3 ...
	// 这里就是记数有多少个文件（空格+1） 和argc记数一样
	for (i = 0, filenum = 0; i < strlen(filenames); filenum++)
	{
		for (; filenames[i] != ' '; i++)
		{
			if (i > strlen(filenames))
			{
				break;
			}
		}
		i++;
	}
	// 信息头PAK
	pak[0] = 'P';
	pak[1] = 'A';
	pak[2] = 'K';
	*(short *)&pak[3] = (short)filenum;
	pakfinfo->size += filenum * 16 + 16;	// + 16是因为信息头
	// 分割filenames 并且顺序写入PAK文件信息头
	// len = filenames的底指数
	for (i = 0; i != filenum; i++)
	{
		clean(filename, 16);
		// 也是类似argc的记数方法 遇到' '或者0就跳出（分割结束
		for (point = 0; filenames[point + len] != ' '; point++)
		{
			if (filenames[point + len] == 0)
				goto out;
			// 不是空格或者0 把这个字节传送到filename[point]中
			filename[point] = filenames[point + len];
		}
	out:
		len += point + 1;	// 记录len
		filename[point] = 0;	// 防止有脏数据
		// 打开这个文件
		finfo = Get_File_Address(filename);
		file = fopen(filename);
		if (finfo == 0)
		{
			print("File not find!\n\n");
			break;
		}
		// 下面就是按照PAK文件格式给PAK文件信息头赋值了
		int j;
		for (j = 0; j != 12; j++)
		{
			pak[i * 16 + j + 16] = finfo->name[j];
		}
		pak[i * 16 + 12 + 16] = 0x20;
		*(short *)&pak[i * 16 + 12 + 16] = (short)pakfinfo->size;	// 在PAK文件的偏移地址
		*(short *)&pak[i * 16 + 14 + 16] = (short)finfo->size;
		for (j = 0; j != finfo->size; j++)
		{
			pak[j + pakfinfo->size] = file[j];
		}
		pakfinfo->size += finfo->size;
	}
	return;
}
void unpak(char *pakname)
{
	// 解包
	// 和打包步骤相反 自己理解下吧
	struct FILEINFO *finfo;
	char *file;
	char *pak = fopen(pakname);
	if (pak == 0)
	{
		print("File not find!\n\n");
		return;
	}
	if (pak[0] != 'P' || pak[1] != 'A' || pak[2] != 'K')
	{
		print("Is't Powerint DOS 386 PAK file!\n\n");
		return;
	}
	int i;
	char filename[16];
	short filenum = pak[3];
	for (i = 0; i != filenum; i++)
	{
		clean(filename, 16);
		int j, flag = 0, point = 0;
		for (j = 0; j != 12; j++)
		{
			if (pak[16 + i * 16 + j] == ' ')
			{
				if (flag == 0)
				{
					filename[point] = '.';
					point++;
					flag = 1;
				}
				continue;
			}
			filename[point] = pak[16 + i * 16 + j];
			point++;
		}
		filename[0] = 'U';	// 避免文件名重复 解包成的文件文件名首字母改成U
		mkfile(filename);
		finfo = Get_File_Address(filename);
		file = fopen(filename);
		unsigned short addr = *(short *)&pak[16 + i * 16 + 12];
		unsigned short length = *(short *)&pak[16 + i * 16 + 14];
		finfo->size = length;
		for (j = 0; j != length; j++)
		{
			file[j] = pak[addr + j];
		}
	}
	return;
}