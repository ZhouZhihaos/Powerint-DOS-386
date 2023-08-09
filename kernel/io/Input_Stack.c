#include <io.h>
char **Input_Stack; // 1024个字符串
struct Input_StacK *STACK = NULL;
void Input_Stack_Init(void) {
  int i;
  STACK = (struct Input_StacK *)page_malloc(sizeof(Input_Stack));
  Input_Stack = (char *)page_malloc(sizeof(char *) * 1024);
  for (i = 0; i < 1024; i++) {
    Input_Stack[i] = (char *)page_malloc(sizeof(char) * 4096);
  }

  for (i = 0; i < 1024; i++) {
    for (int j = 0; j < 1024; j++) {
      Input_Stack[i][j] = 0;
    }
  }
  STACK->Stack = Input_Stack;
  STACK->Stack_Size = 1024;
  STACK->free = 1023;
  STACK->Now = 1023;
  STACK->times = 0;
}
void input_stack_put(char *str) {
  int i;

  if (STACK->Now == 0) {
    for (i = 0; i < 1024; i++) {
      STACK->Stack[i] = NULL;
    }
    STACK->free = 1023;
    STACK->Now = 1023;
    STACK->times = 0;
    input_stack_put(str);
  } else {
    strcpy(STACK->Stack[STACK->Now], str);
    STACK->free--;
    STACK->Now--;
    STACK->times++;
  }
}
int input_stack_get_now() { return STACK->Now; }
int Get_times() { return STACK->times; }
char *input_stack_pop() { return STACK->Stack[STACK->Now + 1]; }
void input_stack_set_now(int now) { STACK->Now = now; }
int get_free() { return STACK->free; }