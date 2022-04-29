#include "system.h"

char *fopen(char *name)
{
	struct FILEINFO *finfo;
	finfo = Get_File_Address(name);
	if (finfo == 0)
	{
		return 0;
	}
	else
	{
		char *p = (char *)(finfo->clustno * 512 + 0x003e00 + ADR_DISKIMG);
		return p;
	}
}
FILE *fopen_for_FILE(char *path,char *mode)
{
	struct FILEINFO *finfo;
	finfo = Get_File_Address(path);
	if (finfo == 0)
	{
		return 0;
	}
	else
	{
		FILE *fp = (FILE *)page_kmalloc(sizeof(FILE));
		fp->buf = (char *)(finfo->clustno * 512 + 0x003e00 + ADR_DISKIMG);
		fp->size = finfo->size;
		fp->p = 0; //指向文件的开头
	}
}
//fgetc
int fgetc(FILE *fp)
{
	if (fp->p >= fp->size)
	{
		return EOF;
	}
	else
	{
		return fp->buf[fp->p++];
	}
}
//fgets
char *fgets(char *s, int size, FILE *fp)
{
	int i;
	for (i = 0; i < size - 1; i++)
	{
		int c = fgetc(fp);
		if (c == EOF)
		{
			if (i == 0)
			{
				return 0;
			}
			else
			{
				break;
			}
		}
		s[i] = c;
		if (c == '\n')
		{
			break;
		}
	}
	s[i] = '\0';
	return s;
}
//fseek
int fseek(FILE *fp, int offset, int whence)
{
	if (whence == 0)
	{
		fp->p = offset;
	}
	else if (whence == 1)
	{
		fp->p += offset;
	}
	else if (whence == 2)
	{
		fp->p = fp->size + offset;
	}
	else
	{
		return -1;
	}
	return 0;
}
int fclose(FILE *fp)
{
	page_kfree((int)fp, sizeof(FILE));
	return 0;
}
//fread
int fread(void *buf, int size, int count, FILE *fp)
{
	int i;
	for (i = 0; i < size * count; i++)
	{
		if (fp->p >= fp->size)
		{
			break;
		}
		((char *)buf)[i] = fp->buf[fp->p++];
	}
	return i;
}
void EDIT_FILE(char *name, char *dest, int length)
{
	struct FILEINFO *finfo;
	finfo = Get_File_Address(name);
	if (finfo == 0)
	{
		//没有找到文件，创建一个，然后再编辑
		mkfile(name);
		EDIT_FILE(name, dest, length);
		return;
	}
	finfo->size = 0;
	char *p = fopen(name);
	for (; finfo->size < length; finfo->size++)
	{
		p[finfo->size] = *dest++;
	}
	int size = finfo->size;
	int Bmp = dictaddr;
	char BMP_PATH[224 * 13 + 1];
	for (int i = 0; i < 224 * 13; i++)
	{
		BMP_PATH[i] = 0;
	}
	strcpy(BMP_PATH, path);
	while (dictaddr != 0x2600)
	{
		if (dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224) != 0)
		{
			struct FILEINFO *finfo_this_dict_clust = dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
			struct FILEINFO *finfo_this_dict = clust_sech(finfo_this_dict_clust->clustno, (struct FILEINFO *)(ADR_DISKIMG + Get_dictaddr("../")), 224);
			finfo_this_dict->size += size;
		}
		changedict("..");
	}
	dictaddr = Bmp;
	strcpy(path, BMP_PATH); //还原PATH
	return;
}