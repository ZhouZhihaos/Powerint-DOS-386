#include <gui.h>
#include <syscall.h>
window_t mainWindow;
HWND listBox;
button_t add;
HWND text1;
int PowerMain() {
  mainWindow = create_window(0,0,640,400,"Student");
  listBox = MakeListBox(mainWindow,5,28,200,350);
  text1   = MakeTextBox(300,28,16*8,20,mainWindow);
  add     = create_button(300,30,3*8,20,mainWindow,"Add");
  while(1) {
    window_t id;
    int w_sz;
    int b_sz;
    w_sz = MessageLength(WINDOW_CLOSE_MSG_ID);
    b_sz = MessageLength(BUTTON_MSG_ID);
    if(w_sz == 4) {
      GetMessage(&id, WINDOW_CLOSE_MSG_ID);
      if(id == mainWindow) {
        close_window(mainWindow);
        DeleteListBox(listBox);
        DeleteTextBox(text1);
        delete_button(add);
        return 0;
      }
    }
    if(b_sz == 4) {
      GetMessage(&id, BUTTON_MSG_ID);
      if(id == add) {
        char *str = api_malloc(sizeof(char)*16);
        GetTextBoxText(str,text1);
        AddItem(listBox,str);
        api_free(str,sizeof(char)*16);
      }
    }
  }
}