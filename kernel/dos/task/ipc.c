// 进程间通讯
// Copyright (C) zhouzhihao & min0911_ 2022
// 核心代码：zhouzhihao编写
// UPDATE: min0911_ : 增加了一个新的函数 可以用来修改发信人
// BUGFIX: min0911_ :
// GetIPCMessage函数 如果有多个消息 会返回最后一个消息 而不是最近发送的消息

#include <dos.h>
int send_ipc_message(int to_tid, void *data, unsigned int size, char type) {
  irq_mask_set(0);
  struct TASK *to_task = get_task(to_tid), *this_task = current_task();
  int levelold = this_task->level;
  if (to_task->IPC_header.now == MAX_IPC_MESSAGE - 1) {
    irq_mask_clear(0);
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
  irq_mask_clear(0);
  if (type == synchronous) { // 同步
    change_level(this_task, 3);
    while (to_task->IPC_header.now != now - 1)
      ;
    change_level(this_task, levelold);
  }
  return 0;
}
int send_ipc_message_by_name(char *tname, void *data, unsigned int size,
                          char type) {
  struct TASK *to_task = get_task_by_name(tname);
  return send_ipc_message((to_task->sel / 8) - 103, data, size, type);
}
int get_ipc_message(void *data, int from_tid) {
  irq_mask_set(0);
  struct TASK *this_task = current_task();
  if (this_task->IPC_header.now == 0) {
    irq_mask_clear(0);
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
      irq_mask_clear(0);
      //  sleep(10);
      return 1;
    }
  }
  // printk("%08x\n",*(unsigned int *)data);
  irq_mask_clear(0);
  // sleep(10);
  return -1;
}
int GetIPCMessageTID(int tid, void *data, int from_tid) {
  irq_mask_set(0);
  struct TASK *this_task = get_task(tid);
  if (this_task->IPC_header.now == 0) {
    irq_mask_clear(0);
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
      irq_mask_clear(0);
      //  sleep(10);
      return 1;
    }
  }
  // printk("%08x\n",*(unsigned int *)data);
  irq_mask_clear(0);
  // sleep(10);
  return -1;
}
int get_ipc_message_by_name(void *data, char *tname) {
  struct TASK *from_task = get_task_by_name(tname);
  return get_ipc_message(data, (from_task->sel / 8) - 103);
}
int ipc_message_status() { return current_task()->IPC_header.now; }
unsigned int ipc_message_len(int from_tid) {
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
  irq_mask_clear(0);
  return 0xFFFFFFFF;
}
int send_ipc_message_by_tid(int to_tid,        // 收信人
                      int y_tid,         // 发信人
                      void *data,        // 数据
                      unsigned int size, // 大小
                      char type          // 类型
) {
  irq_mask_set(0);
  struct TASK *to_task = get_task(to_tid), *this_task = current_task();
  int levelold = this_task->level;
  if (to_task->IPC_header.now == MAX_IPC_MESSAGE - 1) {
    irq_mask_clear(0);
    return -1;
  }
  void *now_data = page_kmalloc(size);
  memcpy(now_data, data, size);
  to_task->IPC_header.data[to_task->IPC_header.now] = now_data;
  to_task->IPC_header.size[to_task->IPC_header.now] = size;
  to_task->IPC_header.from_tid[to_task->IPC_header.now] = y_tid;
  to_task->IPC_header.now++;
  int now = to_task->IPC_header.now;
  irq_mask_clear(0);
  // sleep(10);
  if (type == synchronous) { // 同步
    change_level(this_task, 3);
    while (to_task->IPC_header.now != now - 1)
      ;
    change_level(this_task, levelold);
  }
  return 0;
}
bool have_msg() {
  irq_mask_set(0);
  // printf("????");
  // 你自己有没有消息
  struct TASK *this_task = current_task();
  // printk("entry IPCMessageLength\n");
  for (int i = MAX_IPC_MESSAGE - 1; i >= 0; i--) {
    // printk("TID-- %d\n", this_task->IPC_header.from_tid[i]);
    if (this_task->IPC_header.from_tid[i] != 0) {
      // printk("Find %d\n", i);
      irq_mask_clear(0);
      return this_task->IPC_header.size[i];
    }
  }
  //  printk("Not Find %d\n");
  irq_mask_clear(0);
  return false;
}
int get_msg_all(void *data) {
  // printk("entry getMsgAll\n");
  irq_mask_set(0);
  struct TASK *this_task = current_task();
  if (this_task->IPC_header.now == 0) {
    irq_mask_clear(0);
    return -1;
  }
  for (int i = MAX_IPC_MESSAGE - 1; i >= 0; i--) { // 优先取后发的
    if (this_task->IPC_header.from_tid[i] != 0) {
      memcpy(data, this_task->IPC_header.data[i],
             this_task->IPC_header.size[i]);
      page_kfree((int)this_task->IPC_header.data[i], this_task->IPC_header.size[i]);
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
      irq_mask_clear(0);
      //  sleep(10);
      return 1;
    }
  }
  // printk("%08x\n",*(unsigned int *)data);
  irq_mask_clear(0);
  // sleep(10);
  return -1;
}