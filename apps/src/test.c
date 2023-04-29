#include <stdio.h>
#include <syscall.h>

int main() {
  start_keyboard_message();
  while (1) {
    if(key_press_status()) {
      printf("PRESS:%02x\n",get_key_press());
    }
    if(key_up_status()) {
      printf("UP:%02x\n",get_key_up());
    }
  }
}