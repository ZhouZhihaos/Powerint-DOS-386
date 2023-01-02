#include <string.h>
#include <syscall.h>
#define All_Kernel_files_count 7
#define All_App_files_count 25
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
  // SwitchToText8025_BIOS();
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
  if (format('C')) {
    OKMsg("Disk Read Error.");
    system("color 07");
    system("cls");
    return 0;
  }
  system("C:");
  Set_Loading(100);
  int CopyFilesCount = 0;

  setState("Install System --- Copy file: dosldr.bin");
  Set_Loading(0);
  Copy("A:\\dosldr.bin", "dosldr.bin");
  CopyFilesCount++;
  Set_Loading(
      (int)((float)((float)(CopyFilesCount) / (float)All_Kernel_files_count) *
            100.0));
  mkdir("config");
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
  setState("Install System --- mkdir other");
  mkdir("OTHER");
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
  if (OKCancelMsg("System is installed,do you Want to copy apps and config?")) {
    Box(0, 0, 80, 24);
    goto_xy(26, 0);
    print("Powerint DOS 386 Installation");
    T_DrawBox(26, 0, 29, 1, 0x4f);
    CopyFilesCount = 0;
    Set_Loading(0);
    setState("Copy Apps --- mkdir bin");
    mkdir("bin");

    // 1
    setState("Copy Apps --- Copy file: bin/editor.bin");
    Copy("A:\\OTHER\\EDITOR.BIN", "bin/editor.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    setState("Copy Apps --- Copy file: bin/codeedit.bin");
    Copy("A:\\OTHER\\CODEEDIT.BIN", "bin/codeedit.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    // 2
    setState("Copy Apps --- Copy file: bin/bitz.bin");
    Copy("A:\\OTHER\\BITZ.BIN", "bin/bitz.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));
    // 3
    setState("Copy Apps --- Copy file: bin/winver.bin");
    Copy("A:\\WINVER.BIN", "bin/winver.bin");
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
    setState("Copy Apps --- Copy file: bin/uicalc.bin");
    Copy("A:\\UICALC.BIN", "bin/uicalc.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 5
    setState("Copy Apps --- Copy file: bin/cale.bin");
    Copy("A:\\OTHER\\CALE.BIN", "bin/cale.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 6
    setState("Copy Apps --- Copy file: bin/copy.bin");
    Copy("A:\\OTHER\\COPY.BIN", "bin/copy.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 7
    setState("Copy Apps --- Copy file: bin/calc.bin");
    Copy("A:\\OTHER\\CALC.BIN", "bin/calc.bin");
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
    OKMsg("Please Insert Disk2.");
    system("A:");
    system("C:");
    setState("Copy Apps --- Copy file: bin/uigobang.bin");
    Copy("A:\\UIGOBANG.BIN", "bin/uigobang.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 8
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
    setState("Copy Apps --- Copy file: bin/gobang.bin");
    Copy("A:\\GOBANG.BIN", "bin/gobang.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 11
    setState("Copy Apps --- Copy file: bin/cgobang.bin");
    Copy("A:\\CGOBANG.BIN", "bin/cgobang.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 12
    setState("Copy Apps --- Copy file: bin/game.bin");
    Copy("A:\\GAME.BIN", "bin/game.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 13
    setState("Copy Apps --- Copy file: bin/login.bin");
    Copy("A:\\LOGIN.BIN", "bin/login.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 14
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
    setState("Copy Apps --- Copy file: bin/student.bin");
    Copy("A:\\STUDENT.BIN", "bin/student.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 19
    setState("Copy Apps --- Copy file: bin/bf.bin");
    Copy("A:\\BF.BIN", "bin/bf.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 20
    setState("Copy Apps --- Copy file: bin/bf.txt");
    Copy("A:\\BF.TXT", "bin/bf.txt");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 21
	setState("Copy Apps --- Copy file: bin/basic.bin");
    Copy("A:\\basic.bin", "bin/basic.bin");
    CopyFilesCount++;
    Set_Loading(
        (int)((float)((float)(CopyFilesCount) / (float)All_App_files_count) *
              100.0));  // 21
    Set_Loading(0);
    setState("Config --- mkdir path.sys");
    mkfile("path.sys");
    Set_Loading(50);
    setState("Config --- Write path.sys");
    Edit_File("path.sys", "C:\\BIN;C:\\;", 12, 0);
    Set_Loading(100);
    OKMsg("Done.Press Enter to Reboot Your computer,then config");
    mkfile("config.bat");
    Edit_File("config.bat",
              "ECHO Insert Boot floppy disk.\r\nA:\r\nC:\r\ncopy "
              "A:\\CONFIG\\CODEEDIT.INI C:\\CONFIG\\CODEEDIT.INI",
              90, 0);
    system("reboot");
    for (;;)
      ;
  } else {
    system("cls");
    system("color 07");
    print("Press Any key to reboot your computer...");
    getch();
    system("reboot");
    for (;;)
      ;
  }
  // sleep(500);
  //  Set_Loading(5);
  //  sleep(500);
  //  Set_Loading(25);
  //  sleep(500);
  //  Set_Loading(35);
  //  sleep(500);
  //  Set_Loading(55);
  //  sleep(500);
  //  Set_Loading(75);
  //  sleep(500);
  //  Set_Loading(85);
  //  sleep(500);
  //  Set_Loading(100);
  for (;;)
    ;
}