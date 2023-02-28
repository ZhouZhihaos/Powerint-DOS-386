/* CodeEdit.c : 代码编辑器*/
#include <gui.h>
#include <stdio.h>
#include <string.h>
#include <syscall.h>

#define T_DrawBox(x, y, w, h, c) Text_Draw_Box((y), (x), (h) + y, (w) + x, (c))
void get_next(char* T, int next[]) {
  int strs_len = strlen(T);
  int i = 0, j = -1;
  next[0] = -1;
  while (i < strs_len) {  //遍历
    if (j == -1 || T[i] == T[j]) {
      i++;
      j++;
      next[i] = j;
    } else {
      j = next[j];
    }
  }
}

int kmp(char* str_main, char* str_branch) {
  int next[100];
  int str_main_len = strlen(str_main);
  int str_branch_len = strlen(str_branch);
  int i, j;
  get_next(str_branch, next);
  i = 0;
  j = 0;
  /*kmp 核心
      当主串i和子串j相等的时候，i++,j++;
      反之 j = next[j];
      当 j==-1 的时候，i++,j++
  */
  while (i < str_main_len && j < str_branch_len) {
    if (str_main[i] == str_branch[j]) {
      i++;
      j++;
    } else {
      j = next[j];
      if (j == -1) {
        j++;
        i++;
      }
    }
  }
  if (j >= str_branch_len) {
    /*此时说明在主串中找到了子串*/
    return i - str_branch_len;
  } else {
    return -1;  //没有找到的话返回-1
  }
}

int ReadIni_GetSectionForCount(char* IniFile, int count, char* result);
int ReadIni_GetValueForName(char* IniFile,
                            char* name,
                            char* section,
                            char* result);
int ReadIni_GetNameForCount(char* IniFile,
                            int count,
                            char* section,
                            char* result);
int ReadIni_GetSectionForCount(char* IniFile, int count, char* result) {
  char* p = IniFile;
  for (int i = 0, c = 0, j = 0; i < strlen(IniFile); i++) {
    if (IniFile[i] == '[') {
      //		printf("%d %d\n",count,c);
      if (i != 0) {
        if (IniFile[i - 1] != '\n') {
          continue;
        }
      }
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
      // printf(IniFile);
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
  //		printf("Is:\n%s\n",IniFile);
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
      if (i != 0) {
        if (IniFile[i - 1] == '\n') {
          //		printf("%d CC %s\n",count,IniFile+i);
          return 1;
        } else {
        }
      } else {
        return 1;
      }
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
        if (IniFile[i] == '\0' ||
            (IniFile[i] == '[' && IniFile[i - 1] == '\n')) {
          return 1;
        }
      }
    }
  }
  return 1;
}
struct Key {
  char key[256];
  int len;
  int color;
};
struct Key_Space {
  Key keySpace[256];
  int Max;
};
struct Key_NoSpace {
  Key keyNoSpace[256];
  int Max;
};
struct Key_NoSpace_Double {
  Key keyNoSpaceDouble[256];
  Key end[256];
  int Max;
};
Key_Space ks;
Key_NoSpace kns;
Key_NoSpace_Double knsd;
struct Camera {
  int y;  // 摄像机高度
  int curser_pos_x, curser_pos_y;
  int index;  //光标指向位置的index
  char* buffer;
  int array_len;
  int len;
};
struct Char {
  int index;  // 在buffer中的index
  unsigned char ch;
};
struct Line {
  int line_flag;  // 是不是回车敲出的行的结尾
  // char line[80];
  Char line[80];
  int len;
  int start_index;  // 行首索引
};
void insert_char(char* str, int pos, char ch, Camera* c) {
  if (strlen(str) + 1 > c->array_len) {
    str = (char*)realloc(c->buffer, c->array_len + 100);
    c->buffer = str;
    c->array_len += 100;
  }
  int i;
  for (i = strlen(str); i >= pos; i--) {
    str[i + 1] = str[i];
  }
  str[pos] = ch;
}
void insert_str(char* str, int pos, Camera* c) {
  for (int i = 0; i < strlen(str); i++) {
    insert_char(c->buffer, pos++, str[i], c);
  }
}
void delete_char(char* str, int pos) {
  int i;
  int l = strlen(str);
  if (l == 0) {
    return;
  }
  for (i = pos; i < strlen(str); i++) {
    str[i] = str[i + 1];
  }
  str[l - 1] = 0;
}
class parse {
  Camera* camera;
  Line l[25];

 public:
  parse(Camera* c) {
    camera = c;
    clean();
  }
  void Set() {
    // 根据camera的y值
    clean();
    int l = 0;
    int sc = 0;
    int f = 0;
    int nl = 0;
    int len = 0;
    int sl = 0;
    int i;
    for (i = 0; i < strlen(camera->buffer) && nl < 25; i++) {
      // printf("%c", camera->buffer[i] == '\n' ? 'n' : camera->buffer[i]);
      if (l == camera->y) {
        // printf("OK1\n");
        if (sc == 0) {
          this->l[nl].start_index = i;
          if (nl != 0) {
          }
        }
        if (camera->buffer[i] == '\n' || sc == 80) {
          //  printf("\nN!!(%02x) sc=%d\n", camera->buffer[i], sc);
          this->l[nl].line_flag = 1;  //在这里设置
          this->l[nl].len = len;
          len = 0;
          sl = 0;
          nl++;
          f = sc == 80 ? 1 : 0;
          sc = 0;
        } else {
          //   printf("Y,sc=%d\n", sc);
          this->l[nl].line[sc++].ch = camera->buffer[i];
          this->l[nl].line[sc - 1].index = i;
          len++;
          if (sc == 80) {
            this->l[nl].len = 80;
            // what the fuck?
            //     printf("\nN!!(%02x) sc=%d\n", camera->buffer[i], sc);
            nl++;
            f = sc == 80 ? 1 : 0;
            sc = 0;
            len = 0;
          }
        }

      } else {
        if (camera->buffer[i] == '\n' || sc == 80) {
          l++;
          f = sc == 80 ? 1 : 0;
          sc = 0;

        } else {
          sc++;
        }
      }
    }
    f = sc == 80 ? 1 : 0;
    sc = 0;
    if (sc == 0) {
      this->l[nl].line_flag = 1;  //在这里设置
      this->l[nl].len = len;
      this->l[nl].start_index = i;
      // this->l[nl - 1].start_index = sl;
      len = 0;
      sl = 0;
    }
  }
  Line* getBuf() { return l; }

 private:
  void clean() {
    for (int i = 0; i < 25; i++) {
      l[i].line_flag = 0;
      l[i].len = 0;
      l[i].start_index = 0;
      for (int j = 0; j < 80; j++) {
        l[i].line[j].index = 0;
        l[i].line[j].ch = 0;
      }
    }
  }
};
class render {
  char* buf;
  parse* p;
  Camera* camera;

 public:
  render(char* buffer, Camera* c, parse* _p) {
    buf = buffer;
    p = _p;
    camera = c;
  }
  void showAll() {
    Line* l = p->getBuf();
    char buf[90];
    int fg = 0;
    int b_color;
    Key s;
    goto_xy(0, 0);
    for (int i = 0; i < 25; i++) {
      goto_xy(0, i);
      for (int j = 0, l1 = 0; j < 80; j++) {
        printf("%c", l[i].line[j].ch == '\0' ? ' ' : l[i].line[j].ch);
        if (!fg) {
          if (l[i].line[j].ch == ' ' || l[i].line[j].ch == '\0') {
            buf[l1] = 0;
            l1 = 0;
            for (int h = 0; h < ks.Max; h++) {
              int ret = kmp(buf, ks.keySpace[h].key);
              if (ret != -1) {
                T_DrawBox(j - ((strlen(buf) - ret)), i, ks.keySpace[h].len, 1,
                          ks.keySpace[h].color + 0x30);
                break;
              }
            }
          } else {
            buf[l1++] = l[i].line[j].ch;
          }
          for (int h = 0; h < kns.Max; h++) {
            if (l[i].line[j].ch == kns.keyNoSpace[h].key[0]) {
              T_DrawBox(j, i, 1, 1, kns.keyNoSpace[h].color + 0x30);
              break;
            }
          }
        } else {
          T_DrawBox(j, i, 1, 1, b_color + 0x30);
        }
        if (!fg) {
          for (int h = 0; h < knsd.Max; h++) {
            if (l[i].line[j].ch == knsd.keyNoSpaceDouble[h].key[0]) {
              T_DrawBox(j, i, 1, 1, knsd.keyNoSpaceDouble[h].color + 0x30);
              fg = !fg;
              buf[0] = 0;
              l1 = 0;
              b_color = knsd.keyNoSpaceDouble[h].color;
              s = knsd.end[h];
            }
          }
        } else {
          if (s.len == 1) {
            if (s.key[0] == l[i].line[j].ch) {
              T_DrawBox(j, i, 1, 1, b_color + 0x30);
              fg = !fg;
              buf[0] = 0;
              l1 = 0;
            }
          }
        }
      }
      buf[0] = 0;
      fg = 0;
    }
    // printf("X=%d,Y=%d\n", camera->curser_pos_x, camera->curser_pos_y);
    goto_xy(camera->curser_pos_x, camera->curser_pos_y);
  }
};
bool Need_Sroll(Line* l) {
  if (l[24].line_flag == 1 || l[24].line[0].ch != '\0') {
    return true;
  }
  return false;
}
int Show_Line_Max(Line* l) {
  int i;
  for (i = 0; i < 25; i++) {
    if (l[i].line[0].ch == '\0' && l[i].line_flag != 1) {
      return i;
    }
  }
  return i;
}
class Note {
  Camera* camera;
  parse* p;

 public:
  int maxLine() {
    int l = 0;
    int sc = 0;
    for (int i = 0; i < strlen(camera->buffer); i++) {
      if (camera->buffer[i] == '\n' || sc == 80) {
        l++;
        sc = 0;
      } else {
        sc++;
      }
    }
    return l;
  }

  Note(Camera* c, parse* _p) {
    camera = c;
    p = _p;
  }
  void Insert(char ch) {
    insert_char(camera->buffer, camera->index, ch, camera);
  }
  void Delete() {
    /* 判断3“0”情况 */
    delete_char(camera->buffer, camera->index);
  }
  /* 上下左右操作 */
  void up() {
    if (camera->y == 0 && camera->curser_pos_y == 0) {
      // 无法上移
      // printf("Can not up.\n");
      return;
    }
    if (camera->curser_pos_y == 0) {
      camera->y--;
    }
    p->Set();
    Line* l = p->getBuf();  // 获取当前行布局
    if (camera->curser_pos_y == 0) {
      if (l[0].len == 0) {
        camera->index = l[0].start_index;
      } else {
        camera->index = l[0].line[l[0].len - 1].index + 1;
      }
      camera->curser_pos_x = l[0].len;
      camera->curser_pos_y = 0;
    } else {
      camera->curser_pos_y--;
      if (l[camera->curser_pos_y].len == 0) {
        camera->index = l[camera->curser_pos_y].start_index;
      } else {
        camera->index = l[camera->curser_pos_y]
                            .line[l[camera->curser_pos_y].len - 1]
                            .index +
                        1;
      }
      camera->curser_pos_x = l[camera->curser_pos_y].len;
    }
  }
  int down() {
    Line* l;
    p->Set();
    l = p->getBuf();
    int ml = maxLine();
    if (camera->curser_pos_y != 24) {
      if (l[camera->curser_pos_y + 1].line[0].ch == '\0' &&
          l[camera->curser_pos_y + 1].line_flag == 0) {
        //	  printf("Can not Down2.\n");
        return 0;
      }
    } else {
      if (ml < (camera->y + camera->curser_pos_y) + 1) {
        // printf("Can not Down1. %d %d
        // %d\n",ml,camera->y,camera->curser_pos_y);
        //	  for(;;);
        return 0;
      }
    }
    //  printf("sure!\n");
    if (camera->curser_pos_y == 24) {
      camera->y++;
    }
    p->Set();
    l = p->getBuf();  // 获取当前行布局
    if (camera->curser_pos_y == 24) {
      if (l[24].len == 0) {
        camera->index = l[24].start_index;
      } else {
        camera->index = l[24].line[l[24].len - 1].index + 1;
      }
      camera->curser_pos_x = l[24].len;
      camera->curser_pos_y = 24;
    } else {
      camera->curser_pos_y++;
      if (l[camera->curser_pos_y].len == 0) {
        camera->index = l[camera->curser_pos_y].start_index;
      } else {
        camera->index = l[camera->curser_pos_y]
                            .line[l[camera->curser_pos_y].len - 1]
                            .index +
                        1;
        //     printf("INDEX=%d\n", camera->index);
      }
      camera->curser_pos_x = l[camera->curser_pos_y].len;
    }
    return 1;
  }
  void left() {
    if (camera->curser_pos_x == 0) {
      up();
      Line* l = p->getBuf();

    } else {
      camera->curser_pos_x--;
      camera->index--;
    }
  }
  void right(int b) {
    // printf("this.\n");
    p->Set();
    Line* l = p->getBuf();
    if ((camera->curser_pos_x == l[camera->curser_pos_y].len && b)) {
      // printf("%d %d   s Y=%d\n", camera->curser_pos_x,
      //        l[camera->curser_pos_y].len, camera->curser_pos_y);
      // system("PAUSE");
      int ret = down();
      // system("PAUSE");

      if (ret) {
        if (b) {
          camera->curser_pos_x = 0;
          if (l[camera->curser_pos_y].len != 0) {
            camera->index = l[camera->curser_pos_y].line[0].index;
          }
        } else {
          camera->curser_pos_x = 1;
          camera->index = l[camera->curser_pos_y].start_index + 1;
        }
      }
      // asm("nop");
    } else {
      if (camera->curser_pos_x == 80) {
        camera->curser_pos_x = 1;
        if (camera->curser_pos_y == 24) {
          camera->y++;
        } else {
          camera->curser_pos_y++;
        }
      } else {
        camera->curser_pos_x++;
      }
      camera->index++;
    }
  }
};
class Editor {
 public:
  char* Main(char* filename) {
    system("cls");
    Camera* c = (Camera*)malloc(sizeof(Camera));
    c->buffer = (char*)malloc(filesize(filename) + 1000);
    char* bf2 = (char*)malloc(filesize(filename) + 1000);
    c->array_len = filesize(filename) + 1000;
    c->len = 0;

    c->y = 0;
    c->curser_pos_x = 0;
    c->curser_pos_y = 0;
    c->index = 0;
    // printf("%d %s\n",c->array_len,filename);
    if (filesize(filename) != -1) {
      api_ReadFile(filename, bf2);
      for (int i = 0, j = 0; i < filesize(filename) + 1000; i++) {
        if (bf2[i] != '\r') {
          c->buffer[j++] = bf2[i];
        }
      }
      char buffe2[100];
      sprintf(buffe2, "del %s", filename);
      system(buffe2);
    }
    mkfile(filename);

    free(bf2);
    parse* prse = new parse(c);
    prse->Set();
    Note* n = new Note(c, prse);
    Line* l = prse->getBuf();
    //  printf("%s\n", c->buffer);
    render* r = new render(c->buffer, c, prse);
    r->showAll();
    int times = 0;
    for (;;) {
      int ch = getch();
      if (ch == 0) {
        continue;
      }
      if (ch == '\n') {
        n->Insert('\n');
        n->down();
        // l = prse->getBuf();
        if (c->curser_pos_x != 0) {
          c->curser_pos_x = 0;
          c->index = l[c->curser_pos_y].line[0].index;  // Holy Fuck
        }

      } else if (ch == '\b') {
        if (c->y + c->curser_pos_x + c->curser_pos_y != 0) {
          n->left();
          n->Delete();
        }
      } else if (ch == '\t') {
        return c->buffer;
      }
      // else if (ch == 224) {
      // 	if (ch == 77) {
      // 		n->right(1);
      // 	} else if (ch == 75) {
      // 		n->left();
      // 	} else if (ch == 72) {
      // 		n->up();
      // 	} else if (ch == 80) {
      // 		n->down();
      // 	}
      else if (ch == -1) {
        n->up();
      } else if (ch == -2) {
        n->down();
      } else if (ch == -3) {
        n->left();
      } else if (ch == -4) {
        n->right(1);
      } else {
        n->Insert(ch);
        n->right(0);
      }
      prse->Set();
      r->showAll();
    }
  }
};
int mLine(char* buffer) {
  int l = 0;
  int sc = 0;
  for (int i = 0; i < strlen(buffer); i++) {
    if (buffer[i] == '\n' || sc == 80) {
      l++;
      sc = 0;
    } else {
      sc++;
    }
  }
  return l;
}
int main(int argc, char** argv) {
  if (IsGuiMode()) {
    printf("warn: this program has bug in gui mode.\n");
    system("pause");
  }
  if (argc == 1) {
    print("Usage:");
    print(" ");
    print(argv[0]);
    print(" ");
    print("<FileName>");
    print("\n");
    return 0;
  }
  printf("Read Config file...");
  /*
   ks.Max = 1;
   strcpy(ks.keySpace[0].key,"asm");
   ks.keySpace[0].color = 0x01;
   ks.keySpace[0].len = strlen(ks.keySpace[0].key);
   kns.keyNoSpace[0].key[0] = '{';
   kns.Max = 1;
   kns.keyNoSpace[0].color = 0x02;
   kns.keyNoSpace[0].len = 1;
   knsd.Max = 1;
   knsd.end[0].key[0] = '\"';
   knsd.keyNoSpaceDouble[0].key[0] = '\"';
   knsd.keyNoSpaceDouble[0].color = 0x03;
   knsd.keyNoSpaceDouble[0].len = 1;*/
  // char p[] =
  //     "[asm_keySpace]\r\nkey1=mov\r\nkey1_color=1\r\nkey2=add\r\nkey2_color="
  //     "1\r\nkey3=jmp\r\nkey3_color=1\r\nkey4=call\r\nkey4_color=1\r\nkey5="
  //     "cmp\r\nkey5_color=1\r\nkey6=je\r\nkey6_color=1\r\nkey7=int\r\nkey7_"
  //     "color=1\r\nkey8=db\r\nkey8_color=1\r\n[asm_keyNoSpace]\r\nkey1=@"
  //     "\r\nkey1_color=5\r\nkey2=0\r\nkey2_color=5\r\nkey3=1\r\nkey3_color="
  //     "5\r\nkey4=2\r\nkey4_color=5\r\nkey5=3\r\nkey5_color=5\r\nkey6=4\r\nkey6_"
  //     "color=5\r\nkey7=5\r\nkey7_color=5\r\nkey8=6\r\nkey8_color=5\r\nkey9="
  //     "7\r\nkey9_color=5\r\nkey10=8\r\nkey10_color=5\r\nkey11=9\r\nkey11_color="
  //     "5\r\n[asm_keyNoSpaceDouble]\r\nkey1=(\r\nkey1_end=)\r\nkey1_color="
  //     "2\r\nkey2=%\r\nkey2_end=,\r\nkey2_color=2\r\nkey3=;\r\nkey3_end="
  //     "AA\r\nkey3_color=3\r\nkey4=[\r\nkey4_end=]\r\nkey4_color=2\r\nkey5='"
  //     "\r\nkey5_end='\r\nkey5_color=2";
  char* p;
  p = (char*)malloc(filesize("tskdrv:\\config\\codeedit.ini") + 1);
  api_ReadFile("tskdrv:\\config\\codeedit.ini", p);
  printf("done\n");
  printf("Parse config...");
  char buf[500] = {0};
  char ext[50] = {0};
  int q = 0;
  for (q = 0; q < strlen(argv[1]); q++) {
    if (argv[1][q] == '.') {
      q++;
      break;
    }
  }
  strcpy(ext, argv[1] + q);
  // printf("Sel %s\n",ext);
  char e[50] = {0};
  int fd = 0;
  char ext_s[500];
  char ext_ns[500];
  char ext_nsd[500];
  sprintf(ext_s, "%s_keySpace", ext);
  sprintf(ext_ns, "%s_keyNoSpace", ext);
  sprintf(ext_nsd, "%s_keyNoSpaceDouble", ext);
  for (int i = 1; ReadIni_GetSectionForCount(p, i, buf) != -1; i++) {
    char name_buf[500];
    // printf("EXT=");
    for (int j = 0; j < strlen(buf); j++) {
      if (buf[j] == '_') {
        break;
      }
      e[j] = buf[j];
    }
    if (strcmp(ext, e) == 0) {
      // printf("Found\n");
      fd = i;
      break;
    }
    for (int j = 0; j < 50; j++) {
      e[j] = 0;
    }
    /*		printf("\nSection=%s\n",buf);

                    for(int j =
       0;ReadIni_GetNameForCount(p,j,buf,name_buf)!=1;j++) {
                            printf("%s=",name_buf);
                            char v_buf[500];
                            ReadIni_GetValueForName(p,name_buf,buf,v_buf);
                            printf("%s\n",v_buf);
                            for(int o = 0;o<500;o++) {
                                    v_buf[o] = 0;
                            }

                    }
                    for(int o = 0;o<500;o++) {
                            name_buf[o] = 0;
                    }
                    printf("End Section.\n");*/
  }
  ks.Max = 0;
  kns.Max = 0;
  knsd.Max = 0;
  if (fd) {
    char name_buf[100];
    char v_buf[500];
    // printf("----------Space----------\n");
    for (int j = 0, l = 0; ReadIni_GetNameForCount(p, j, ext_s, name_buf) != 1;
         j++, l++) {
      if (l == 0) {
        //    printf("Key=");
        ReadIni_GetValueForName(p, name_buf, ext_s, v_buf);
        //    printf("%s\n", v_buf);
        strcpy(ks.keySpace[ks.Max].key, v_buf);
        ks.keySpace[ks.Max].len = strlen(v_buf);
      } else if (l == 1) {
        //   printf("Color=");
        ReadIni_GetValueForName(p, name_buf, ext_s, v_buf);
        //   printf("%s\n", v_buf);
        l = -1;
        ks.keySpace[ks.Max].color = v_buf[0] - 0x30;
        ks.Max++;
      }

      for (int o = 0; o < 500; o++) {
        v_buf[o] = 0;
      }
    }

    //   printf("-------NoSpace------------\n");
    for (int o = 0; o < 500; o++) {
      v_buf[o] = 0;
    }
    for (int j = 0, l = 0; ReadIni_GetNameForCount(p, j, ext_ns, name_buf) != 1;
         j++, l++) {
      if (l == 0) {
        //      printf("Key=");
        ReadIni_GetValueForName(p, name_buf, ext_ns, v_buf);
        //      printf("%s\n", v_buf);
        strcpy(kns.keyNoSpace[kns.Max].key, v_buf);
        kns.keyNoSpace[kns.Max].len = strlen(v_buf);
      } else if (l == 1) {
        //      printf("Color=");
        ReadIni_GetValueForName(p, name_buf, ext_ns, v_buf);
        ///     printf("%s\n", v_buf);
        l = -1;
        kns.keyNoSpace[kns.Max].color = v_buf[0] - 0x30;
        kns.Max++;
      }

      for (int o = 0; o < 500; o++) {
        v_buf[o] = 0;
      }
    }
    //   printf("-------NoSpaceDouble------------\n");
    for (int o = 0; o < 500; o++) {
      v_buf[o] = 0;
    }
    for (int j = 0, l = 0;
         ReadIni_GetNameForCount(p, j, ext_nsd, name_buf) != 1; j++, l++) {
      if (l == 0) {
        //     printf("Key=");
        ReadIni_GetValueForName(p, name_buf, ext_nsd, v_buf);
        //    printf("%s\n", v_buf);
        strcpy(knsd.keyNoSpaceDouble[knsd.Max].key, v_buf);
        knsd.keyNoSpaceDouble[knsd.Max].len = strlen(v_buf);
      } else if (l == 1) {
        //  printf("KeyEnd=");
        ReadIni_GetValueForName(p, name_buf, ext_nsd, v_buf);
        //  printf("%s\n", v_buf);
        strcpy(knsd.end[knsd.Max].key, v_buf);
        knsd.end[knsd.Max].len = strlen(v_buf);
      } else if (l == 2) {
        // printf("Color=");
        ReadIni_GetValueForName(p, name_buf, ext_nsd, v_buf);
        //  printf("%s\n", v_buf);
        l = -1;
        knsd.keyNoSpaceDouble[knsd.Max].color = v_buf[0] - 0x30;
        knsd.Max++;
      }

      for (int o = 0; o < 500; o++) {
        v_buf[o] = 0;
      }
    }
  }
  printf("done\n");
  printf("Powerint DOS Editor v0.1a\n");
  printf("We can help you writing note(code)s in Powerint DOS\n");
  printf("Copyright min0911_ 2022 all right reserved\n");
  printf("Build in %s %s\n", __DATE__, __TIME__);
  system("PAUSE");

  Editor* ed = new Editor();
  system("color 37");
  char* c = ed->Main(argv[1]);
  system("cls");
  char* bb = (char*)malloc(strlen(c) + 1 + mLine(c));
  for (int i = 0, j = 0; i < strlen(c); i++) {
    if (c[i] == '\n') {
      bb[j++] = '\r';
      bb[j++] = '\n';
    } else {
      bb[j++] = c[i];
    }
  }
  Edit_File(argv[1], bb, strlen(c) + mLine(c), 0);
  // char b[50];
  // printf("File:");
  // gets(b);
  // FILE *fp = fopen(b,"wb");
  // for(int i = 0;i<strlen(c);i++) {
  // 	if(c[i] == '\n') {
  // 		fputc('\r',fp);
  // 		fputc('\n',fp);
  // 	} else {
  // 		fputc(c[i],fp);
  // 	}
  // }
  system("color 07");
  return 0;
}