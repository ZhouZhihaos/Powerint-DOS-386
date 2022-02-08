#include "system.h"
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

char *fopen(char *name)
{
    struct FILEINFO *finfo;
    finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
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