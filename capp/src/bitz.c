#include <syscall.h>
#include <string.h>
int main(int argc,char **argv)
{
    char *Buf = malloc(128);
    GetCmdline(Buf);
    char *arg = malloc(128);
    int i;
    for (i = 0; i < strlen(Buf); i++)
    {
        if (Buf[i] == ' ')
        {
            break;
        }
    }
    i++;
    int j;
    for (j = i; j < strlen(Buf); j++)
    {
        arg[j - i] = Buf[j];
    }
    arg[j] = '\0';
    if (arg[0] == 0)
    {
        print("ERROR:No input file.");
        return 0;
    }
    Bitz(arg);
    return 0;
}
void Bitz(char *filename)
{
    if (filesize(filename) == -1)
    {
        print("File not found\n");
        return;
    }
    int size = filesize(filename);
    unsigned char *file_buffer = (unsigned char *)malloc(size);
    fopen(filename, file_buffer);
    print("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF\n");
    int i;
    int l = 0;
    for (i = 0; i < size; i++)
    {
        char *buffer = malloc(15);
        htoa(file_buffer[i], buffer);
        print(buffer);
        print(" ");
        if ((i+1) % 16 == 0)
        {
            //输出对应的ascii码
            print(" ");
            for (int j = i - 15; j <= i; j++)
            {
                if (file_buffer[j] >= 32 && file_buffer[j] <= 126 && file_buffer[j] != ' ')
                {
                    putch(file_buffer[j]);
                }
                else
                {
                    putch('.');
                }
            }
            l++;
            print("\n");
        }
        if (l == 23)
        {
            print("Press any key to continue...");
            getch();
            print("\n");
            l = 0;
        }
        free(buffer, 15);
    }
    //剩下的
    i--;
    for (;; i++)
    {

        if (i % 16 == 15)
        {
            break;
        }
        print("00 ");
    }
    print(" ");
    for (int j = i - 15; j <= size; j++)
    {
        if (file_buffer[j] >= 32 && file_buffer[j] <= 126)
        {
            putch(file_buffer[j]);
        }
        else
        {
            putch('.');
        }
    }
    free(file_buffer, size);
}
void htoa(unsigned char n, char *_Ptr)
{
    //将n转换为8位的16进制字符串
    char *_Hex = "0123456789ABCDEF";
    _Ptr[0] = _Hex[n >> 4];
    _Ptr[1] = _Hex[n & 0x0F];
    _Ptr[2] = '\0';
}