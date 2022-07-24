// Powerint DOS 386
// Copyright (C) 2021-2022 zhouzhihao & min0911
#include <dos.h>
int running_mode = POWERINTDOS; // 运行模式
char *line, *path;
char *font, *ascfont, *hzkfont;
void shell(void)
{
  /*到这里 系统的初始化才真正结束*/
  font = "A:\\other\\font.bin";
  ascfont = fopen("A:\\other\\font.bin");
  hzkfont = fopen("A:\\other\\hzk16");
  clear();
  path = (char *)page_malloc(256);
  clean(path, 256);
  strcpy(path, "");
  line = (char *)page_malloc(1024);
  extern int autoexec;
  if (autoexec)
  {
    goto whilef;
  }
  else
  {
    autoexec = 1;
  }
  SwitchToText8025_BIOS();
  Set_Font(font);
  if (*(unsigned char *)(ADR_DISKIMG) != 0xeb) //校验软盘数据
  {
    //正常的Powerint DOS启动盘的第一个字节是0xeb
    //所以如果不是0xeb，说明是一个非法的Powerint
    // DOS启动盘（可能读取出差错了）
    clear();
    printf("[Read Err]This floppy disk first byte is not 0xeb\n");
    for (;;)
      ;
  }
#ifndef __DEBUG__
  putchar(0);
  char *user = fopen("A:\\USER.INI");
  char *password = fopen("A:\\PASSWORD.INI");
  if (user == 0 || password == 0)
  {
    clear();
    if (fopen("A:\\CREUS.bin") == 0)
    {
      printf("System Start Error: No CREUS.bin\n");
      for (;;)
        ;
    }
    command_run("A:\\CREUS.bin");
  }
  else
  {
    char inpass[32];
    char inusr[32];
    printf("Login:\n");
  rei:
    printf("User:");

    input(inusr, 32);
    printf("Password:");

    input(inpass, 32);
    if (strcmp(user, inusr) != 0 || strcmp(password, inpass) != 0)
    {
      sleep(100);
      printf("User or Password Error\n");
      clean(inusr, 32);
      clean(inpass, 32);
      goto rei;
    }
  }
  printf("----Login Or register Successful done!----\n");
#endif
  if (fopen("autoexec.bat") == 0)
  {
    printf("Boot Warning:No AUTOEXEC.BAT in Drive %c\n", drive);
  }
  else
  {
    run_bat("autoexec.bat");
  }
whilef:
  while (1)
  {
    printchar(drive);
    print(":\\");
    print(path);
    print(">");
    clean(line, 1024);
    input(line, 1024);
    command_run(line);
  }
}

void task_sr1()
{
  // 提供安全结束进程

  extern int tasknum;
  while (1)
  {
  re:
    for (int i = 1; i != tasknum + 1; i++)
    {
      struct TASK *task = GetTask(i);
      if (task->running == 0)
      { // 进程没有运行
        // printf("Has no running task\n");
        if (task->is_child == 0) // 不是一个线程
        {
          asm volatile("cli");
          for (int j = 1; j != tasknum + 1; j++)
          {
            struct TASK *t1 = GetTask(j);
            if (t1->is_child)
            {
              if (t1->thread.father == task)
              {
                __SubTask(t1);
              }
            }
          }
          __SubTask(task);
          asm volatile("sti");
          goto re;
        }
        __SubTask(task);
        goto re;
      }
    }
  }
}
void task_sr2()
{
  while (1)
  {
    if (IPCMessageStatus() != 0)
    {
      int tid = NowTask()->IPC_header.from_tid[0];
      int length = IPCMessageLength(tid);
      printf("Task_sr2: Get IPC Message. From:Task ID %d\n", tid);
      void *data = page_malloc(length);
      GetIPCMessage(data, tid);
      printf("LENGTH:%d DATA:%s\n", length, data);
    }
  }
}
