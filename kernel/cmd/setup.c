#include <dos.h>
void setup(void) {
  command_run("cls");
  command_run("A:");
  printf("Format The Hard Disk.\n");
  command_run("format C");
  printf("Now Copy files\n");
  command_run("C:");
  Copy("A:\\dosldr.bin", "dosldr.bin");
  Copy("A:\\kernel.bin", "kernel.bin");
  Copy("A:\\autoexec.bat", "autoexec.bat");
  command_run("mkdir other");
  Copy("A:\\other\\font.bin", "other\\font.bin");
  printf("OK.\n");
  command_run("C:");
  return;
}
