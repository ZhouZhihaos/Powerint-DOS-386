#include <cmd.h>
#include <string.h>
int Get_Arg(char *Arg, char *CmdLine, int Count) {
  if (Count == 0) {
    for (int i = 0; i < strlen(CmdLine) && CmdLine[i] != ' '; i++) {
      Arg[i] = CmdLine[i];
    }
    return 0;
  }
  int i = 0, j;
  //获取第Count个参数
  //先判断一共有几个参数
  int co = 0;
  for (i = 0; CmdLine[i] != '\0'; i++) {
    if (CmdLine[i] == ' ') {
      co++;
    }
  }
  if (Count > co) {
    //超出参数个数
    return -1;
  }
  //获取第Count个参数
  co = 0;
  for (i = 0; CmdLine[i] != '\0'; i++) {
    if (CmdLine[i] == ' ') {
      co++;
      if (co == Count) {
        break;
      }
    }
  }
  i++;
  for (j = 0; CmdLine[i] != ' ' && CmdLine[i] != '\0'; i++, j++) {
    Arg[j] = CmdLine[i];
  }
  Arg[j] = 0;
  return 0;
}
int Get_Argc(char *CmdLine) {
  int co = 0;
  for (int i = 0; CmdLine[i] != '\0'; i++) {
    if (CmdLine[i] == ' ') {
      co++;
    }
  }
  return co;
}
