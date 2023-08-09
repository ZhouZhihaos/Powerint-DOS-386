#include <stdio.h>
#include <string.h>
#include <syscall.h>
void copy_drive(char d, char *p1, char *p2) {
  char *buf = malloc(strlen(p2) + 4);
  if (p2[0] == '/') {
    sprintf(buf, "%c:%s", d, p2);
  } else {
    sprintf(buf, "%c:\\%s",d, p2);
  }
  Copy(p1, buf);
  free(buf);
}
char tosmaller(char s) {
  if(s >= 'A' && s <= 'Z') {
    s += 32;
  }
  return s;
}
void change_disk(char d) {
  char *buf = malloc(4);
  sprintf(buf, "%c:", d);
  system(buf);
  free(buf);
}
int main() {
  char d, c;
  c = api_current_drive();
R:
  printf("Which drive do you want to install? [A/C/D/E/F]\n");
  d = getch();
  d = toupper(d);
  printf("tcc will install in Drive %c [y/n]\n", d);
  if (getch() == 'n') {
    goto R;
  }
  printf("installation is making tcc dict....\n");
  change_disk(d);
  printf(" -> tcc\n");
  mkdir("tcc");
  system("cd tcc");
  printf(" -> tcc/crt\n");
  mkdir("crt");
  printf(" -> tcc/lib\n");
  mkdir("lib");
  printf(" -> tcc/inst\n");
  mkdir("inst");
  printf(" -> tcc/include\n");
  mkdir("include");
  change_disk(c);
  printf("now, copy binary\n");
  printf(" -> tcc.bin\n");
  copy_drive(d, "tcc.bin", "/tcc.bin");
  printf(" -> tcc/lib/libp.a\n");
  copy_drive(d, "tcc/lib/libp.a", "/tcc/lib/libp.a");
  printf(" -> tcc/inst/libtcc1.a\n");
  copy_drive(d, "tcc/inst/libtcc1.a", "/tcc/inst/libtcc1.a");
  printf("now, copy headers\n");
  struct finfo_block *f= listfile("tcc/include");
  for(int i = 0;f[i].name[0];i++) {
    if(f[i].type == DIR) {
      continue;
    }
    char s[500];
    char s2[500];
    for(int j = 0;j<strlen(f[i].name);j++) {
      f[i].name[j] = tosmaller(f[i].name[j]);
    }
    printf(" -> %s\n",f[i].name);
    sprintf(s,"tcc/include/%s",f[i].name);
    sprintf(s2,"tcc/include/%s",f[i].name);
    copy_drive(d,s,s2);
  }
  printf("last, build crti.c\n");
  char buf[255];
  sprintf(buf,"tcc.bin -c crti.c -o %c:\\tcc\\crt\\crti.o",d);
  printf("execute: %s\n",buf);
  system(buf);
}