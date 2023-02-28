#include <string.h>
#include <syscall.h>

int main(int argc, char **argv) {
  //注册用户向导
  system("cls");
  print("Welcome to use the user guide!\n");
  char user_name[20];
  char user_password[20];
  char user_password_again[20];
re:
  for (int i = 0; i < 20; ++i) {
    user_name[i] = 0;
    user_password[i] = 0;
    user_password_again[i] = 0;
  }
  print("User:");
  scan(user_name, 20);
  print("Password:");
  scan(user_password, 20);
  print("Password again:");
  scan(user_password_again, 20);
  if (strcmp(user_password, user_password_again) != 0) {
    print("The two passwords are not the same!\n");
    goto re;
  }
  //创建用户
  print("All OK.\n");
  print("last step:create user file and password file in floppy disk.\n");
  print("system is creating directory USER ... ");
  mkdir("USER");
  print("OK. Path is USER\n");
  print("system is creating USER.INI ... ");
  mkfile("USER\\USER.INI");
  print("OK. Path is USER\\USER.INI\n");
  print("Now Writing user info to USER.INI ... ");
  Edit_File("USER\\USER.INI", user_name, strlen(user_name) + 1, 0);
  print("done.\n");
  print("system is creating PASSWORD.INI ... ");
  mkfile("USER\\PASSWORD.INI");
  print("OK. Path is USER\\PASSWORD.INI\n");
  print("Now Writing password info to PASSWORD.INI ... ");
  Edit_File("USER\\PASSWORD.INI", user_password, strlen(user_password) + 1, 0);
  print("done.\n");
  print("All OK.\n");
  return 0;
}