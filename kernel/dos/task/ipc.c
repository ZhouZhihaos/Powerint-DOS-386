// 进程间通讯
// Copyright (C) zhouzhihao & min0911_ 2022
// 核心代码：zhouzhihao编写
// UPDATE: min0911_ : 增加了一个新的函数 可以用来修改发信人
// BUGFIX: min0911_ :
// GetIPCMessage函数 如果有多个消息 会返回最后一个消息 而不是最近发送的消息

#include <dos.h>
int SendIPCMessage(int to_tid, void *data, unsigned int size, char type) {
  Maskirq(0);
  struct TASK *to_task = GetTask(to_tid), *this_task = current_task();
  int levelold = this_task->level;
  if (to_task->IPC_header.now == MAX_IPC_MESSAGE - 1) {
    ClearMaskIrq(0);
    return -1;
  }
  void *now_data = page_kmalloc(size);
  memcpy(now_data, data, size);
  to_task->IPC_header.data[to_task->IPC_header.now] = now_data;
  to_task->IPC_header.size[to_task->IPC_header.now] = size;
  to_task->IPC_header.from_tid[to_task->IPC_header.now] =
      (this_task->sel / 8) - 103;
  to_task->IPC_header.now++;
  int now = to_task->IPC_header.now;
  ClearMaskIrq(0);
  if (type == synchronous) { // 同步
    ChangeLevel(this_task, 3);
    while (to_task->IPC_header.now != now - 1)
      ;
    ChangeLevel(this_task, levelold);
  }
  return 0;
}
int SendIPCMessageForName(char *tname, void *data, unsigned int size,
                          char type) {
  struct TASK *to_task = GetTaskForName(tname);
  return SendIPCMessage((to_task->sel / 8) - 103, data, size, type);
}
int GetIPCMessage(void *data, int from_tid) {
  Maskirq(0);
  struct TASK *this_task = current_task();
  if (this_task->IPC_header.now == 0) {
    ClearMaskIrq(0);
    return -1;
  }
  for (int i = MAX_IPC_MESSAGE - 1; i >= 0; i--) { // 优先取后发的
    // printk("TID-- %d / %d\n", this_task->IPC_header.from_tid[i],from_tid);
    if (this_task->IPC_header.from_tid[i] == from_tid) {
      // printk("GetIPCMessage--%d now = \n", from_tid,
      // this_task->IPC_header.now);
      memcpy(data, this_task->IPC_header.data[i],
             this_task->IPC_header.size[i]);
      page_kfree((int)this_task->IPC_header.data[i], this_task->IPC_header.size[i]);
      // printk("%08x\n",*(unsigned int*)data);
      if (i == MAX_IPC_MESSAGE - 1) {
        this_task->IPC_header.data[i] = 0;
        this_task->IPC_header.size[i] = 0;
        this_task->IPC_header.from_tid[i] = 0;
      } else {
        for (int j = i; j != MAX_IPC_MESSAGE - 1; j++) {
          this_task->IPC_header.data[j] = this_task->IPC_header.data[j + 1];
          this_task->IPC_header.size[j] = this_task->IPC_header.size[j + 1];
          this_task->IPC_header.from_tid[j] =
              this_task->IPC_header.from_tid[j + 1];
        }
      }
      this_task->IPC_header.now--;
      ClearMaskIrq(0);
      //  sleep(10);
      return 1;
    }
  }
  // printk("%08x\n",*(unsigned int *)data);
  ClearMaskIrq(0);
  // sleep(10);
  return -1;
}
int GetIPCMessageTID(int tid, void *data, int from_tid) {
  Maskirq(0);
  struct TASK *this_task = GetTask(tid);
  if (this_task->IPC_header.now == 0) {
    ClearMaskIrq(0);
    return -1;
  }
  for (int i = MAX_IPC_MESSAGE - 1; i >= 0; i--) { // 优先取后发的
    // printk("TID-- %d / %d\n", this_task->IPC_header.from_tid[i],from_tid);
    if (this_task->IPC_header.from_tid[i] == from_tid) {
      // printk("GetIPCMessage--%d now = \n", from_tid,
      // this_task->IPC_header.now);
      memcpy(data, this_task->IPC_header.data[i],
             this_task->IPC_header.size[i]);
      page_kfree((int)this_task->IPC_header.data[i], this_task->IPC_header.size[i]);
      // printk("%08x\n",*(unsigned int*)data);
      if (i == MAX_IPC_MESSAGE - 1) {
        this_task->IPC_header.data[i] = 0;
        this_task->IPC_header.size[i] = 0;
        this_task->IPC_header.from_tid[i] = 0;
      } else {
        for (int j = i; j != MAX_IPC_MESSAGE - 1; j++) {
          this_task->IPC_header.data[j] = this_task->IPC_header.data[j + 1];
          this_task->IPC_header.size[j] = this_task->IPC_header.size[j + 1];
          this_task->IPC_header.from_tid[j] =
              this_task->IPC_header.from_tid[j + 1];
        }
      }
      this_task->IPC_header.now--;
      ClearMaskIrq(0);
      //  sleep(10);
      return 1;
    }
  }
  // printk("%08x\n",*(unsigned int *)data);
  ClearMaskIrq(0);
  // sleep(10);
  return -1;
}
int GetIPCMessageForName(void *data, char *tname) {
  struct TASK *from_task = GetTaskForName(tname);
  return GetIPCMessage(data, (from_task->sel / 8) - 103);
}
int IPCMessageStatus() { return current_task()->IPC_header.now; }
unsigned int IPCMessageLength(int from_tid) {
  // Maskirq(0);
  struct TASK *this_task = current_task();
  // printk("entry IPCMessageLength\n");
  for (int i = MAX_IPC_MESSAGE - 1; i >= 0; i--) {
    // printk("TID-- %d / %d\n", this_task->IPC_header.from_tid[i], from_tid);
    if (this_task->IPC_header.from_tid[i] == from_tid) {
      // printk("Find %d\n", i);
      // ClearMaskIrq(0);
      return this_task->IPC_header.size[i];
    }
  }
  // printk("Not Find %d\n", from_tid);
  ClearMaskIrq(0);
  return 0xFFFFFFFF;
}
int SendIPCMessageTID(int to_tid,        // 收信人
                      int y_tid,         // 发信人
                      void *data,        // 数据
                      unsigned int size, // 大小
                      char type          // 类型
) {
  // printk("entry SendIPCMessageTID \n");
  // printk("To:\n");
  // printk("TID-- %d\n", to_tid);
  // printk("Task ADDR -- %08x\n", GetTask(to_tid));
  // printk("Task Name -- %s\n", GetTask(to_tid)->name);
  // printk("From:\n");
  // if (y_tid >= 0) {
  //   printk("TID-- %d\n", y_tid);
  //   printk("Task ADDR -- %08x\n", GetTask(y_tid));
  //   printk("Task Name -- %s\n", GetTask(y_tid)->name);
  // } else {
  //   printk("TID-- %d\n", y_tid);
  // }

  Maskirq(0);
  struct TASK *to_task = GetTask(to_tid), *this_task = current_task();
  int levelold = this_task->level;
  if (to_task->IPC_header.now == MAX_IPC_MESSAGE - 1) {
    ClearMaskIrq(0);
    return -1;
  }
  void *now_data = page_kmalloc(size);
  memcpy(now_data, data, size);
  to_task->IPC_header.data[to_task->IPC_header.now] = now_data;
  to_task->IPC_header.size[to_task->IPC_header.now] = size;
  to_task->IPC_header.from_tid[to_task->IPC_header.now] = y_tid;
  to_task->IPC_header.now++;
  int now = to_task->IPC_header.now;
  ClearMaskIrq(0);
  // sleep(10);
  if (type == synchronous) { // 同步
    ChangeLevel(this_task, 3);
    while (to_task->IPC_header.now != now - 1)
      ;
    ChangeLevel(this_task, levelold);
  }
  return 0;
}
bool haveMsg() {
  Maskirq(0);
  // printf("????");
  // 你自己有没有消息
  struct TASK *this_task = current_task();
  // printk("entry IPCMessageLength\n");
  for (int i = MAX_IPC_MESSAGE - 1; i >= 0; i--) {
    // printk("TID-- %d\n", this_task->IPC_header.from_tid[i]);
    if (this_task->IPC_header.from_tid[i] != 0) {
      // printk("Find %d\n", i);
      ClearMaskIrq(0);
      return this_task->IPC_header.size[i];
    }
  }
  //  printk("Not Find %d\n");
  ClearMaskIrq(0);
  return false;
}
int getMsgAll(void *data) {
  // printk("entry getMsgAll\n");
  Maskirq(0);
  struct TASK *this_task = current_task();
  if (this_task->IPC_header.now == 0) {
    ClearMaskIrq(0);
    return -1;
  }
  for (int i = MAX_IPC_MESSAGE - 1; i >= 0; i--) { // 优先取后发的
    // printk("TID-- %d / %d\n", this_task->IPC_header.from_tid[i],from_tid);
    if (this_task->IPC_header.from_tid[i] != 0) {
      // printk("GetIPCMessage--%d now = \n", from_tid,
      // this_task->IPC_header.now);
      //  printk("Get ALL ---> this_task->IPC_header.data[i][0] = %02x\n",
      //         ((unsigned char *)(this_task->IPC_header.data[i]))[0]);
      memcpy(data, this_task->IPC_header.data[i],
             this_task->IPC_header.size[i]);
      page_kfree((int)this_task->IPC_header.data[i], this_task->IPC_header.size[i]);
      // printk("%08x\n",*(unsigned int*)data);
      if (i == MAX_IPC_MESSAGE - 1) {
        this_task->IPC_header.data[i] = 0;
        this_task->IPC_header.size[i] = 0;
        this_task->IPC_header.from_tid[i] = 0;
      } else {
        for (int j = i; j != MAX_IPC_MESSAGE - 1; j++) {
          this_task->IPC_header.data[j] = this_task->IPC_header.data[j + 1];
          this_task->IPC_header.size[j] = this_task->IPC_header.size[j + 1];
          this_task->IPC_header.from_tid[j] =
              this_task->IPC_header.from_tid[j + 1];
        }
      }
      this_task->IPC_header.now--;
      ClearMaskIrq(0);
      //  sleep(10);
      return 1;
    }
  }
  // printk("%08x\n",*(unsigned int *)data);
  ClearMaskIrq(0);
  // sleep(10);
  return -1;
}