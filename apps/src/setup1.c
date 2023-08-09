#include <string.h>
#include <syscall.h>
#define All_Kernel_files_count 10
#define All_App_files_count 30
#define All_Res_files_count 21
#define Line_X 205
#define Line_Y 186
#define Left_Up 201
#define Left_Down 200
#define Right_Up 187
#define Right_Down 188
#define T_DrawBox(x, y, w, h, c) Text_Draw_Box((y), (x), (h) + y, (w) + x, (c))
void Box(int x, int y, int w, int h) {
  goto_xy(x, y);
  putch(Left_Up);
  w--;
  h--;
  for (int i = 0; i < w; i++) {
    putch(Line_X);
  }
  goto_xy(x, y + 1);
  for (int i = 0; i < h; i++) {
    goto_xy(x, y + 1 + i);
    putch(Line_Y);
  }
  goto_xy(x, y + 1 + h - 1);
  putch(Left_Down);
  for (int i = 0; i < w; i++) {
    putch(Line_X);
  }
  goto_xy(x + w, y);
  putch(Right_Up);
  for (int i = 0; i < h; i++) {
    goto_xy(x + w, y + 1 + i);
    putch(Line_Y);
  }
  goto_xy(x + w, y + 1 + h - 1);
  putch(Right_Down);
}
void Set_Loading(int percentage) {
  T_DrawBox(6, 19, 67, 1, 0x1f);
  float p = ((float)percentage / 100.0) * 67;
  int percent = (int)p;
  T_DrawBox(6, 19, percent, 1, 0x5f);
}
void putSpace(int x, int y, int w, int h) {
  goto_xy(x, y);
  for (int i = 0; i < h; i++) {
    goto_xy(x, y + i);
    for (int j = 0; j < w; j++) {
      putch(' ');
    }
  }
}
int OKCancelMsg(char* msg) {
  putSpace(40 - (strlen(msg) + 15) / 2, 4, strlen(msg) + 20, 8);
  Box(40 - (strlen(msg) + 15) / 2, 4, strlen(msg) + 15, 8);
  goto_xy(40 - (strlen(msg) + 15) / 2 + 1, 4);
  print("Message");
  goto_xy(40 - (strlen(msg) + 15) / 2 + 1, 6);
  print(msg);
  goto_xy(40 - (strlen(msg) + 15) / 2 + 8, 10);
  print("OK");
  goto_xy(40 - (strlen(msg) + 15) / 2 + 15, 10);
  print("CANCEL");
  T_DrawBox(40 - (strlen(msg) + 15) / 2 + 6, 10, 6, 1, 0x4f);
  T_DrawBox(40 - (strlen(msg) + 15) / 2 + 13, 10, 10, 1, 0x0f);
  unsigned int c = 1;
  int res = 1;
  while (c != '\n') {
    c = getch();
    switch (c) {
      case -3:
        T_DrawBox(40 - (strlen(msg) + 15) / 2 + 6, 10, 6, 1, 0x4f);
        T_DrawBox(40 - (strlen(msg) + 15) / 2 + 13, 10, 10, 1, 0x0f);
        res = 1;
        break;
      case -4:
        res = 0;
        T_DrawBox(40 - (strlen(msg) + 15) / 2 + 6, 10, 6, 1, 0x0f);
        T_DrawBox(40 - (strlen(msg) + 15) / 2 + 13, 10, 10, 1, 0x4f);
        break;
      default:
        break;
    }
  }
  putSpace(40 - (strlen(msg) + 15) / 2, 4, strlen(msg) + 15, 8);
  return res;
}
int OKMsg(char* msg) {
  putSpace(40 - (strlen(msg) + 15) / 2, 4, strlen(msg) + 15, 8);
  Box(40 - (strlen(msg) + 15) / 2, 4, strlen(msg) + 15, 8);
  goto_xy(40 - (strlen(msg) + 15) / 2 + 1, 4);
  print("Message");
  goto_xy(40 - (strlen(msg) + 15) / 2 + 1, 6);
  print(msg);
  goto_xy(40 - (strlen(msg) + 15) / 2 + 8, 10);
  print("OK");

  T_DrawBox(40 - (strlen(msg) + 15) / 2 + 6, 10, 6, 1, 0x4f);
  // T_DrawBox(40 - (strlen(msg) + 15) / 2 + 13, 10, 10, 1, 0x0f);
  unsigned int c = 1;
  int res = 1;
  while (c != '\n') {
    c = getch();
  }
  putSpace(40 - (strlen(msg) + 15) / 2, 4, strlen(msg) + 15, 8);
  return res;
}
void setState(char* msg) {
  putSpace(0, 24, 80, 1);
  goto_xy(0, 24);
  print(msg);
  T_DrawBox(0, 24, 80, 1, 0x70);
}
int main() {
  system("cls");
  system("color 1f");
  Box(0, 0, 80, 24);
  T_DrawBox(0, 24, 80, 1, 0x70);
  goto_xy(26, 0);
  print("Powerint DOS 386 Installation");
  T_DrawBox(26, 0, 29, 1, 0x4f);
  setState("Welcome to Powerint DOS 386 Installation");
  goto_xy(5, 17);
  print("Installing...");
  Box(5, 18, 69, 3);
  if (!OKCancelMsg("Do you want to Install Powerint DOS?")) {
    system("color 07");
    system("cls");
    return 0;
  }
  Set_Loading(0);
  setState("Install: Format Disk");
  Box(34, 4, 11, 5);
  goto_xy(35, 5);
  print("Choose fs");
  goto_xy(35, 6);
  print("FAT");
  goto_xy(35, 7);
  print("PFS");
  T_DrawBox(35 ,6, 9, 1, 0x4f);
  T_DrawBox(35 ,7, 9, 1, 0x0f);
  char *fs_choice = malloc(4);
  strcpy(fs_choice, "FAT");
  for (;;) {
    int i = getch();
	if (i == '\n') {
	  break;
	} else if (i == -1) {
      T_DrawBox(35 ,6, 9, 1, 0x4f);
      T_DrawBox(35 ,7, 9, 1, 0x0f);
	  strcpy(fs_choice, "FAT");
	} else if (i == -2) {
      T_DrawBox(35 ,6, 9, 1, 0x0f);
      T_DrawBox(35 ,7, 9, 1, 0x4f);
	  strcpy(fs_choice, "PFS");
	}
  }
  putSpace(34, 4, 11, 5);
  T_DrawBox(34, 4, 11, 5, 0x1f);
  if (!format('C', fs_choice)) {
    OKMsg("Disk Read Error.");
    system("color 07");
    system("cls");
    return 0;
  }
  system("rdrv C:");
  Set_Loading(100);
  int CopyFilesCount = 0;

  setState("Install System --- Copy file: dosldr.bin");
  Set_Loading(0);
  Copy("A:\\dosldr.bin", "dosldr.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- Copy file: kernel.bin");
  Copy("A:\\kernel.bin", "kernel.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- Copy file: autoexec.bat");
  Copy("A:\\autoexec.bat", "autoexec.bat");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- Copy file: creus.bin");
  Copy("A:\\creus.bin", "creus.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- Copy file: boot.bin");
  Copy("A:\\boot.bin", "boot.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- Copy file: boot32.bin");
  Copy("A:\\boot32.bin", "boot32.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- Copy file: boot_pfs.bin");
  Copy("A:\\boot_pfs.bin", "boot_pfs.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- mkdir other");
  mkdir("other");
  setState("Install System --- Copy file: other/font.bin");
  Copy("A:\\other\\font.bin", "other/font.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- Copy file: other/hzk16");
  Copy("A:\\OTHER\\HZK16", "other/hzk16");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  setState("Install System --- Copy file: other/uname.bin");
  Copy("A:\\other\\uname.bin", "other/uname.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  if (OKCancelMsg("System has installed. Install apps?")) {
    Box(0, 0, 80, 24);
    goto_xy(26, 0);
    print("Powerint DOS 386 Installation");
    T_DrawBox(26, 0, 29, 1, 0x4f);
    OKMsg("Please Insert Disk2.");
    system("rdrv A");
    system("C:");
    CopyFilesCount = 0;
    Set_Loading(0);
    setState("Copy Apps --- mkdir bin");
    mkdir("bin");

    // 1
    setState("Copy Apps --- Copy file: bin/editor.bin");
    Copy("A:\\EDITOR.BIN", "bin/editor.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    setState("Copy Apps --- Copy file: bin/editor.mst");
    Copy("A:\\EDITOR.MST", "bin/editor.mst");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    // 2
    setState("Copy Apps --- Copy file: bin/bitz.bin");
    Copy("A:\\BITZ.BIN", "bin/bitz.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    // 3
    setState("Copy Apps --- Copy file: bin/nask.bin");
    Copy("A:\\NASK.BIN", "bin/nask.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
                  setState("Copy Apps --- Copy file: bin/obj2bim.bin");
    Copy("A:\\OBJ2BIM.BIN", "bin/obj2bim.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
                  setState("Copy Apps --- Copy file: bin/bim2hrb.bin");
    Copy("A:\\BIM2HRB.BIN", "bin/bim2hrb.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
                  setState("Copy Apps --- Copy file: bin/bim2hrb.bin");
    Copy("A:\\PLIB.LIB", "plib.lib");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
                  setState("Copy Apps --- Copy file: make.rul");
    Copy("A:\\MAKE.RUL", "make.rul");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    setState("Copy Apps --- Copy file: bin/Maze.bin");
    Copy("A:\\MAZE.BIN", "bin/Maze.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    // 4
    setState("Copy Apps --- Copy file: bin/cale.bin");
    Copy("A:\\CALE.BIN", "bin/cale.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 6
    setState("Copy Apps --- Copy file: bin/copy.bin");
    Copy("A:\\COPY.BIN", "bin/copy.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 7
    setState("Copy Apps --- Copy file: bin/calc.bin");
    Copy("A:\\CALC.BIN", "bin/calc.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    setState("Copy Apps --- Copy file: bin/clock.bin");
    Copy("A:\\CLOCK.BIN", "bin/clock.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    setState("Copy Apps --- Copy file: bin/aigobang.bin");
    Copy("A:\\AIGOBANG.BIN", "bin/aigobang.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 9
    setState("Copy Apps --- Copy file: bin/snake.bin");
    Copy("A:\\SNAKE.BIN", "bin/snake.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 10
    setState("Copy Apps --- Copy file: bin/cgobang.bin");
    Copy("A:\\CGOBANG.BIN", "bin/cgobang.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 12
    setState("Copy Apps --- Copy file: bin/pwsh.bin");
    Copy("A:\\pwsh.BIN", "bin/pwsh.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 15
    setState("Copy Apps --- Copy file: bin/pfn.bin");
    Copy("A:\\PFN.BIN", "bin/pfn.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 16
    setState("Copy Apps --- Copy file: bin/RandNum.bin");
    Copy("A:\\RANDNUM.BIN", "bin/RandNum.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 17
    setState("Copy Apps --- Copy file: bin/sort.bin");
    Copy("A:\\SORT.BIN", "bin/sort.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 18
    setState("Copy Apps --- Copy file: bin/cc.bin");
    Copy("A:\\CC.BIN", "bin/cc.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    setState("Copy Apps --- Copy file: bin/hanzi.txt");
    Copy("A:\\HANZI.TXT", "bin/hanzi.txt");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    setState("Copy Apps --- Copy file: bin/bf.bin");
    Copy("A:\\BF.BIN", "bin/bf.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 20
    setState("Copy Apps --- Copy file: bin/basic.bin");
    Copy("A:\\basic.bin", "bin/basic.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 21
    system("C:");
    setState("Copy Apps --- Copy file: bin/lua.bin");
    Copy("A:\\lua.bin", "bin/lua.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 21
    OKMsg("Please Insert Disk3.");
    system("rdrv A");
    system("C:");
    setState("Copy Apps --- Copy file: bin/luac.bin");
    Copy("A:\\luac.bin", "bin/luac.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 21
    setState("Copy Apps --- Copy file: bin/ttf.bin");
    Copy("A:\\ttf.bin", "bin/ttf.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 21
    setState("Copy Apps --- Copy file: bin/font.ttf");
    Copy("A:\\font.ttf", "bin/font.ttf");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 21
    setState("Copy Apps --- Copy file: bin/c4.bin");
    Copy("A:\\c4.bin", "bin/c4.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    setState("Copy Apps --- Copy file: bin/image.bin");
    Copy("A:\\image.bin", "bin/image.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    if (OKCancelMsg("Apps have installed. Install resources?")) {
      CopyFilesCount = 0;
      Box(0, 0, 80, 24);
      goto_xy(26, 0);
      print("Powerint DOS 386 Installation");
      T_DrawBox(26, 0, 29, 1, 0x4f);
      OKMsg("Please Insert res disk.");
      system("rdrv A");
      system("C:");
      Set_Loading(0);
      setState("Copy Apps --- mkdir res");
      mkdir("res");

      setState("Copy Apps --- Copy file: res/asm.lua");
      Copy("A:\\asm.lua", "res/asm.lua");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/bainian.lua");
      Copy("A:\\bainian.lua", "res/bainian.lua");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/fib.lua");
      Copy("A:\\fib.lua", "res/fib.lua");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/gobang.lua");
      Copy("A:\\gobang.lua", "res/gobang.lua");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/guess.lua");
      Copy("A:\\guess.lua", "res/guess.lua");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/student.lua");
      Copy("A:\\studnet.lua", "res/student.lua");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/desktop.jpg");
      Copy("A:\\desktop.jpg", "res/desktop.jpg");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/load.jpg");
      Copy("A:\\load.jpg", "res/load.jpg");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/huaji.jpg");
      Copy("A:\\huaji.jpg", "res/huaji.jpg");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/yee.bmp");
      Copy("A:\\yee.bmp", "res/yee.bmp");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/shot.png");
      Copy("A:\\shot.png", "res/shot.png");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/shot1.png");
      Copy("A:\\shot1.png", "res/shot1.png");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/bf.txt");
      Copy("A:\\bf.txt", "res/bf.txt");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/chinese.txt");
      Copy("A:\\chinese.txt", "res/chinese.txt");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/c4.c");
      Copy("A:\\c4.c", "res/c4.c");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/hello.nas");
      Copy("A:\\hello.nas", "res/hello.nas");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/io.bas");
      Copy("A:\\io.bas", "res/io.bas");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/test.htm");
      Copy("A:\\test.htm", "res/test.htm");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/test1.htm");
      Copy("A:\\test1.htm", "res/test1.htm");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/coin.wav");
      Copy("A:\\coin.wav", "res/coin.wav");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
      setState("Copy Apps --- Copy file: res/test.wav");
      Copy("A:\\test.wav", "res/test.wav");
      CopyFilesCount++;
      Set_Loading(
          (int)((float)((float)(CopyFilesCount) / (float)All_Res_files_count) *
                100.0));
    }
  }
  Set_Loading(0);
  setState("Config --- Create env.cfg");
  mkfile("env.cfg");
  Set_Loading(50);
  setState("Config --- Write env.cfg");
  Edit_File("env.cfg", "\"path\" = \"C:\\bin;C:;\"", 22, 0);
  Set_Loading(100);
  OKMsg("Press Enter to Reboot Your computer.");
  system("reboot");
  for (;;)
    ;
}