/*Edit.c : 文本编辑器（OLD）*/
#include <string.h>
#include <syscall.h>

struct List {
  int val;
  struct List* next;
};
void AddVal(int val, struct List* Obj) {
  while (Obj->next != NULL) {
    Obj = Obj->next;
  }
  Obj->next = (struct List*)malloc(sizeof(struct List));
  Obj = Obj->next;
  Obj->next = NULL;
  Obj->val = val;
}
struct List* FindForCount(int count, struct List* Obj) {
  for (int i = 0;; i++) {
    if (Obj == NULL) {
      // not found
      return NULL;  // 找不到
    }
    if (i == count) {
      return Obj;
    }
    Obj = Obj->next;
  }
}
void DeleteVal(int count, struct List* Obj) {
  if (FindForCount(count, Obj) == NULL) {
    // Not found!
    return;
  }
  if (count == 0) {
    // 你删你麻痹头结点
    return;
  }
  struct List* Will_Free = FindForCount(count, Obj);
  if (FindForCount(count, Obj)->next == NULL) {
    // 是尾节点
    FindForCount(count - 1, Obj)->next = NULL;
  } else {
    FindForCount(count - 1, Obj)->next = FindForCount(count + 1, Obj);
  }
  api_free(Will_Free, sizeof(struct List));
}
void InsertVal(int count, int val, struct List* Obj) {
  if (FindForCount(count, Obj) == NULL) {
    // Not found!
    return;
  }
  if (count == 0) {
    // 你删你麻痹头结点
    return;
  }

  if (FindForCount(count, Obj)->next == NULL) {
    // 是尾节点
    AddVal(val, Obj);
  } else {
    struct List* Will = (struct List*)malloc(sizeof(struct List));
    Will->val = val;
    Will->next = FindForCount(count, Obj)->next;
    FindForCount(count, Obj)->next = Will;
  }
}
struct List* NewList() {
  struct List* Obj = (struct List*)malloc(sizeof(struct List));
  Obj->val = 0x123456;  // 头结点数据不可用
  Obj->next = NULL;
  return Obj;
}
void Change(int count, struct List* Obj, int val) {
  if (FindForCount(count + 1, Obj) != NULL) {
    FindForCount(count + 1, Obj)->val = val;
  } else {
    AddVal(val, Obj);
  }
}
//获取尾节点的count
int GetLastCount(struct List* Obj) {
  int count = 0;
  while (Obj->next != NULL) {
    Obj = Obj->next;
    count++;
  }
  return count;
}
int get_x() {
  int xy = get_xy();
  short x = (short)(xy >> 16);
  return (int)(x);
}
int get_y() {
  int xy = get_xy();
  short y = (short)xy;
  return (int)(y);
}
struct List* MainNode;
char mp[25] = {0};
int ct = 0;
void loadFile(char* buf, int len) {
  for (int i = 0; i < len; i++) {
    int ch = buf[i];
    if (get_x() == 80) {
      ct++;
    }
    if (ch == '\n') {
      AddVal('\r', MainNode);
      AddVal(ch, MainNode);
      putch('\n');
      mp[ct]++;
      ct++;
    } else if (ch == '\b') {
      if (get_x() == 0) {
        if (get_y() > 0) {
          ct--;
          goto_xy(mp[ct], get_y() - 1);
        } else {
          continue;
        }
      }
      mp[ct]--;
      DeleteVal(GetLastCount(MainNode), MainNode);
      putch('\b');
    } else if (ch == '\t') {
      continue;
    } else if (ch == '\r') {
      continue;
    } else {
      AddVal(ch, MainNode);
      putch(ch);
      mp[ct]++;
    }
  }
}
int main(int argc, char** argv) {
  // 初始化操作
  if (argc == 1) {
    print("Usage:");
    print(" ");
    print(argv[0]);
    print(" ");
    print("<FileName>");
    print("\n");
    return 0;
  }
  system("cls");
  MainNode = NewList();
  if (filesize(argv[1]) != -1) {
    char* buf = (char*)malloc(filesize(argv[1]));
    api_ReadFile(argv[1], buf);
    loadFile(buf, filesize(argv[1]));
    api_free(buf, filesize(argv[1]));
  }
  // else
  // {
  //     print("File not found!");
  // }
  while (1) {
    int ch = getch();
    if (get_x() == 80) {
      ct++;
    }
    if (ch == '\n') {
      AddVal('\r', MainNode);
      AddVal(ch, MainNode);
      putch('\n');
      mp[ct]++;
      ct++;
    } else if (ch == '\b') {
      if (get_x() == 0) {
        if (get_y() > 0) {
          ct--;
          goto_xy(mp[ct], get_y() - 1);
          DeleteVal(GetLastCount(MainNode), MainNode);
        } else {
          continue;
        }
      }
      mp[ct]--;
      DeleteVal(GetLastCount(MainNode), MainNode);
      putch('\b');
    } else if (ch == '\t') {
      //结束
      system("cls");
      //打印列表
      char* p = malloc(GetLastCount(MainNode) + 1);
      int len = 0;
      for (int i = 1; FindForCount(i, MainNode) != NULL; i++) {
        p[i - 1] = (FindForCount(i, MainNode)->val);
        len++;
      }
      if (filesize(argv[1]) == -1) {
        mkfile(argv[1]);
      }
      Edit_File(argv[1], p, len, 0);
      return 0;
    } else {
      AddVal(ch, MainNode);
      putch(ch);
      mp[ct]++;
    }
  }
}