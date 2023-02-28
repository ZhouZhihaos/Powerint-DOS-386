#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// 替换字符串
char* replace(char* str, char* old, char* new) {
  char *ret, *r;
  int i, count = 0;
  int newlen = strlen(new);
  int oldlen = strlen(old);
  for (i = 0; str[i] != '\0'; i++) {
    if (strncmp(&str[i], old, oldlen) == 0)
      count++;
  }
  ret = (char*)malloc(i + count * (newlen - oldlen) + 1);
  if (ret == NULL)
    return NULL;
  r = ret;
  while (*str) {
    if (strncmp(str, old, oldlen) == 0) {
      str += oldlen;
      memcpy(r, new, newlen);
      r += newlen;
    } else
      *r++ = *str++;
  }
  *r = '\0';
  return ret;
}

int ReadIni_GetSectionForCount(char* IniFile, int count, char* result) {
  char* p = IniFile;
  for (int i = 0, c = 0, j = 0; i < strlen(IniFile); i++) {
    if (IniFile[i] == '[') {
      c++;
      continue;
    }
    if (c == count) {
      if (IniFile[i] != ']') {
        result[j++] = IniFile[i];
      } else {
        result[j] = '\0';
        return i + 1;  // 运行正常 返回index
      }
    }
  }
  // 程序如果执行到这里，说明是没找到，或者出错
  return -1;
}
int ReadIni_GetValueForName(char* IniFile,
                            char* name,
                            char* section,
                            char* result) {
  char* p = IniFile;
  char buf[500];
  for (int i = 0; ReadIni_GetNameForCount(IniFile, i, section, buf) != 1; i++) {
    //		printf("\n%s\n",buf);
    if (strcmp(buf, name) == 0) {
      int index = ReadIni_GetNameForCount(IniFile, i, section, buf);
      IniFile += index;
      //			printf(IniFile);
      for (; *IniFile != '='; IniFile++)
        ;
      IniFile++;
      int l = 0;
      for (; *IniFile != '\r' && *IniFile != '\n' && *IniFile != '#' &&
             *IniFile != '\0';
           IniFile++) {
        if (*IniFile != ' ' && *IniFile != '\t') {
          result[l++] = *IniFile;
        }
      }
      result[l] = 0;
      // 获取完成，处理插值
      /*	char *Abuf;
              for(int i = 0;i<strlen(result);i++) {
                      if(result[i] == '$') {
                              if(result[i+1] == '(') {
                                      int j = i+2;
                                      char name[500];
                                      int k = 0;
                                      for(;result[j] != ')';j++) {
                                              name[k++] = result[j];
                                      }
                                      name[k] = 0;
                                      char value[500];
                                      if(ReadIni_GetValueForName(p,name,section,value))
         return 1;
                                      //printf("%s\n",p);
                                      //printf("\nV:%s\n",value);
                                      char old[500];
                                      sprintf(old,"$(%s)",name);
                                      Abuf = replace(result,old,value);
                                      i+=strlen(old); // 跳过插值
                                      strcpy(result,Abuf);
                              }
                      }
              }
              */
      return 0;
    }
  }
  return 1;
}
int ReadIni_GetNameForCount(char* IniFile,
                            int count,
                            char* section,
                            char* result) {
  char buf[500];
  int flag = 0, adr;
  for (int i = 1; ReadIni_GetSectionForCount(IniFile, i, buf) != -1; i++) {
    if (strcmp(section, buf) == 0) {
      int index = ReadIni_GetSectionForCount(IniFile, i, buf);
      flag = 1;
      IniFile += index;
      adr = index;
      break;
    }
  }
  if (!flag) {
    return 1;
  }
  //          printf(IniFile);
  for (int i = 0; i < 500; i++) {
    buf[i] = 0;
  }
  for (int i = 0, j = 0; i < strlen(IniFile); i++) {
    if (IniFile[i] == '\r' || IniFile[i] == '\n' || IniFile[i] == ' ' ||
        IniFile[i] == '\t') {
      continue;
    }
    if (IniFile[i] == '[') {
      return 1;
    }
    if (j == count) {
      //			printf("Got It! %d\n",i);
      while (IniFile[i] == '#' || IniFile[i] == ' ' ||
             IniFile[i] == '\t')  // 注释
      {
        for (; (IniFile[i] == ' ' || IniFile[i] == '\t') && i < strlen(IniFile);
             i++)
          ;
        if (IniFile[i] != '#')
          break;
        for (; IniFile[i] != '\r' && IniFile[i] != '\n' && i < strlen(IniFile);
             i++)
          ;
        for (;
             (IniFile[i] == '\r' || IniFile[i] == '\n') && i < strlen(IniFile);
             i++)
          ;
      }
      int l = 0;
      for (int k = i; IniFile[k] != '='; k++) {
        if (IniFile[k] == '\0') {
          return 1;
        }
        if (IniFile[k] != ' ' && IniFile[k] != '\t') {
          result[l++] = IniFile[k];
        }
        //				printf("%d %c\n",k,result[l-1]);
      }
      result[l] = 0;
      //	printf("result = %c\n",result[0]);
      return i + adr;
    }
    if (IniFile[i] == '=') {
      j++;
      for (; IniFile[i] != '\r' && IniFile[i] != '\n'; i++) {
        if (IniFile[i] == '\0' || IniFile[i] == '[') {
          return 1;
        }
      }
    }
  }
  return 1;
}