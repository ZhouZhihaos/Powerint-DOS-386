#include <stdio.h>
#include <mst.h>

int main() {
  MST_Object *mst = Init_MstObj("\"a\" = 123");
  Var *v1 = MST_GetVar("a",MST_GetRootSpace(mst));
  printf("a = %d\n",MST_Space_GetInteger(v1));
  return 0;
}