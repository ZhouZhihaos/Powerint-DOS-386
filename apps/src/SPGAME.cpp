#include <stdio.h>
#include <syscall.h>

//char* BitMap[] = {"01110", "10001", "10001", "10001", "01110"};

int main() {
  SwitchTo320X200X256_BIOS();
  Draw_Str(0,0,"320x200x256 graphic mode.",0x2);
  for (;;)
    ;
}