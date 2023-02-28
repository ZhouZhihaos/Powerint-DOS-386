// 这里是环境变量的具体实现
#include <fs.h>
//首先 我们要解析环境变量的字符串
//环境变量的字符串是以分号分隔的
void Path_GetPath(int count, char *ptr, char *PATH_ADDR) {
  // count 获取第几个环境变量？
  // ptr   储存在哪里？
  // PATH_ADDR 环境变量的信息在哪里？
  // 我们要解析环境变量的字符串
  // printf("count = %d\n",count);
  // printf("PATH_ADDR = %s\n",PATH_ADDR);
  int str_base = 0;
  for (int i = 0, j = 0;; i++) {
    if (PATH_ADDR[i] == ';') {
      ++j;
    }
    if (j == count) {
      str_base = i;
      break;
    }
    if (i >= strlen(PATH_ADDR)) {
      // 没找到
      // printf("\nNOT FOUND PATH\n");
      return;
    }
  }
  if (PATH_ADDR[str_base] == ';') {
    str_base++;
  }
  // printf("\nPATH_ADDR[%d] = %s\n",str_base,PATH_ADDR+str_base);
  // 找到了
  // copy
  int i;
  for (i = 0; PATH_ADDR[str_base + i] != ';'; i++) {
    ptr[i] = PATH_ADDR[str_base + i];
  }
  ptr[i] = '\0';
}
int Path_GetPathCount(char *PATH_ADDR) {
  int count = 0;
  for (int i = 0; i < strlen(PATH_ADDR); i++) {
    if (PATH_ADDR[i] == ';') {
      count++;
    }
  }
  return count;
}
static void GetFullPath(char *result, char *name, char *dictpath) {
  strcpy(result, dictpath);
  strcat(result, "\\");
  strcat(result, name);
}
struct FILEINFO *Path_Find_File(char *fileName, char *PATH_ADDR) {
  char path_result1[100];
  char path_result2[100];
  for (int i = 0; i < Path_GetPathCount(PATH_ADDR); i++) {
    Path_GetPath(i, path_result1, PATH_ADDR);
    GetFullPath(path_result2, fileName, path_result1);
    struct FILEINFO *file = Get_File_Address(path_result2);
    if (file != 0) {
      return file;
    }
  }
  return NULL;
}
void Path_Find_FileName(char *Result, char *fileName, char *PATH_ADDR) {
  char path_result1[100];
  char path_result2[100];
  for (int i = 0; i < Path_GetPathCount(PATH_ADDR); i++) {

    Path_GetPath(i, path_result1, PATH_ADDR);
    GetFullPath(path_result2, fileName, path_result1);
    struct FILEINFO *file = Get_File_Address(path_result2);
    if (file != 0) {
      strcpy(Result, path_result2);
    }
  }
}