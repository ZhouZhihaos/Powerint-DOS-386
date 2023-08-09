#include <stdio.h>
#include <syscall.h>
#define SYSVERSION 1 << 1
#define SYSNAME 1 << 2
#define CREATETOR 1 << 3
#define SHOWCOPYRIGHT 1 << 4

void sysver() {
  printf("system version:%08x ---(API Version).\n",Get_System_Version());
}
void sysname() {
  printf("Powerint_DOS_386_KRNL(Kernel file in BOOTDRV:\\kernel.bin)\n");
}
void show_creator() {
  printf("This system made by zhouzhihao & min0911_.The system is free, if you pay to get it, please report the person who sold it to you to us.\n");
}
void show_copy() {
  printf("Copyright (C) 2022-2023 zhouzhihao & min0911_\nAll right reserved\n");
}
void start_tips() {
  printf("P.D.O.S -CHECK: MODE GUI?=>");
  int color = get_cons_color();
  set_cons_color(0x0a);
  printf("%s\n", "false");
  set_cons_color(color);
}
int main(int argc,char **argv) {
  int flag = 0;
  for(int i = 1;i<argc;i++) {
    if(argv[i][0] != '-') {
      printf("arg parse error.\n");
      return 1;
    }
    switch(argv[i][1]) {
      case 'a':
        flag |= SYSVERSION;
        flag |= SYSNAME;
        flag |= SHOWCOPYRIGHT;
        flag |= CREATETOR;
        break;
      case 'v':
        flag |= SYSVERSION;
        break;
      case 'n':
        flag |= SYSNAME;
        break;
      case 'c':
        flag |= CREATETOR;
        break;
      case 'r':
        flag |= SHOWCOPYRIGHT;
        break;
      default:
        printf("unknow arg.\n");
        return 1;
    }
  }
  start_tips();
  if(flag & SHOWCOPYRIGHT) {
    show_copy();
  }
  if(flag & CREATETOR) {
    show_creator();
  }
  if(flag & SYSNAME) {
    sysname();
  }
  if(flag &SYSVERSION) {
    sysver();
  }
  return 0;
}