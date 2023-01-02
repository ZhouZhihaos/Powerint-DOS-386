#include <stdio.h>
#include <string.h>
#include <syscall.h>
struct Drivers_callArg {
  int func_num;
  void* arg;
  int tid;
};
int DriversArgGet(void* arg) {
  char data[4];
  data[0] = PhyMemGetByte(arg);
  data[1] = PhyMemGetByte(arg + 1);
  data[2] = PhyMemGetByte(arg + 2);
  data[3] = PhyMemGetByte(arg + 3);
  return *(int*)data;
}
int main() {
  printf("Driver ---> Start\n");
  while (1) {
    if (haveMsg()) {
      struct Drivers_callArg arg;
      GetMessageAll(&arg);
      printf("Driver ---> Call %08x\n", arg.func_num);
      printf("Driver ---> Arg %08x\n", DriversArgGet(arg.arg));
      printf("Driver ---> TID %d\n", arg.tid);
      if (arg.func_num == 0xffff) {
        PhyMemSetByte(DriversArgGet(arg.arg), 'D');
        PhyMemSetByte(DriversArgGet(arg.arg) + 1, 'R');
        PhyMemSetByte(DriversArgGet(arg.arg) + 2, 'I');
        PhyMemSetByte(DriversArgGet(arg.arg) + 3, 'V');
        PhyMemSetByte(DriversArgGet(arg.arg) + 4, 'E');
        PhyMemSetByte(DriversArgGet(arg.arg) + 5, 'R');
      } else {
				int res = 0x404;
				SendMessage(arg.tid, &res, 4);
				continue;
			}
      int result = 0x200;
      SendMessage(arg.tid, &result, 4);
    }
  }
}