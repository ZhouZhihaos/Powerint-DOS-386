extern "C" {
#include <dos.h>
}
#include <graphic.hpp>
int login_flag = 0;
ui::PowerWindow *login_window;
ui::PowerButton *login_button;
ui::PowerButton *register_button;
ui::PowerButton *exit_button;
ui::PowerTextBox *textbox1;
ui::PowerTextBox *textbox2;

ui::PowerWindow *register_window;
ui::PowerButton *register_button1;
ui::PowerButton *register_button2;
ui::PowerTextBox *reg_textbox1;
ui::PowerTextBox *reg_textbox2;
void Exit_Button_OnClick() {
  running_mode = POWERINTDOS;
  acpi_shutdown();
  SwitchToText8025_BIOS();
  clear();
  printf("Now you can safe to shutdown you computer.\n");
  io_cli();
  for (;;)
    ;
}
void login_button_OnClick() {
  if (fsz("A:\\USER\\USER.INI") != -1 && fsz("A:\\USER\\PASSWORD.INI") != -1) {
    char *right_user = new char[fsz("A:\\USER\\USER.INI") + 1];
    char *right_pass = new char[fsz("A:\\USER\\PASSWORD.INI") + 1];
    struct FILEINFO *f1 = Get_File_Address("A:\\USER\\USER.INI");
    struct FILEINFO *f2 = Get_File_Address("A:\\USER\\PASSWORD.INI");
    file_loadfile(f1->clustno, f1->size, right_user, drive_ctl.drives[0x0].fat,
                  0x0);
    file_loadfile(f2->clustno, f2->size, right_pass, drive_ctl.drives[0x0].fat,
                  0x0);
    if (strcmp(textbox1->Text(), right_user) == 0 &&
        strcmp(textbox2->Text(), right_pass) == 0) {
      login_flag = 1;
    } else {
      MsgBox("登录失败！", "登录");
      printk("Debug: Right : %s Input %s (User)", right_user, textbox1->Text());
      printk("Debug: Right : %s Input %s (Pass)", right_pass, textbox2->Text());
    }
    delete[] right_user;
    delete[] right_pass;

    if (login_flag != 0) {
      delete textbox1;
      delete textbox2;
      delete exit_button;
      delete login_button;
      delete register_button;
      delete login_window;
    }
  } else {
    MsgBox("You are not registered yet.Please register first.", "Error");
  }
}
void close() { MsgBox("You can't close this window", "Error"); }
void register_window_close() {
  delete register_button1;
  delete register_button2;
  delete reg_textbox1;
  delete reg_textbox2;
  delete register_window;
}
void register_window_button1_OnClick() {
  if (fsz("A:\\USER\\USER.INI") != -1 && fsz("A:\\USER\\PASSWORD.INI") != -1) {
    MsgBox("You are not need Register!", "Error");
    register_window_close();
    return;
  }

  command_run("mkdir user");
  mkfile("A:\\USER\\USER.INI");
  mkfile("A:\\USER\\PASSWORD.INI");

  EDIT_FILE("A:\\USER\\USER.INI", reg_textbox1->Text(),
            strlen(reg_textbox1->Text()), 0);
  EDIT_FILE("A:\\USER\\PASSWORD.INI", reg_textbox2->Text(),
            strlen(reg_textbox2->Text()), 0);
  register_window_close();
}
void register_window_button2_OnClick() { register_window_close(); }
void reg_button_OnClick() {
  register_window = new ui::PowerWindow(
      ui::POSITION((1024 / 2 - 400) + 50, 768 / 2 - 200, 400, 200), "注册",
      register_window_close);
  register_window->Text(65, 52, "用户:", COL_000000);
  register_window->Text(65, 52 + 16 + 8, "密码:", COL_000000);
  reg_textbox1 = new ui::PowerTextBox(register_window,
                                      ui::POSITION(65 + 40 + 8, 52, 160, 16));
  reg_textbox2 = new ui::PowerTextBox(
      register_window, ui::POSITION(33 + 72 + 8, 52 + 16 + 8, 160, 16));
  int base = 74;
  register_button1 = new ui::PowerButton(
      register_window, ui::POSITION(base + 45 - 5 + 14, 135 - 28, 16*2 + 5, 16),
      "好了", register_window_button1_OnClick);
  register_button2 = new ui::PowerButton(
      register_window,
      ui::POSITION(base + 45 - 5 + 14 + 10 + 15 + 14+8, 135 - 28, 16*2 + 5, 16),
      "取消", register_window_button2_OnClick);
}
void __gui_login() {
  login_window = new ui::PowerWindow(
      ui::POSITION(1024 / 2 - 400, 768 / 2 - 200, 400, 200), "登录", close);
  login_window->Text(65, 52, "用户:", COL_000000);
  login_window->Text(65, 52 + 16 + 8, "密码:", COL_000000);
  textbox1 = new ui::PowerTextBox(login_window,
                                  ui::POSITION(65 + 40 + 8, 52, 160, 16));
  textbox2 = new ui::PowerTextBox(
      login_window, ui::POSITION(33 + 72 + 8, 52 + 16 + 8, 160, 16));
  int base = 60;
  login_button = new ui::PowerButton(
      login_window, ui::POSITION(base + 45 - 5 + 14, 135 - 28, 16*2 + 5, 16),
      "登录", login_button_OnClick);
  register_button = new ui::PowerButton(
      login_window,
      ui::POSITION(base + 45 - 5 + 14 + 10 + 15 + 14+8, 135 - 28, 16*2 + 5, 16),
      "注册", reg_button_OnClick);
  exit_button = new ui::PowerButton(
      login_window,
      ui::POSITION(base + 45 - 5 + 14 + 40 + 15 + 14 + 64 + 15 + 14 - 70, 135 - 28,
                   32 + 5, 16),
      "退出", Exit_Button_OnClick);
}
extern "C" void gui_login() { __gui_login(); }
extern "C" int get_login_flag() {
  if (login_flag == 1) {
    login_flag = -1; // 已经登入
    return 1;
  } else if (login_flag == 0) {
    return 0;
  } else {
    return -1;
  }
}