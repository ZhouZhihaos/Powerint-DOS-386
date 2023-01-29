#include <string.h>
#include <syscall.h>
#include <stdio.h>
int main(int argc, char** argv) {
  if (argc == 1) {
    print("ERROR:No input file.");
    return 0;
  }
  Bitz(argv[1]);
  return 0;
}
void Bitz(char* filename) {
  
  if (filesize(filename) == -1) {
    print("File not found\n");
    return;
  }
  
  int size = filesize(filename);
  //printf("%s",filename);
  unsigned char* file_buffer = (unsigned char*)api_malloc(size);
  api_ReadFile(filename, file_buffer);

  print(
      "         00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  "
      "0123456789ABCDEF\n");
  print("00000000 ");
  int i;
  int l = 0;
  for (i = 0; i < size; i++) {
    char* buffer = api_malloc(15);
    sprintf(buffer, "%02x ", file_buffer[i]);
    print(buffer);
    if ((i + 1) % 16 == 0) {
      //输出对应的ascii码
      print(" ");
      for (int j = i - 15; j <= i; j++) {
        if (file_buffer[j] >= 32 && file_buffer[j] <= 126 &&
            file_buffer[j] != ' ') {
          putch(file_buffer[j]);
        } else {
          putch('.');
        }
      }
      l++;
      print("\n");
      if (l == 23) {
        print("Press any key to continue...");
        getch();
        print("\n");
        l = 0;
      }
      printf("%08x ", i + 1);
    }
    api_free(buffer, 15);
  }
  //剩下的
  i--;
  for (;; i++) {
    if (i % 16 == 15) {
      break;
    }
    print("00 ");
  }
  print(" ");
  for (int j = i - 15; j <= size; j++) {
    if (file_buffer[j] >= 32 && file_buffer[j] <= 126) {
      putch(file_buffer[j]);
    } else {
      putch('.');
    }
  }
  api_free(file_buffer, size);
}