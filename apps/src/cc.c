// 利用HZK字库编码制成的汉字拼音输入法
#include <string.h>
#include <syscall.h>

static char pinyin[100] = {0};
int main(int argc, char** argv) {
  // if (filesize("A:\\other\\hanzi.txt") == 0) {
  // 	print("Cannot find hanzi.txt.");
  // 	return 0; }
  // unsigned char *hanzi = (unsigned char
  // *)malloc(filesize("A:\\other\\hanzi.txt"));
  // fopen("A:\\other\\hanzi.txt",hanzi);
  SwitchToHighTextMode();
  HighPrint("Hello World! 你好世界！\n");
  // HighPrint("请输入拼音：");
  // scan(pinyin,100);
  // HighPrint(pinyin);
  // HighPrint("\n");
  // char *p = hanzi;
  // char *q = hanzi;
  // while (*p != '\0')
  // {
  //     if (*p == '\n')
  //     {
  //         *p = '\0';
  //         // 找到拼音
  //         char *bmp = q;
  //         // 找到空格
  //         while (*bmp != ' ')
  //             bmp++;
  //         bmp++;
  //         if(strncmp(pinyin, bmp, strlen(pinyin)) == 0)
  //         {
  //             HighPrint(q);
  // 			HighPrint(" ");
  //         }
  //         q = p + 1;
  //     }
  //     p++;
  // }
  // HighPrint("\n请按ENTER返回系统。。。");
  while(getch()!=0x0a);
  ReTurnToNornalTextMode();
  // free(hanzi,filesize("A:\\other\\hanzi.txt"));
  // free(pinyin,100);
  return 0;
}