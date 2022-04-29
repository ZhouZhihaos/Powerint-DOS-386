#include "system.h"
extern char cons_color;
char *_Asm(const char *asm_code1,int addr)
{
    if (asm_code1[0] == ';')
    {
        return (char *)"\0";
    }
    //处理code
    char *asm_code = (char *)page_kmalloc(1000);
	int i;
	for (i = 0; i != 1000; i++) {
		asm_code[i] = 0;
	}
    //setup 1.去掉空格
    int t, q, kl, len;
    for (t = 0; t < strlen(asm_code1); t++)
    {
        if (asm_code1[t] != ' ' && asm_code1[t] != '\t')
        {
            break;
        }
    }
    if (asm_code1[t] == ';')
    {
		page_kfree(asm_code,1000);
        return (char *)"\1";
    }
    kl = t;
    for (len = 0; len < strlen(asm_code1); len++)
    {
        if (asm_code1[len] == ';')
        {
            break;
        }
    }
    for (t = 0; kl < len; t++, kl++)
    {
        asm_code[t] = asm_code1[kl];
    }

    char *Result = (char *)page_kmalloc(strlen(asm_code) + 1);
    if (strncmp(asm_code, "mov ", 4) == 0)
    {
        if (asm_code[4] == '$') //立即数
        {
            char temp[30] = {0};
            int i = 0;
            for (i = 5; i < strlen(asm_code) - 4; i++)
            {
                temp[i - 5] = asm_code[i];
            }
            temp[i - 5] = '\0';
            if (strlen(temp) == 2) //这是一个byte
            {
                //判断寄存器
                char temp2[30] = {0};
                int j = 0;
                for (j = 5 + strlen(temp); j < strlen(asm_code); j++)
                {
                    temp2[j - 5 - strlen(temp)] = asm_code[j];
                }
                temp2[j - 5 - strlen(temp)] = '\0';
                if (strcmp(temp2, ",eax") == 0)
                {
                    sprintf(Result, "b8 %s 00 00 00", temp);
                }
                if (strcmp(temp2, ",ebx") == 0)
                {
                    sprintf(Result, "bb %s 00 00 00", temp);
                }
                if (strcmp(temp2, ",ecx") == 0)
                {
                    sprintf(Result, "b9 %s 00 00 00", temp);
                }
                if (strcmp(temp2, ",edx") == 0)
                {
                    sprintf(Result, "ba %s 00 00 00", temp);
                }
                if (strcmp(temp2, ",esi") == 0)
                {
                    sprintf(Result, "be %s 00 00 00", temp);
                }
                if (strcmp(temp2, ",edi") == 0)
                {
                    sprintf(Result, "bf %s 00 00 00", temp);
                }
            }
            else if (strlen(temp) == 1)
            {
                char bmp3[] = {'0', temp[0], '\0'};
                char temp2[30];
                int j = 0;
                for (j = 5 + strlen(bmp3); j < strlen(asm_code); j++)
                {
                    temp2[j - 5 - strlen(bmp3)] = asm_code[j];
                }
                temp2[j - 5 - strlen(bmp3)] = '\0';
                if (strcmp(temp2, "eax") == 0)
                {
                    sprintf(Result, "b8 %s 00 00 00", bmp3);
                }
                else if (strcmp(temp2, "ebx") == 0)
                {
                    sprintf(Result, "bb %s 00 00 00", bmp3);
                }
                else if (strcmp(temp2, "edx") == 0)
                {
                    sprintf(Result, "ba %s 00 00 00", bmp3);
                }
                else if (strcmp(temp2, "ecx") == 0)
                {
                    sprintf(Result, "b9 %s 00 00 00", bmp3);
                }
                else if (strcmp(temp2, "esi") == 0)
                {
                    sprintf(Result, "be %s 00 00 00", bmp3);
                }
                else if (strcmp(temp2, "edi") == 0)
                {
                    sprintf(Result, "bf %s 00 00 00", bmp3);
                }
            }
            else if (strlen(temp) == 4) //a word
            {
                char num1[] = {temp[0], temp[1], 0};
                char num2[] = {temp[2], temp[3], 0};
                char temp2[30] = {0};
                int j = 0;
                for (j = 5 + strlen(temp); j < strlen(asm_code); j++)
                {
                    temp2[j - 5 - strlen(temp)] = asm_code[j];
                }
                temp2[j - 5 - strlen(temp)] = '\0';
                if (strcmp(temp2, ",eax") == 0)
                {
                    sprintf(Result, "b8 %s %s 00 00", num2, num1);
                }
                else if (strcmp(temp2, ",ebx") == 0)
                {
                    sprintf(Result, "bb %s %s 00 00", num2, num1);
                }
                else if (strcmp(temp2, ",edx") == 0)
                {
                    sprintf(Result, "ba %s %s 00 00", num2, num1);
                }
                else if (strcmp(temp2, ",ecx") == 0)
                {
                    sprintf(Result, "b9 %s %s 00 00", num2, num1);
                }
                else if (strcmp(temp2, ",esi") == 0)
                {
                    sprintf(Result, "be %s %s 00 00", num2, num1);
                }
                else if (strcmp(temp2, ",edi") == 0)
                {
                    sprintf(Result, "bf %s %s 00 00", num2, num1);
                }
            }
            // double world
            else if (strlen(temp) == 8)
            {
                char num1[] = {temp[0], temp[1], 0};
                char num2[] = {temp[2], temp[3], 0};
                char num3[] = {temp[4], temp[5], 0};
                char num4[] = {temp[6], temp[7], 0};
                char temp2[30] = {0};
                int j = 0;
                for (j = 5 + strlen(temp); j < strlen(asm_code); j++)
                {
                    temp2[j - 5 - strlen(temp)] = asm_code[j];
                }
                temp2[j - 5 - strlen(temp)] = '\0';
                if (strcmp(temp2, ",eax") == 0)
                {
                    sprintf(Result, "b8 %s %s %s %s", num4, num3, num2, num1);
                }
                else if (strcmp(temp2, ",ebx") == 0)
                {
                    sprintf(Result, "bb %s %s %s %s", num4, num3, num2, num1);
                }
                else if (strcmp(temp2, ",edx") == 0)
                {
                    sprintf(Result, "ba %s %s %s %s", num4, num3, num2, num1);
                }
                else if (strcmp(temp2, ",ecx") == 0)
                {
                    sprintf(Result, "b9 %s %s %s %s", num4, num3, num2, num1);
                }
                else if (strcmp(temp2, ",esi") == 0)
                {
                    sprintf(Result, "be %s %s %s %s", num4, num3, num2, num1);
                }
                else if (strcmp(temp2, ",edi") == 0)
                {
                    sprintf(Result, "bf %s %s %s %s", num4, num3, num2, num1);
                }
            }
            else
            {
                printf("\nError!");
				page_kfree(Result,strlen(asm_code)+1);
				page_kfree(asm_code,1000);
                return 0;
            }
        }
        else if (asm_code[4] == '%') //寄存器
        {
            char temp[30] = {0};
            int i = 0;
            for (i = 5; i < strlen(asm_code) - 4; i++)
            {
                temp[i - 5] = asm_code[i];
            }
            temp[i - 5] = '\0';
            char temp2[30] = {0};
            int j = 0;
            for (j = 5 + strlen(temp); j < strlen(asm_code); j++)
            {
                temp2[j - 5 - strlen(temp)] = asm_code[j];
            }
            temp2[j - 5 - strlen(temp)] = '\0';

            if (strcmp("eax", temp) == 0 && strcmp(",eax", temp2) == 0)
            {
                sprintf(Result, "8b c0");
            }
            else if (strcmp("ebx", temp) == 0 && strcmp(",eax", temp2) == 0)
            {
                sprintf(Result, "8b c3");
            }
            else if (strcmp("ecx", temp) == 0 && strcmp(",eax", temp2) == 0)
            {
                sprintf(Result, "8b c1");
            }
            else if (strcmp("edx", temp) == 0 && strcmp(",eax", temp2) == 0)
            {
                sprintf(Result, "8b c2");
            }
            else if (strcmp("esi", temp) == 0 && strcmp(",eax", temp2) == 0)
            {
                sprintf(Result, "8b c6");
            }
            else if (strcmp("edi", temp) == 0 && strcmp(",eax", temp2) == 0)
            {
                sprintf(Result, "8b c7");
            }
            else if (strcmp("ebp", temp) == 0 && strcmp(",eax", temp2) == 0)
            {
                sprintf(Result, "8b c5");
            }
            //ebx
            if (strcmp("eax", temp) == 0 && strcmp(",ebx", temp2) == 0)
            {
                sprintf(Result, "8b d8");
            }
            else if (strcmp("ebx", temp) == 0 && strcmp(",ebx", temp2) == 0)
            {
                sprintf(Result, "8b db");
            }
            else if (strcmp("ecx", temp) == 0 && strcmp(",ebx", temp2) == 0)
            {
                sprintf(Result, "8b d9");
            }
            else if (strcmp("edx", temp) == 0 && strcmp(",ebx", temp2) == 0)
            {
                sprintf(Result, "8b da");
            }
            else if (strcmp("esi", temp) == 0 && strcmp(",ebx", temp2) == 0)
            {
                sprintf(Result, "8b de");
            }
            else if (strcmp("edi", temp) == 0 && strcmp(",ebx", temp2) == 0)
            {
                sprintf(Result, "8b df");
            }
            else if (strcmp("ebp", temp) == 0 && strcmp(",ebx", temp2) == 0)
            {
                sprintf(Result, "8b dd");
            }

            //ecx
            if (strcmp("eax", temp) == 0 && strcmp(",ecx", temp2) == 0)
            {
                sprintf(Result, "8b c8");
            }
            else if (strcmp("ebx", temp) == 0 && strcmp(",ecx", temp2) == 0)
            {
                sprintf(Result, "8b cb");
            }
            else if (strcmp("ecx", temp) == 0 && strcmp(",ecx", temp2) == 0)
            {
                sprintf(Result, "8b c9");
            }
            else if (strcmp("edx", temp) == 0 && strcmp(",ecx", temp2) == 0)
            {
                sprintf(Result, "8b ca");
            }
            else if (strcmp("esi", temp) == 0 && strcmp(",ecx", temp2) == 0)
            {
                sprintf(Result, "8b ce");
            }
            else if (strcmp("edi", temp) == 0 && strcmp(",ecx", temp2) == 0)
            {
                sprintf(Result, "8b cf");
            }
            else if (strcmp("ebp", temp) == 0 && strcmp(",ecx", temp2) == 0)
            {
                sprintf(Result, "8b cd");
            }
            //edx
            if (strcmp("eax", temp) == 0 && strcmp(",edx", temp2) == 0)
            {
                sprintf(Result, "8b d0");
            }
            else if (strcmp("ebx", temp) == 0 && strcmp(",edx", temp2) == 0)
            {
                sprintf(Result, "8b d3");
            }
            else if (strcmp("ecx", temp) == 0 && strcmp(",edx", temp2) == 0)
            {
                sprintf(Result, "8b d1");
            }
            else if (strcmp("edx", temp) == 0 && strcmp(",edx", temp2) == 0)
            {
                sprintf(Result, "8b d2");
            }
            else if (strcmp("esi", temp) == 0 && strcmp(",edx", temp2) == 0)
            {
                sprintf(Result, "8b d6");
            }
            else if (strcmp("edi", temp) == 0 && strcmp(",edx", temp2) == 0)
            {
                sprintf(Result, "8b d7");
            }
            else if (strcmp("ebp", temp) == 0 && strcmp(",edx", temp2) == 0)
            {
                sprintf(Result, "8b d5");
            }
            //ebp
            if (strcmp("eax", temp) == 0 && strcmp(",ebp", temp2) == 0)
            {
                sprintf(Result, "8b e8");
            }
            else if (strcmp("ebx", temp) == 0 && strcmp(",ebp", temp2) == 0)
            {
                sprintf(Result, "8b eb");
            }
            else if (strcmp("ecx", temp) == 0 && strcmp(",ebp", temp2) == 0)
            {
                sprintf(Result, "8b e9");
            }
            else if (strcmp("edx", temp) == 0 && strcmp(",ebp", temp2) == 0)
            {
                sprintf(Result, "8b ea");
            }
            else if (strcmp("esi", temp) == 0 && strcmp(",ebp", temp2) == 0)
            {
                sprintf(Result, "8b ee");
            }
            else if (strcmp("edi", temp) == 0 && strcmp(",ebp", temp2) == 0)
            {
                sprintf(Result, "8b ef");
            }
            else if (strcmp("ebp", temp) == 0 && strcmp(",ebp", temp2) == 0)
            {
                sprintf(Result, "8b ed");
            }
            else if (strcmp("esp", temp) == 0 && strcmp(",ebp", temp2) == 0)
            {
                sprintf(Result, "8b ec");
            }
            //esi
            if (strcmp("eax", temp) == 0 && strcmp(",esi", temp2) == 0)
            {
                sprintf(Result, "8b f0");
            }
            else if (strcmp("ebx", temp) == 0 && strcmp(",esi", temp2) == 0)
            {
                sprintf(Result, "8b f3");
            }
            else if (strcmp("ecx", temp) == 0 && strcmp(",esi", temp2) == 0)
            {
                sprintf(Result, "8b f1");
            }
            else if (strcmp("edx", temp) == 0 && strcmp(",esi", temp2) == 0)
            {
                sprintf(Result, "8b f2");
            }
            else if (strcmp("esi", temp) == 0 && strcmp(",esi", temp2) == 0)
            {
                sprintf(Result, "8b f6");
            }
            else if (strcmp("edi", temp) == 0 && strcmp(",esi", temp2) == 0)
            {
                sprintf(Result, "8b f7");
            }
            else if (strcmp("ebp", temp) == 0 && strcmp(",esi", temp2) == 0)
            {
                sprintf(Result, "8b f5");
            }
            else if (strcmp("esp", temp) == 0 && strcmp(",esi", temp2) == 0)
            {
                sprintf(Result, "8b f4");
            }
            //edi
            if (strcmp("eax", temp) == 0 && strcmp(",edi", temp2) == 0)
            {
                sprintf(Result, "8b f8");
            }
            else if (strcmp("ebx", temp) == 0 && strcmp(",edi", temp2) == 0)
            {
                sprintf(Result, "8b fb");
            }
            else if (strcmp("ecx", temp) == 0 && strcmp(",edi", temp2) == 0)
            {
                sprintf(Result, "8b f9");
            }
            else if (strcmp("edx", temp) == 0 && strcmp(",edi", temp2) == 0)
            {
                sprintf(Result, "8b fa");
            }
            else if (strcmp("esi", temp) == 0 && strcmp(",edi", temp2) == 0)
            {
                sprintf(Result, "8b fe");
            }
            else if (strcmp("edi", temp) == 0 && strcmp(",edi", temp2) == 0)
            {
                sprintf(Result, "8b ff");
            }
            else if (strcmp("ebp", temp) == 0 && strcmp(",edi", temp2) == 0)
            {
                sprintf(Result, "8b fd");
            }
            else if (strcmp("esp", temp) == 0 && strcmp(",edi", temp2) == 0)
            {
                sprintf(Result, "8b fc");
            }
        }
        else if (asm_code[4] == '&') //指针
        {
            //立即数
            if (asm_code[5] == '$') //说明是立即数
            {
                char temp[30] = {0};
                int i = 0;
                for (i = 6; i < strlen(asm_code) - 4; i++)
                {
                    temp[i - 6] = asm_code[i];
                }
                temp[i - 6] = '\0';
                if (strlen(temp) == 2) //这是一个byte
                {
                    //判断寄存器
                    char temp2[30] = {0};
                    int j = 0;
                    for (j = 6 + strlen(temp); j < strlen(asm_code); j++)
                    {
                        temp2[j - 6 - strlen(temp)] = asm_code[j];
                    }
                    temp2[j - 6 - strlen(temp)] = '\0';
                    if (strcmp(temp2, ",eax") == 0)
                    {
                        sprintf(Result, "a1 %s 00 00 00", temp);
                    }
                    if (strcmp(temp2, ",ebx") == 0)
                    {
                        sprintf(Result, "8b 1d %s 00 00 00", temp);
                    }
                    if (strcmp(temp2, ",ecx") == 0)
                    {
                        sprintf(Result, "8b 0d %s 00 00 00", temp);
                    }
                    if (strcmp(temp2, ",edx") == 0)
                    {
                        sprintf(Result, "8b 15 %s 00 00 00", temp);
                    }
                    if (strcmp(temp2, ",esi") == 0)
                    {
                        sprintf(Result, "8b 35 %s 00 00 00", temp);
                    }
                    if (strcmp(temp2, ",edi") == 0)
                    {
                        sprintf(Result, "8b 3d %s 00 00 00", temp);
                    }
                }
                else if (strlen(temp) == 1)
                {
                    char bmp3[] = {'0', temp[0], '\0'};
                    char temp2[30];
                    int j = 0;
                    for (j = 6 + strlen(bmp3); j < strlen(asm_code); j++)
                    {
                        temp2[j - 6 - strlen(bmp3)] = asm_code[j];
                    }
                    temp2[j - 6 - strlen(bmp3)] = '\0';
                    if (strcmp(temp2, ",eax") == 0)
                    {
                        sprintf(Result, "a1 %s 00 00 00", bmp3);
                    }
                    if (strcmp(temp2, ",ebx") == 0)
                    {
                        sprintf(Result, "8b 1d %s 00 00 00", bmp3);
                    }
                    if (strcmp(temp2, ",ecx") == 0)
                    {
                        sprintf(Result, "8b 0d %s 00 00 00", bmp3);
                    }
                    if (strcmp(temp2, ",edx") == 0)
                    {
                        sprintf(Result, "8b 15 %s 00 00 00", bmp3);
                    }
                    if (strcmp(temp2, ",esi") == 0)
                    {
                        sprintf(Result, "8b 35 %s 00 00 00", bmp3);
                    }
                    if (strcmp(temp2, ",edi") == 0)
                    {
                        sprintf(Result, "8b 3d %s 00 00 00", bmp3);
                    }
                }
                else if (strlen(temp) == 4) //a word
                {
                    char num2[] = {temp[0], temp[1], 0};
                    char num1[] = {temp[2], temp[3], 0};
                    char temp2[30] = {0};
                    int j = 0;
                    for (j = 6 + strlen(temp); j < strlen(asm_code); j++)
                    {
                        temp2[j - 6 - strlen(temp)] = asm_code[j];
                    }
                    temp2[j - 6 - strlen(temp)] = '\0';
                    if (strcmp(temp2, ",eax") == 0)
                    {
                        sprintf(Result, "a1 %s %s 00 00", num1, num2);
                    }
                    if (strcmp(temp2, ",ebx") == 0)
                    {
                        sprintf(Result, "8b 1d %s %s 00 00", num1, num2);
                    }
                    if (strcmp(temp2, ",ecx") == 0)
                    {
                        sprintf(Result, "8b 0d %s %s 00 00", num1, num2);
                    }
                    if (strcmp(temp2, ",edx") == 0)
                    {
                        sprintf(Result, "8b 15 %s %s 00 00", num1, num2);
                    }
                    if (strcmp(temp2, ",esi") == 0)
                    {
                        sprintf(Result, "8b 35 %s %s 00 00", num1, num2);
                    }
                    if (strcmp(temp2, ",edi") == 0)
                    {
                        sprintf(Result, "8b 3d %s %s 00 00", num1, num2);
                    }
                }
                // double world
                else if (strlen(temp) == 8)
                {
                    char num4[] = {temp[0], temp[1], 0};
                    char num3[] = {temp[2], temp[3], 0};
                    char num2[] = {temp[4], temp[5], 0};
                    char num1[] = {temp[6], temp[7], 0};
                    char temp2[30] = {0};
                    int j = 0;
                    for (j = 6 + strlen(temp); j < strlen(asm_code); j++)
                    {
                        temp2[j - 6 - strlen(temp)] = asm_code[j];
                    }
                    temp2[j - 6 - strlen(temp)] = '\0';
                    if (strcmp(temp2, ",eax") == 0)
                    {
                        sprintf(Result, "a1 %s %s %s %s", num1, num2, num3, num4);
                    }
                    if (strcmp(temp2, ",ebx") == 0)
                    {
                        sprintf(Result, "8b 1d %s %s %s %s", num1, num2, num3, num4);
                    }
                    if (strcmp(temp2, ",ecx") == 0)
                    {
                        sprintf(Result, "8b 0d %s %s %s %s", num1, num2, num3, num4);
                    }
                    if (strcmp(temp2, ",edx") == 0)
                    {
                        sprintf(Result, "8b 15 %s %s %s %s", num1, num2, num3, num4);
                    }
                    if (strcmp(temp2, ",esi") == 0)
                    {
                        sprintf(Result, "8b 35 %s %s %s %s", num1, num2, num3, num4);
                    }
                    if (strcmp(temp2, ",edi") == 0)
                    {
                        sprintf(Result, "8b 3d %s %s %s %s", num1, num2, num3, num4);
                    }
                }
                else
                {
                    printf("\nError!");
					page_kfree(Result,strlen(asm_code)+1);
					page_kfree(asm_code,1000);
                    return 0;
                }
            }
        }
        else
        {
			page_kfree(Result,strlen(asm_code)+1);
			page_kfree(asm_code,1000);
            printf("\nSyntaxError-->%c", asm_code[4]);
            return 0;
        }
    }
    else if (strncmp(asm_code, "xchg ", 5) == 0)
    {
        //get first reg
        char temp[30] = {0};
        int i = 0;
        for (i = 6;i<strlen(asm_code)-4; i++)
        {
            temp[i - 6] = asm_code[i];
        }
        temp[i - 6] = '\0';
        //get second reg
        char temp2[30] = {0};
        int j = 0;
        for (j = 6 + strlen(temp); j < strlen(asm_code); j++)
        {
            temp2[j - 6 - strlen(temp)] = asm_code[j];
        }
        temp2[j - 6 - strlen(temp)] = '\0';
        //printf("\n%s %s", temp, temp2);
        //转译为xchg的机器码
        if(strcmp(temp, "eax") == 0)
        {
            if(strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "90");
            }
            else if(strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "93");
            }
            else if(strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "91");
            }
            else if(strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "92");
            }
            else if(strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "96");
            }
            else if(strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "97");
            }
        }
        else if(strcmp(temp, "ebx") == 0)
        {
            if(strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "93");
            }
            else if(strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "87 db");
            }
            else if(strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "87 d9");
            }
            else if(strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "87 da");
            }
            else if(strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "87 de");
            }
            else if(strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "87 df");
            }
        }
        else if(strcmp(temp, ",ecx") == 0)
        {
            if(strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "91");
            }
            else if(strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "87 cb");
            }
            else if(strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "87 c9");
            }
            else if(strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "87 ca");
            }
            else if(strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "87 ce");
            }
            else if(strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "87 cf");
            }
        }
        else if(strcmp(temp, "edx") == 0)
        {
            if(strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "92");
            }
            else if(strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "87 d3");
            }
            else if(strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "87 d1");
            }
            else if(strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "87 d2");
            }
            else if(strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "87 d6");
            }
            else if(strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "87 d7");
            }
        }
        //esi
        else if(strcmp(temp, "esi") == 0)
        {
            if(strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "96");
            }
            else if(strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "87 f3");
            }
            else if(strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "87 f1");
            }
            else if(strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "87 f2");
            }
            else if(strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "87 f6");
            }
            else if(strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "87 f7");
            }
        }
        //edi
        else if(strcmp(temp, "edi") == 0)
        {
            if(strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "97");
            }
            else if(strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "87 fb");
            }
            else if(strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "87 f9");
            }
            else if(strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "87 fa");
            }
            else if(strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "87 fe");
            }
            else if(strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "87 ff");
            }
        }
        else
        {
            printf("\nSyntaxError-->%c", asm_code[4]);
			page_kfree(Result,strlen(asm_code)+1);
			page_kfree(asm_code,1000);
            return 0;
        }
    }
    else if (strncmp(asm_code, "push ", 5) == 0)
    {
        /*获取将要压入栈的寄存器（32位）*/
        char temp[30] = {0};
        int i = 0;
        for (i = 6; i < strlen(asm_code); i++)
        {
            temp[i - 6] = asm_code[i];
        }
        temp[i - 6] = '\0';
        if (strcmp(temp, "eax") == 0)
        {
            sprintf(Result, "50");
        }
        if (strcmp(temp, "ebx") == 0)
        {
            sprintf(Result, "53");
        }
        if (strcmp(temp, "ecx") == 0)
        {
            sprintf(Result, "51");
        }
        if (strcmp(temp, "edx") == 0)
        {
            sprintf(Result, "52");
        }
        if (strcmp(temp, "esi") == 0)
        {
            sprintf(Result, "56");
        }
        if (strcmp(temp, "edi") == 0)
        {
            sprintf(Result, "57");
        }
        if (strcmp(temp, "ebp") == 0)
        {
            sprintf(Result, "55");
        }
        if (strcmp(temp, "esp") == 0)
        {
            sprintf(Result, "54");
        }
    }
    else if (strncmp(asm_code, "pop ", 4) == 0)
    {
        //获取将要取出栈的寄存器（32位）
        char temp[30] = {0};
        int i = 0;
        for (i = 5; i < strlen(asm_code); i++)
        {
            temp[i - 5] = asm_code[i];
        }
        temp[i - 5] = '\0';
        if (strcmp(temp, "eax") == 0)
        {
            sprintf(Result, "58");
        }
        if (strcmp(temp, "ebx") == 0)
        {
            sprintf(Result, "5b");
        }
        if (strcmp(temp, "ecx") == 0)
        {
            sprintf(Result, "59");
        }
        if (strcmp(temp, "edx") == 0)
        {
            sprintf(Result, "5a");
        }
        if (strcmp(temp, "esi") == 0)
        {
            sprintf(Result, "5e");
        }
        if (strcmp(temp, "edi") == 0)
        {
            sprintf(Result, "5f");
        }
        if (strcmp(temp, "ebp") == 0)
        {
            sprintf(Result, "5d");
        }
        if (strcmp(temp, "esp") == 0)
        {
            sprintf(Result, "5c");
        }
    }
    else if (strncmp(asm_code, "cmp ", 4) == 0 && asm_code[4] == '$') //立即数
    {
        //寄存器与数字比较
        //先数字 再寄存器
        //获取数字
        char temp[30] = {0};
        int i = 0;
        for (i = 5; i < strlen(asm_code) - 4; i++)
        {
            temp[i - 5] = asm_code[i];
        }
        temp[i - 5] = '\0';
        //获取寄存器
        char temp2[30] = {0};
        int j = 0;
        for (j = 5 + strlen(temp); j < strlen(asm_code); j++)
        {
            temp2[j - 5 - strlen(temp)] = asm_code[j];
        }
        temp2[j - 5 - strlen(temp)] = '\0';

        if (strlen(temp) == 2) //byte
        {

            if (strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "83 f8 %s", temp);
            }
            if (strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "83 fb %s", temp);
            }
            if (strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "83 f9 %s", temp);
            }
            if (strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "83 fa %s", temp);
            }
            if (strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "83 fe %s", temp);
            }
            if (strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "83 ff %s", temp);
            }
        }
        if (strlen(temp) == 4) //word
        {
            //拆分数字
            char num1[30] = {temp[0], temp[1], 0};
            char num2[30] = {temp[2], temp[3], 0};
            if (strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "81 f8 %s %s", num2, num1);
            }
            if (strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "81 fb %s %s", num2, num1);
            }
            if (strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "81 f9 %s %s", num2, num1);
            }
            if (strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "81 fa %s %s", num2, num1);
            }
            if (strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "81 fe %s %s", num2, num1);
            }
            if (strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "81 ff %s %s", num2, num1);
            }
        }
        else if (strlen(temp) == 8) //dowrd
        {
            //拆分数字
            char num1[30] = {temp[0], temp[1], 0};
            char num2[30] = {temp[2], temp[3], 0};
            char num3[30] = {temp[4], temp[5], 0};
            char num4[30] = {temp[6], temp[7], 0};
            if (strcmp(temp2, ",eax") == 0)
            {
                sprintf(Result, "81 f8 %s %s %s %s", num4, num3, num2, num1);
            }
            if (strcmp(temp2, ",ebx") == 0)
            {
                sprintf(Result, "81 fb %s %s %s %s", num4, num3, num2, num1);
            }
            if (strcmp(temp2, ",ecx") == 0)
            {
                sprintf(Result, "81 f9 %s %s %s %s", num4, num3, num2, num1);
            }
            if (strcmp(temp2, ",edx") == 0)
            {
                sprintf(Result, "81 fa %s %s %s %s", num4, num3, num2, num1);
            }
            if (strcmp(temp2, ",esi") == 0)
            {
                sprintf(Result, "81 fe %s %s %s %s", num4, num3, num2, num1);
            }
            if (strcmp(temp2, ",edi") == 0)
            {
                sprintf(Result, "81 ff %s %s %s %s", num4, num3, num2, num1);
            }
        }
    }
    else if (strncmp(asm_code, "cmp ", 4) == 0 && asm_code[4] == '%') //寄存器
    {
        //获取寄存器
        char temp[30] = {0};
        int i = 0;
        for (i = 5; i < strlen(asm_code) - 4; i++)
        {
            temp[i - 5] = asm_code[i];
        }
        temp[i - 5] = '\0';
        //获取寄存器
        char temp2[30] = {0};
        int j = 0;
        for (j = 5 + strlen(temp); j < strlen(asm_code); j++)
        {
            temp2[j - 5 - strlen(temp)] = asm_code[j];
        }
        temp2[j - 5 - strlen(temp)] = '\0';
        if (strcmp(temp2, ",eax") == 0 && strcmp(temp, "ebx"))
        {
            sprintf(Result, "3b d8", temp);
        }
        if (strcmp(temp2, ",ebx") == 0 && strcmp(temp, "ebx"))
        {
            sprintf(Result, "3b db", temp);
        }
        if (strcmp(temp2, ",ecx") == 0 && strcmp(temp, "ebx"))
        {
            sprintf(Result, "3b cb", temp);
        }
        if (strcmp(temp2, ",edx") == 0 && strcmp(temp, "ebx"))
        {
            sprintf(Result, "3b d3", temp);
        }
        if (strcmp(temp2, ",esi") == 0 && strcmp(temp, "ebx"))
        {
            sprintf(Result, "3b f3", temp);
        }
        if (strcmp(temp2, ",edi") == 0 && strcmp(temp, "ebx"))
        {
            sprintf(Result, "3b fb", temp);
        }

        if (strcmp(temp2, ",eax") == 0 && strcmp(temp, "eax"))
        {
            sprintf(Result, "3b c0", temp);
        }
        if (strcmp(temp2, ",ebx") == 0 && strcmp(temp, "eax"))
        {
            sprintf(Result, "3b c3", temp);
        }
        if (strcmp(temp2, ",ecx") == 0 && strcmp(temp, "eax"))
        {
            sprintf(Result, "3b c1", temp);
        }
        if (strcmp(temp2, ",edx") == 0 && strcmp(temp, "eax"))
        {
            sprintf(Result, "3b c2", temp);
        }
        if (strcmp(temp2, ",esi") == 0 && strcmp(temp, "eax"))
        {
            sprintf(Result, "3b c6", temp);
        }
        if (strcmp(temp2, ",edi") == 0 && strcmp(temp, "eax"))
        {
            sprintf(Result, "3b c7", temp);
        }

        if (strcmp(temp2, ",eax") == 0 && strcmp(temp, "ecx"))
        {
            sprintf(Result, "3b c8", temp);
        }
        if (strcmp(temp2, ",ebx") == 0 && strcmp(temp, "ecx"))
        {
            sprintf(Result, "3b cb", temp);
        }
        if (strcmp(temp2, ",ecx") == 0 && strcmp(temp, "ecx"))
        {
            sprintf(Result, "3b c9", temp);
        }
        if (strcmp(temp2, ",edx") == 0 && strcmp(temp, "ecx"))
        {
            sprintf(Result, "3b ca", temp);
        }
        if (strcmp(temp2, ",esi") == 0 && strcmp(temp, "ecx"))
        {
            sprintf(Result, "3b ce", temp);
        }
        if (strcmp(temp2, ",edi") == 0 && strcmp(temp, "ecx"))
        {
            sprintf(Result, "3b cf", temp);
        }

        if (strcmp(temp2, ",eax") == 0 && strcmp(temp, "edx"))
        {
            sprintf(Result, "3b d0", temp);
        }
        if (strcmp(temp2, ",ebx") == 0 && strcmp(temp, "edx"))
        {
            sprintf(Result, "3b d3", temp);
        }
        if (strcmp(temp2, ",ecx") == 0 && strcmp(temp, "edx"))
        {
            sprintf(Result, "3b d1", temp);
        }
        if (strcmp(temp2, ",edx") == 0 && strcmp(temp, "edx"))
        {
            sprintf(Result, "3b d2", temp);
        }
        if (strcmp(temp2, ",esi") == 0 && strcmp(temp, "edx"))
        {
            sprintf(Result, "3b d6", temp);
        }
        if (strcmp(temp2, ",edi") == 0 && strcmp(temp, "edx"))
        {
            sprintf(Result, "3b d7", temp);
        }

        if (strcmp(temp2, ",eax") == 0 && strcmp(temp, "esi"))
        {
            sprintf(Result, "3b f0", temp);
        }
        if (strcmp(temp2, ",ebx") == 0 && strcmp(temp, "esi"))
        {
            sprintf(Result, "3b f3", temp);
        }
        if (strcmp(temp2, ",ecx") == 0 && strcmp(temp, "esi"))
        {
            sprintf(Result, "3b f1", temp);
        }
        if (strcmp(temp2, ",edx") == 0 && strcmp(temp, "esi"))
        {
            sprintf(Result, "3b f2", temp);
        }
        if (strcmp(temp2, ",esi") == 0 && strcmp(temp, "esi"))
        {
            sprintf(Result, "3b f6", temp);
        }
        if (strcmp(temp2, ",edi") == 0 && strcmp(temp, "esi"))
        {
            sprintf(Result, "3b f7", temp);
        }

        if (strcmp(temp2, ",eax") == 0 && strcmp(temp, "edi"))
        {
            sprintf(Result, "3b f8", temp);
        }
        if (strcmp(temp2, ",ebx") == 0 && strcmp(temp, "edi"))
        {
            sprintf(Result, "3b fb", temp);
        }
        if (strcmp(temp2, ",ecx") == 0 && strcmp(temp, "edi"))
        {
            sprintf(Result, "3b f9", temp);
        }
        if (strcmp(temp2, ",edx") == 0 && strcmp(temp, "edi"))
        {
            sprintf(Result, "3b fa", temp);
        }
        if (strcmp(temp2, ",esi") == 0 && strcmp(temp, "edi"))
        {
            sprintf(Result, "3b fe", temp);
        }
        if (strcmp(temp2, ",edi") == 0 && strcmp(temp, "edi"))
        {
            sprintf(Result, "3b ff", temp);
        }
    }
    else if (strncmp(asm_code, "int ", 4) == 0)
    {
        char temp[30] = {0};
        int i = 0;
        for (i = 4; i < strlen(asm_code) + 1; i++)
        {
            temp[i - 4] = asm_code[i];
        }
        temp[i - 4] = '\0';
        sprintf(Result, "cd %s", temp);
    }
    else if (strcmp(asm_code, "retf") == 0)
    {
        sprintf(Result, "cb");
    }
    else if (strcmp(asm_code, "ret") == 0)
    {
        sprintf(Result, "c3");
    }
    else if (strncmp(asm_code, "jmp ", 4) == 0)
    {
        if (asm_code[4] == '%') //寄存器
        {
            char temp[30] = {0};
            //获取寄存器名
            int i = 0;
            for (i = 5; i < strlen(asm_code) + 1; i++)
            {
                temp[i - 5] = asm_code[i];
            }
            temp[i - 5] = '\0';
            if (strcmp(temp, "eax") == 0)
            {
                sprintf(Result, "ff e0");
            }
            else if (strcmp(temp, "ebx") == 0)
            {
                sprintf(Result, "ff e3");
            }
            else if (strcmp(temp, "ecx") == 0)
            {
                sprintf(Result, "ff e1");
            }
            else if (strcmp(temp, "edx") == 0)
            {
                sprintf(Result, "ff e2");
            }
            else if (strcmp(temp, "esi") == 0)
            {
                sprintf(Result, "ff e6");
            }
            else if (strcmp(temp, "edi") == 0)
            {
                sprintf(Result, "ff e7");
            }
        } else if (asm_code[4] == '$') {	// 立即数
			//获取立即数
			char temp[30] = { 0 };
			int i = 0;
			for (i = 5; i < strlen(asm_code); i++)
			{
				temp[i - 5] = asm_code[i];
			}
			temp[i - 5] = '\0';
			int num = strtol(temp, NULL, 16);
			// 1003 * 8 = 0x1f58
			sprintf(Result, "ea %02x %02x %02x %02x 00 18",num&0xff,(num&0xff00)>>8,(num&0xff0000)>>16,(num&0xff000000)>>24);
		}
    }
    else if (strncmp(asm_code, "je ", 3) == 0)
    {
        //获取立即数
        char temp[30] = { 0 };
        int i = 0;
        for (i = 4; i < strlen(asm_code); i++)
        {
            temp[i - 4] = asm_code[i];
        }
        temp[i - 4] = '\0';
        int num = strtol(temp, NULL, 16);
		if (num-addr-2 > 0xff) {	// 跳转的大小大于256
			print("\nJumping address is too big.");
		} else {
			sprintf(Result, "74 %02x",(num-addr-2)&0xff);
		}
    }
    else if (strncmp(asm_code, "jne ", 4) == 0)
    {
        //获取立即数
        char temp[30] = { 0 };
        int i = 0;
        for (i = 5; i < strlen(asm_code); i++)
        {
            temp[i - 5] = asm_code[i];
        }
        temp[i - 5] = '\0';
        int num = strtol(temp, NULL, 16);
		if (num-addr-2 > 0xff) {	// 跳转的大小大于256
			print("\nJumping address is too big.");
		} else {
			sprintf(Result, "75 %02x",(num-addr-2)&0xff);
		}
    }
    else if (strncmp(asm_code, "ja ", 3) == 0)
    {
        //获取立即数
        char temp[30] = { 0 };
        int i = 0;
        for (i = 4; i < strlen(asm_code); i++)
        {
            temp[i - 4] = asm_code[i];
        }
        temp[i - 4] = '\0';
        int num = strtol(temp, NULL, 16);
		if (num-addr-2 > 0xff) {	// 跳转的大小大于256
			print("\nJumping address is too big.");
		} else {
			sprintf(Result, "77 %02x",(num-addr-2)&0xff);
		}
    }
    else if (strncmp(asm_code, "jna ", 4) == 0)
    {
        //获取立即数
        char temp[30] = { 0 };
        int i = 0;
        for (i = 5; i < strlen(asm_code); i++)
        {
            temp[i - 5] = asm_code[i];
        }
        temp[i - 5] = '\0';
        int num = strtol(temp, NULL, 16);
		if (num-addr-2 > 0xff) {	// 跳转的大小大于256
			print("\nJumping address is too big.");
		} else {
			sprintf(Result, "76 %02x",(num-addr-2)&0xff);
		}
    }
    else if (strncmp(asm_code, "ja ", 3) == 0)
    {
        //获取立即数
        char temp[30] = { 0 };
        int i = 0;
        for (i = 4; i < strlen(asm_code); i++)
        {
            temp[i - 4] = asm_code[i];
        }
        temp[i - 4] = '\0';
        int num = strtol(temp, NULL, 16);
		if (num-addr-2 > 0xff) {	// 跳转的大小大于256
			print("\nJumping address is too big.");
		} else {
			sprintf(Result, "72 %02x",(num-addr-2)&0xff);
		}
    }
    else if (strncmp(asm_code, "jna ", 4) == 0)
    {
        //获取立即数
        char temp[30] = { 0 };
        int i = 0;
        for (i = 5; i < strlen(asm_code); i++)
        {
            temp[i - 5] = asm_code[i];
        }
        temp[i - 5] = '\0';
        int num = strtol(temp, NULL, 16);
		if (num-addr-2 > 0xff) {	// 跳转的大小大于256
			print("\nJumping address is too big.");
		} else {
			sprintf(Result, "73 %02x",(num-addr-2)&0xff);
		}
    }
    else if (strncmp(asm_code, "add ", 4) == 0)
    {

        if (asm_code[4] == '$') //这是个立即数
        {
            //获取立即数
            char temp[30] = {0};
            int i = 0;
            for (i = 5; i < strlen(asm_code) - 4; i++)
            {
                temp[i - 5] = asm_code[i];
            }
            temp[i - 5] = '\0';
            //获取寄存器名
            char temp2[30] = {0};
            int j = 0;
            for (j = 5 + strlen(temp); j < strlen(asm_code); j++)
            {
                temp2[j - 5 - strlen(temp)] = asm_code[j];
            }
            temp2[j - 5 - strlen(temp)] = '\0';
            //cout << "Ok!\n" << temp << endl << temp2 << endl;
            if (1)
            {
                if (strlen(temp) == 2) //byte
                {
                    if (strcmp(temp2, ",eax") == 0)
                    {
                        sprintf(Result, "83 c0 %s", temp);
                    }
                    if (strcmp(temp2, ",ebx") == 0)
                    {
                        sprintf(Result, "83 c3 %s", temp);
                    }
                    if (strcmp(temp2, ",ecx") == 0)
                    {
                        sprintf(Result, "83 c1 %s", temp);
                    }
                    if (strcmp(temp2, ",edx") == 0)
                    {
                        sprintf(Result, "83 c2 %s", temp);
                    }
                    if (strcmp(temp2, ",esi") == 0)
                    {
                        sprintf(Result, "83 c6 %s", temp);
                    }
                    if (strcmp(temp2, ",edi") == 0)
                    {
                        sprintf(Result, "83 c7 %s", temp);
                    }
                }
                else if (strlen(temp) == 4) //word
                {
                    char num1[30] = {temp[0], temp[1], 0};
                    char num2[30] = {temp[2], temp[3], 0};
                    if (strcmp(temp2, ",eax") == 0)
                    {
                        sprintf(Result, "83 c0 %s %s", num2, num1);
                    }
                    if (strcmp(temp2, ",ebx") == 0)
                    {
                        sprintf(Result, "83 c3 %s %s", num2, num1);
                    }
                    if (strcmp(temp2, ",ecx") == 0)
                    {
                        sprintf(Result, "83 c1 %s %s", num2, num1);
                    }
                    if (strcmp(temp2, ",edx") == 0)
                    {
                        sprintf(Result, "83 c2 %s %s", num2, num1);
                    }
                    if (strcmp(temp2, ",esi") == 0)
                    {
                        sprintf(Result, "83 c6 %s %s", num2, num1);
                    }
                    if (strcmp(temp2, ",edi") == 0)
                    {
                        sprintf(Result, "83 c7 %s %s", num2, num1);
                    }
                }
                if (strlen(temp) == 8) //dword
                {
                    char num1[30] = {temp[0], temp[1], 0};
                    char num2[30] = {temp[2], temp[3], 0};
                    char num3[30] = {temp[4], temp[5], 0};
                    char num4[30] = {temp[6], temp[7], 0};
                    if (strcmp(temp2, ",eax") == 0)
                    {
                        sprintf(Result, "83 c0 %s %s %s %s", num4, num3, num2, num1);
                    }
                    if (strcmp(temp2, ",ebx") == 0)
                    {
                        sprintf(Result, "83 c3 %s %s %s %s", num4, num3, num2, num1);
                    }
                    if (strcmp(temp2, ",ecx") == 0)
                    {
                        sprintf(Result, "83 c1 %s %s %s %s", num4, num3, num2, num1);
                    }
                    if (strcmp(temp2, ",edx") == 0)
                    {
                        sprintf(Result, "83 c2 %s %s %s %s", num4, num3, num2, num1);
                    }
                    if (strcmp(temp2, ",esi") == 0)
                    {
                        sprintf(Result, "83 c6 %s %s %s %s", num4, num3, num2, num1);
                    }
                    if (strcmp(temp2, ",edi") == 0)
                    {
                        sprintf(Result, "83 c7 %s %s %s %s", num4, num3, num2, num1);
                    }
                }
            }
        }
        else if (asm_code[4] == '%') //这是一个寄存器
        {
            //获取寄存器1名
            char temp[30] = {0};
            int i = 0;
            for (i = 5; i < strlen(asm_code) - 4; i++)
            {
                temp[i - 5] = asm_code[i];
            }
            temp[i - 5] = '\0';
            //获取寄存器2名
            char temp2[30] = {0};
            int j = 0;
            for (j = strlen(temp) + 5; j < strlen(asm_code); j++)
            {
                temp2[j - strlen(temp) - 5] = asm_code[j];
            }
            temp2[j - (strlen(asm_code) - 5)] = '\0';
            if (strcmp(temp2, ",eax") == 0)
            {
                if (strcmp(temp, "eax") == 0)
                {
                    sprintf(Result, "03 c0");
                }
                if (strcmp(temp, "ebx") == 0)
                {
                    sprintf(Result, "03 c3");
                }
                if (strcmp(temp, "ecx") == 0)
                {
                    sprintf(Result, "03 c1");
                }
                if (strcmp(temp, "edx") == 0)
                {
                    sprintf(Result, "03 c2");
                }
                if (strcmp(temp, "esi") == 0)
                {
                    sprintf(Result, "03 c6");
                }
                if (strcmp(temp, "edi") == 0)
                {
                    sprintf(Result, "03 c7");
                }
            }
            if (strcmp(temp2, ",ebx") == 0)
            {
                if (strcmp(temp, "eax") == 0)
                {
                    sprintf(Result, "03 d8");
                }
                if (strcmp(temp, "ebx") == 0)
                {
                    sprintf(Result, "03 db");
                }
                if (strcmp(temp, "ecx") == 0)
                {
                    sprintf(Result, "03 d9");
                }
                if (strcmp(temp, "edx") == 0)
                {
                    sprintf(Result, "03 da");
                }
                if (strcmp(temp, "esi") == 0)
                {
                    sprintf(Result, "03 de");
                }
                if (strcmp(temp, "edi") == 0)
                {
                    sprintf(Result, "13 df");
                }
            }
            if (strcmp(temp2, ",ecx") == 0)
            {
                if (strcmp(temp, "eax") == 0)
                {
                    sprintf(Result, "03 c8");
                }
                if (strcmp(temp, "ebx") == 0)
                {
                    sprintf(Result, "03 cb");
                }
                if (strcmp(temp, "ecx") == 0)
                {
                    sprintf(Result, "03 c9");
                }
                if (strcmp(temp, "edx") == 0)
                {
                    sprintf(Result, "03 ca");
                }
                if (strcmp(temp, "esi") == 0)
                {
                    sprintf(Result, "03 ce");
                }
                if (strcmp(temp, "edi") == 0)
                {
                    sprintf(Result, "03 cf");
                }
            }
            if (strcmp(temp2, ",edx") == 0)
            {
                if (strcmp(temp, "eax") == 0)
                {
                    sprintf(Result, "03 d0");
                }
                if (strcmp(temp, "ebx") == 0)
                {
                    sprintf(Result, "03 d3");
                }
                if (strcmp(temp, "ecx") == 0)
                {
                    sprintf(Result, "03 d1");
                }
                if (strcmp(temp, "edx") == 0)
                {
                    sprintf(Result, "03 d2");
                }
                if (strcmp(temp, "esi") == 0)
                {
                    sprintf(Result, "03 d6");
                }
                if (strcmp(temp, "edi") == 0)
                {
                    sprintf(Result, "03 d7");
                }
            }
            if (strcmp(temp2, ",esi") == 0)
            {
                if (strcmp(temp, "eax") == 0)
                {
                    sprintf(Result, "03 f0");
                }
                if (strcmp(temp, "ebx") == 0)
                {
                    sprintf(Result, "03 f3");
                }
                if (strcmp(temp, "ecx") == 0)
                {
                    sprintf(Result, "03 f1");
                }
                if (strcmp(temp, "edx") == 0)
                {
                    sprintf(Result, "03 f2");
                }
                if (strcmp(temp, "esi") == 0)
                {
                    sprintf(Result, "03 f6");
                }
                if (strcmp(temp, "edi") == 0)
                {
                    sprintf(Result, "03 f7");
                }
            }
            if (strcmp(temp2, ",edi") == 0)
            {
                if (strcmp(temp, "eax") == 0)
                {
                    sprintf(Result, "03 f8");
                }
                if (strcmp(temp, "ebx") == 0)
                {
                    sprintf(Result, "03 fb");
                }
                if (strcmp(temp, "ecx") == 0)
                {
                    sprintf(Result, "03 f9");
                }
                if (strcmp(temp, "edx") == 0)
                {
                    sprintf(Result, "03 fa");
                }
                if (strcmp(temp, "esi") == 0)
                {
                    sprintf(Result, "03 fe");
                }
                if (strcmp(temp, "edi") == 0)
                {
                    sprintf(Result, "03 ff");
                }
            }
        }
    }
    else if (strncmp(asm_code, "sub ", 4) == 0) {}
    else if (strcmp(asm_code, "pusha") == 0)
    {
        sprintf(Result, "60");
    }
    else if (strcmp(asm_code, "popa") == 0)
    {
        sprintf(Result, "61");
    }
    else if (strncmp(asm_code, "db ", 3) == 0)
    {
        if (asm_code[3] == '$') //立即数
        {
            //获取立即数
            char temp[10];
            int i = 0;
            for (i = 4; asm_code[i] != '\0'; i++)
            {
                temp[i - 4] = asm_code[i];
            }
            temp[i - 4] = '\0';
            if(strlen(temp) > 2)
            {
                printf("[Error] : %s is not a valid immediate value.\n", temp);
            	page_kfree(Result,strlen(asm_code)+1);
				page_kfree(asm_code,1000);
				return 0;
            }
            else
            {
                sprintf(Result, "%02x", strtol(temp,NULL,16));
            }
        }
    }
    else if(strcmp(asm_code,"in8")==0)
    {
        sprintf(Result,"ec");
    }
    //in 16
    else if(strcmp(asm_code,"in16")==0)
    {
        sprintf(Result,"66 ed");
    }
    // in32
    else if(strcmp(asm_code,"in32")==0)
    {
        sprintf(Result,"ed");
    }
    else if(strcmp(asm_code,"out8")==0)
    {
        sprintf(Result,"ee");
    }
    //out 16
    else if(strcmp(asm_code,"out16")==0)
    {
        sprintf(Result,"66 ef");
    }
    // out 32
    else if(strcmp(asm_code,"out32")==0)
    {
        sprintf(Result,"ef");
    }
    else
    {

        printf("\nerror by %s", asm_code);
		page_kfree(Result,strlen(asm_code)+1);
		page_kfree(asm_code,1000);
        return 0;
    }
	page_kfree(Result,strlen(asm_code)+1);
	page_kfree(asm_code,1000);
    return Result;
}
void casm_shell()
{
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	int i, j, point, point2 = 0, listflag = 0;
	unsigned char c;
	char casmline[64] = {0};
	unsigned char outcode[1024] = {0};
	unsigned char temp[30];
	char *runapp, *alloc;
	char *stack;
	char *keytable = get_point_of_keytable();
	char *keytable1 = get_point_of_keytable1();
	print("Welcome to CASM version 0.1!\n");
	print("Copyright (C) ASM by min0911 2022\n");
	print("SHELL by zhouzhihao 2022\n");
	while (1)
	{
	jmpout:
		print("\nCASM>>>");
		clean(casmline, 64);
		input(casmline, 64);
	command:
		if (strcmp("code", casmline) == 0)
		{
			point2 = 0;
			listflag = 0;
			clean(casmline, 64);
			for (i = 0;;)
			{
				printchar(0x08);
			again:
				printf("\n%x ", i);
				input_No_El(casmline, 64);
				if (casmline[0] == '\0')
				{
					goto jmpout;
				}
			nextline:
				sprintf(temp, "%s", _Asm(casmline, i));
				if (i < 0x10)
				{
					j = 2;
				}
				else if (i < 0x100)
				{
					j = 3;
				}
				else
				{
					j = 4;
				}
				gotoxy(j, get_y());
				for (; *(char *)(0xb8000 + 160 * get_y() + j * 2) != 0x20; j++)
				{
					*(char *)(0xb8000 + 160 * get_y() + j * 2 + 1) = cons_color & 0xf0 + 0x01;
				}
				j++;
				for (; *(char *)(0xb8000 + 160 * get_y() + j * 2) != 0x20; j++)
				{
					if (*(char *)(0xb8000 + 160 * get_y() + j * 2) == ',')
					{
						continue;
					}
					*(char *)(0xb8000 + 160 * get_y() + j * 2 + 1) = cons_color & 0xf0 + 0x04;
				}
				int k, kt;
				for (k = 0, kt = 0; k < strlen(temp); k++)
				{
					if (temp[k] != ' ')
					{
						kt++;
					}
				}
				i += kt / 2;
				for (point = 0; point != (strlen(temp) + 1) / 3; point++)
				{
					outcode[point2] = (ascii2num(temp[point * 3]) << 4) + ascii2num(temp[point * 3 + 1]);
					point2++;
				}
				clean(casmline, 64);
			}
		}
		else if (strcmp("list", casmline) == 0)
		{
			for (i = 0; i < point2; i++)
			{
				c = outcode[i] >> 4;
				printchar(num2ascii(c));
				c = outcode[i] & 0x0f;
				printchar(num2ascii(c));
				printchar(0x20);
			}
			listflag = 1;
		}
		else if (strcmp("run", casmline) == 0)
		{
			if (listflag == 0)
			{ // 必须要先进行list才能run
				print("Please 'list' first!");
			}
			else if (listflag == 1)
			{
				extern int app_task_num;
				runapp = (char *)page_kmalloc(point2 + 128 * 1024); // 鐢宠?穜unapp鍐呭靠??
				*((int *)(0xfe8)) = runapp;									   // cs_base
				*((int *)(0xfec)) = runapp;									   // ds_base
				runapp[0] = 'A';											   // ????????菚
				memcpy(runapp + 1, outcode, point2);
				set_segmdesc(gdt + 3, point2 + 1, (int)runapp, AR_CODE32_ER);
				set_segmdesc(gdt + 4, point2 + 1 + 128 * 1024, (int)runapp, AR_DATA32_RW + 0x60);
				stack = page_vmalloc(64 * 1024) + 64 * 1024;
				io_cli();
				struct TASK *this_task = AddTask("CASMTASK", 2, 3 * 8, 1, 4 * 8, 1 * 8, stack);
				char *kfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
				char *mfifo = (struct FIFO8 *)page_kmalloc(sizeof(struct FIFO8));
				char *kbuf = (char *)page_kmalloc(32);
				char *mbuf = (char *)page_kmalloc(128);
				fifo8_init(kfifo,32,kbuf);
				fifo8_init(mfifo,128,mbuf);
				TaskSetFIFO(this_task,kfifo,mfifo);
				app_task_num = this_task->sel / 8 - 103;
				SleepTaskFIFO(GetTaskForName("Shell"));
				io_sti();
				while(GetTaskForName("CASMTASK") != 0);
				WakeUp(GetTaskForName("Shell"));
				app_task_num = -1;
				page_kfree(kfifo,sizeof(struct FIFO8));
				page_kfree(mfifo,sizeof(struct FIFO8));
				page_kfree(kbuf,32);
				page_kfree(mbuf,128);
				page_kfree((int)runapp, point2 + 1 + 128 * 1024);
				page_vfree(stack-64*1024,64*1024);
			}
		}
		else if (strcmp("exit", casmline) == 0)
		{
			print("\n");
			return;
		}
		else if (strncmp("cas ", casmline, 4) == 0)
		{
			int addr = 0;
			char *p;
			struct FILEINFO *finfo;
			point2 = 0;
			p = fopen(casmline + 4);
			finfo = Get_File_Address(casmline + 4);
			if (finfo == 0)
			{
				print("File not find!\n");
				return;
			}
			for (i = 0; i != finfo->size; i++)
			{
				printchar(p[i]);
			}
			print("\nThis file,right(Y/N)?");
			c = getch();
			if (c == 'Y' || c == 'y')
			{
				printchar(c);
				print("\n");
				clean(casmline, 64);
				i = 0;
				for (j = 0; i + j != finfo->size + 1;)
				{
					for (i = 0; p[i + j] != 0x0d && p[i + j + 1] != 0x0a; i++)
					{
						if (i + j == finfo->size + 1)
						{
							goto outside;
						}
						casmline[i] = p[i + j];
					}
					j = j + i + 2;
				outside:
					sprintf(&temp, "%s", _Asm(casmline, addr));
					int k, kt;
					for (k = 0, kt = 0; k < strlen(temp); k++)
					{
						if (temp[k] != ' ')
						{
							kt++;
						}
					}
					addr += kt / 2;
					for (point = 0; point != (strlen(temp) + 1) / 3; point++)
					{
						outcode[point2] = (ascii2num(temp[point * 3]) << 4) + ascii2num(temp[point * 3 + 1]);
						point2++;
					}
					clean(casmline, 64);
				}
				printf("The program out size is:%d", point2);
			}
			else
			{
				printchar(c);
				return;
			}
		}
		else if (strncmp("out ", casmline, 4) == 0)
		{
			if (listflag == 0)
			{
				print("Please 'list' first!");
			}
			else if (listflag == 1)
			{
				struct FILEINFO *finfo;
				char *p;
				mkfile(casmline + 4);
				p = fopen(casmline + 4);
				finfo = file_search(casmline + 4, (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
				finfo->size = point2 + 1;
				p[0] = 'A';						// 汇编语言程序（单段程序）
				memcpy(p + 1, outcode, point2); //拷贝代码
			}
		}
		else
		{
			command_run(casmline);
			printchar(0x08);
		}
	}
}