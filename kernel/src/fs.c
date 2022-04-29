//fs.c : 文件系统的实现
//目前实现了：fat12文件系统
#include "system.h"
extern int change_dict_times;
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++)
	{
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++)
	{
		if (j >= 11)
		{
			return 0; /*没有找到*/
		}
		if (name[i] == '.' && j <= 8)
		{
			j = 8;
		}
		else
		{
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z')
			{
				/*将小写字母转换为大写字母*/
				s[j] -= 0x20;
			}
			j++;
		}
	}
	for (i = 0; i < max;)
	{
		if (finfo[i].name[0] == 0x00)
		{
			break;
		}
		if ((finfo[i].type & 0x18) == 0)
		{
			for (j = 0; j < 11; j++)
			{
				if (finfo[i].name[j] != s[j])
				{
					goto next;
				}
			}
			return finfo + i; /*找到文件*/
		}
	next:
		i++;
	}
	return 0; /*没有找到*/
}
struct FILEINFO *dict_search(char *name, struct FILEINFO *finfo, int max)
{
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++)
	{
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++)
	{
		if (j >= 11)
		{
			return 0; /*没有找到*/
		}
		else
		{
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z')
			{
				/*将小写字母转换为大写字母*/
				s[j] -= 0x20;
			}
			j++;
		}
	}
	for (i = 0; i < max;)
	{
		if (finfo[i].name[0] == 0x00)
		{
			break;
		}
		if (finfo[i].type == 0x10)
		{
			for (j = 0; j < 11; j++)
			{
				if (finfo[i].name[j] != s[j])
				{
					goto next;
				}
			}
			return finfo + i; /*找到文件*/
		}
	next:
		i++;
	}
	return 0; /*没有找到*/
}
struct FILEINFO *Get_File_Address(char *path1)
{
	int bmpDict = dictaddr;
	char *path = (char *)page_kmalloc(strlen(path1) + 1);
	strcpy(path, path1);
	strtoupper(path);
	if (strncmp("A:\\", path, 3) == 0 || strncmp("A:/", path, 3) == 0)
	{
		path += 3;
		bmpDict = 0x2600;
	}
	if (path[0] == '\\' || path[0] == '/')
	{
		//跳过反斜杠和正斜杠
		for (int i = 0; i < strlen(path); i++)
		{
			if (path[i] != '\\' && path[i] != '/')
			{
				path += i;
				break;
			}
		}
	}
	char *temp_name = (char *)page_kmalloc(128);
	struct FILEINFO *finfo;
	int i = 0;
	while (1)
	{
		int j;
		for (j = 0; i < strlen(path); i++, j++)
		{
			if (path[i] == '\\' || path[i] == '/')
			{
				temp_name[j] = '\0';
				// printf("Got It:%s,ALL:%s\n", temp_name, path);
				i++;
				break;
			}
			temp_name[j] = path[i];
		}

		finfo = dict_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict), 224);
		if (finfo == 0)
		{

			if (path[i] != '\0')
			{
				page_kfree((int)temp_name, 128);
				page_kfree((int)path, strlen(path1) + 1);
				return 0;
			}
			finfo = file_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict), 224);
			if (finfo == 0)
			{

				// printf("Invalid file:%s\n", temp_name);
				page_kfree((int)temp_name, 128);
				page_kfree((int)path, strlen(path1) + 1);
				return 0;
			}
			else
			{
				goto END;
			}
		}
		else
		{
			// printf("dict_search:%s\n", temp_name);
			if (finfo->clustno != 0)
			{
				bmpDict = (finfo->clustno * 512 + 0x003e00);
			}else{
				//print("finfo->clustno == 0\n");
				bmpDict = 0x002600;
			}
			clean(temp_name, 128);
		}
	}
END:
	page_kfree((int)temp_name, 128);
	page_kfree((int)path, strlen(path1) + 1);
	return finfo;
}
int Get_dictaddr(char *path1)
{
	int bmpDict = dictaddr;
	char *path = (char *)page_kmalloc(strlen(path1) + 1);
	strcpy(path, path1);
	strtoupper(path);
	if (strncmp("A:\\", path, 3) == 0 || strncmp("A:/", path, 3) == 0)
	{
		path += 3;
		bmpDict = 0x2600;
	}
	if (path[0] == '\\' || path[0] == '/')
	{
		//跳过反斜杠和正斜杠
		for (int i = 0; i < strlen(path); i++)
		{
			if (path[i] != '\\' && path[i] != '/')
			{
				path += i;
				break;
			}
		}
	}
	char *temp_name = (char *)page_kmalloc(128);
	struct FILEINFO *finfo;
	int i = 0;
	while (1)
	{
		int j;
		for (j = 0; i < strlen(path); i++, j++)
		{
			if (path[i] == '\\' || path[i] == '/')
			{
				temp_name[j] = '\0';
				// printf("Got It:%s,ALL:%s\n", temp_name, path);
				i++;
				break;
			}
			temp_name[j] = path[i];
		}

		finfo = dict_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict), 224);
		if (finfo == 0)
		{
			goto END;
		}
		else
		{
			// printf("dict_search:%s\n", temp_name);
			if (finfo->clustno != 0) {
				bmpDict = (finfo->clustno * 512 + 0x003e00);
			} else {
				bmpDict = 0x002600;
			}
			clean(temp_name, 128);
			if (path[i] == '\0') {
				goto END;
			}
		}
	}
END:
	page_kfree((int)temp_name, 128);
	page_kfree((int)path, strlen(path1) + 1);
	return bmpDict;
}
void mkdir(char *dictname, int last_clust)
{
	/*
		dictname:目录名
		last_clust:上一级目录的簇号
	*/
	mkfile(dictname);
	struct FILEINFO *finfo = Get_File_Address(dictname);
	char *p = fopen(dictname);
	finfo->type = 0x10; // 是目录（文件夹的type属性是0x10）
	// 三个目录项（模板）
	struct FILEINFO dictmodel1; // .目录项，指向自己
	struct FILEINFO dictmodel2; // ..目录项，指向上一级目录
	struct FILEINFO null;		//空目录项（为mkfile函数提供指引）
	memcpy(null.name, "NULL       ", 11);
	null.type = 0x20; // 文件的type属性是0x20
	//将size date time这类属性全部设置为0
	null.size = 0;
	null.date = 0;
	null.time = 0;
	null.clustno = finfo->clustno + 2; // 初始化为第二簇
	dictmodel1.name[0] = '.';
	for (int i = 1; i != 8; i++)
		dictmodel1.name[i] = ' ';
	for (int i = 0; i != 3; i++)
		dictmodel1.ext[i] = ' ';
	dictmodel1.type = 0x10;
	dictmodel1.clustno = finfo->clustno; // 指向自己
	dictmodel1.size = 0;
	dictmodel1.date = 0;
	dictmodel1.time = 0;
	dictmodel2.name[0] = '.';
	dictmodel2.name[1] = '.';
	for (int i = 2; i != 8; i++)
		dictmodel2.name[i] = ' ';
	for (int i = 0; i != 3; i++)
		dictmodel2.ext[i] = ' ';
	dictmodel2.clustno = last_clust;

	dictmodel2.size = 0;
	dictmodel2.date = 0;
	dictmodel2.time = 0;
	dictmodel2.type = 0x10;
	char q[] = "\0";
	memcpy(p, &dictmodel1, 32);
	memcpy(&p[32], &dictmodel2, 32);
	memcpy(&p[64], &null, 32);
	memcpy(&p[96], &q, 1);
	return;
}
void Copy(char *path, char *path1)
{
	mkfile(path1); //创建文件
	struct FILEINFO *finfo = Get_File_Address(path1);
	struct FILEINFO *finfo1 = Get_File_Address(path);
	if (finfo == 0 || finfo1 == 0)
	{
		return;
	}
	char *p = fopen(path);
	char *p1 = fopen(path1);
	// 拷贝文件内容
	memcpy(p1, p, finfo1->size);
	// 拷贝文件属性
	finfo->size = finfo1->size;
	finfo->date = finfo1->date;
	finfo->time = finfo1->time;
	finfo->type = finfo1->type;
	return;
}
struct FILEINFO *clust_sech(int clustno, struct FILEINFO *finfo, int max)
{
	//通过簇号找到文件信息
	int i, j;
	j = 0;
	for (i = 0; i < max; i++)
	{
		if (finfo[i].clustno == clustno)
		{
			return finfo + i;
		}
	}
	return 0; /*没找到*/
}
void del(char *cmdline)
{
	//删除某个文件
	char *name;
	int i;
	struct FILEINFO *finfo;
	for (i = 0; i < strlen(cmdline); i++)
	{
		name[i] = cmdline[i + 4];
	}
	finfo = Get_File_Address(name);
	if (finfo == 0)
	{
		print(name);
		print(" not found!\n\n");
		return;
	}
	char *ptr = fopen(name);
	memset(ptr, 0, finfo->size);
	finfo->name[0] = 0xe5;

	// sleep(1);
	return;
}
void mkfile(char *name)
{
	char s[12];
	int i, j;
	struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + Get_dictaddr(name));

	for (i = strlen(name); i >= 0; i--)
	{
		if (name[i] == '/' || name[i] == '\\')
		{
			name += i + 1;
			break;
		}
	}

	for (j = 0; j != 12; j++)
	{
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++)
	{
		if (j >= 11)
		{
			return;
		}
		if (name[i] == '.' && j <= 8)
		{
			j = 8;
		}
		else
		{
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z')
			{
				s[j] -= 0x20;
			}
			j++;
		}
	}
	for (i = 0;; i++)
	{
		if (finfo[i].name[0] == 0x00)
		{
			finfo = finfo + i;
			break;
		}
	}
	for (i = 0; i != 8; i++)
	{
		finfo->name[i] = s[i];
	}
	for (i = 8; i != 11; i++)
	{
		finfo->ext[i - 8] = s[i];
	}
	finfo->type = 0x20;
	finfo->clustno = finfo[-1].clustno + (finfo[-1].size / 512) + 1;
	for (i = 0; i != 10; i++)
	{
		finfo->reserve[i] = 0;
	}
	finfo->time = 0;
	finfo->date = 0;
	finfo->size = 0;
	finfo[1].name[0] = 0x00; //将下一个项设置为0x00
	sleep(1);
	return;
}
void changedict(char *dictname)
{
	// cd命令的依赖函数
	struct FILEINFO *finfo = dict_search(dictname, (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
	//找文件夹
	if (finfo == 0)
	{
		//没找到
		print("Invalid directory.\n");
		return;
	}
	if (finfo->clustno == 0)
	{
		//根目录
		strcpy(path, "");
		change_dict_times = 0;
		dictaddr = 0x002600; //根目录地址（0x2600）
		return;
	}
	//..进行特殊的处理
	//.不进行处理
	//其他按照下面的方式处理
	if (strcmp(dictname, "..") != 0 && strcmp(dictname, ".") != 0)
	{
		if (change_dict_times == 0)
		{
			strcat(path, dictname);
		}
		else
		{
			strcat(path, "\\");
			strcat(path, dictname);
		}
	}

	if (strcmp(dictname, "..") == 0)
	{
		int i;
		// print("OK\n");
		for (i = strlen(path) - 1; i >= 0; i--)
		{
			if (path[i] == '\\')
			{
				path[i] = '\0';
				break;
			}
		}
		change_dict_times -= 2; //因为下面又++了,所以这里要减2
	}
	dictaddr = (finfo->clustno * 512 + 0x003e00);
	change_dict_times++;
	return;
}