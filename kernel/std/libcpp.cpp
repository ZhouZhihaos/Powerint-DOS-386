
extern "C" {
#include <dos.h>
}
#include <libstdc++.hpp>
string::string() {
  str = new char[1];
  str[0] = 0;
  len = 0;
}
string::~string() {
  delete[] str;
}
string::string(char* s) {
  len = strlen(s);
  str = new char[len + 1];
  strcpy(str, s);
}
string::string(string& s) {
  len = s.len;
  str = new char[len + 1];
  strcpy(str, s.str);
}
string::string(const char* s) {
  len = strlen(s);
  str = new char[len + 1];
  strcpy(str, s);
}
const char* string::c_str() {
  return (const char*)str;
}
string& string::operator=(string& s) {
  if (str)
    delete[] str;
  len = s.len;
  str = new char[len + 1];
  strcpy(str, s.str);
  return *this;
}
string& string::operator=(char* s) {
  if (str)
    delete[] str;
  len = strlen(s);
  str = new char[len + 1];
  strcpy(str, s);
  return *this;
}
string& string::operator+=(string& s) {
  char* tmp = new char[len + s.len + 1];
  strcpy(tmp, str);
  strcat(tmp, s.str);
  if (str) {
    //printf("%s\n", str);
    delete[] str;
  }
  str = tmp;
  len += s.len;
  return *this;
}
string& string::operator+=(char* s) {
  char* tmp = new char[len + strlen(s) + 1];
  strcpy(tmp, str);
  strcat(tmp, s);
  if (str) {
    // printk("%s\n", str);
    delete[] str;
  }
  str = tmp;
  len += strlen(s);
  return *this;
}
string& string::operator+=(char c) {
  char* tmp = new char[len + 2];
  strcpy(tmp, str);
  tmp[len] = c;
  tmp[len + 1] = 0;
  if (str)
    delete[] str;
  str = tmp;
  len++;
  return *this;
}
string& string::operator+=(int i) {
  char* tmp = new char[len + 10];
  strcpy(tmp, str);
  sprintf(tmp + len, "%d", i);
  if (str)
    delete[] str;
  str = tmp;
  len += strlen(tmp + len);
  return *this;
}
string& string::operator+=(long l) {
  char* tmp = new char[len + 20];
  strcpy(tmp, str);
  sprintf(tmp + len, "%ld", l);
  if (str)
    delete[] str;
  str = tmp;
  len += strlen(tmp + len);
  return *this;
}
string& string::operator+=(unsigned int i) {
  char* tmp = new char[len + 10];
  strcpy(tmp, str);
  sprintf(tmp + len, "%d", i);
  if (str)
    delete[] str;
  str = tmp;
  len += strlen(tmp + len);
  return *this;
}
string& string::operator+=(unsigned long l) {
  char* tmp = new char[len + 20];
  strcpy(tmp, str);
  sprintf(tmp + len, "%ld", l);
  if (str)
    delete[] str;
  str = tmp;
  len += strlen(tmp + len);
  return *this;
}
string& string::operator+=(unsigned short s) {
  char* tmp = new char[len + 10];
  strcpy(tmp, str);
  sprintf(tmp + len, "%d", s);
  if (str)
    delete[] str;
  str = tmp;
  len += strlen(tmp + len);
  return *this;
}
bool string::operator==(string& s) {
  if (len != s.len)
    return false;
  return strcmp(str, s.str) == 0;
}
bool string::operator==(char* s) {
  return strcmp(str, s) == 0;
}
bool string::operator!=(string& s) {
  if (len != s.len)
    return true;
  return strcmp(str, s.str) != 0;
}
bool string::operator!=(char* s) {
  return strcmp(str, s) != 0;
}
bool string::operator<(string& s) {
  return strcmp(str, s.str) < 0;
}
bool string::operator<(char* s) {
  return strcmp(str, s) < 0;
}
bool string::operator>(string& s) {
  return strcmp(str, s.str) > 0;
}
bool string::operator>(char* s) {
  return strcmp(str, s) > 0;
}
bool string::operator<=(string& s) {
  return strcmp(str, s.str) <= 0;
}
bool string::operator<=(char* s) {
  return strcmp(str, s) <= 0;
}
bool string::operator>=(string& s) {
  return strcmp(str, s.str) >= 0;
}
bool string::operator>=(char* s) {
  return strcmp(str, s) >= 0;
}
char string::operator[](unsigned int i) {
  return str[i];
}
char string::operator[](unsigned long i) {
  return str[i];
}
string string::operator+(string& s) {
  string tmp;
  tmp.len = len + s.len;
  tmp.str = new char[tmp.len + 1];
  strcpy(tmp.str, str);
  strcat(tmp.str, s.str);
  return tmp;
}
string string::operator+(char* s) {
  string tmp;
  tmp.len = len + strlen(s);
  tmp.str = new char[tmp.len + 1];
  strcpy(tmp.str, str);
  strcat(tmp.str, s);
  return tmp;
}
string string::operator+(const char* s) {
  string tmp;
  tmp.len = len + strlen(s);
  tmp.str = new char[tmp.len + 1];
  strcpy(tmp.str, str);
  strcat(tmp.str, s);
  return tmp;
}
string string::operator+(char c) {
  string tmp;
  tmp.len = len + 1;
  tmp.str = new char[tmp.len + 1];
  strcpy(tmp.str, str);
  tmp.str[len] = c;
  tmp.str[len + 1] = 0;
  return tmp;
}
size_t string::length() {
  return len;
}
COUT& COUT::operator<<(string& s) {
  printf("%s", s.c_str());
  return *this;
}
COUT& COUT::operator<<(char* s) {
  printf("%s", s);
  return *this;
}
COUT& COUT::operator<<(char c) {
  printf("%c", c);
  return *this;
}
COUT& COUT::operator<<(int i) {
  printf("%d", i);
  return *this;
}
COUT& COUT::operator<<(long l) {
  printf("%ld", l);
  return *this;
}
COUT& COUT::operator<<(unsigned int i) {
  printf("%d", i);
  return *this;
}
COUT& COUT::operator<<(unsigned long l) {
  printf("%ld", l);
  return *this;
}
COUT& COUT::operator<<(unsigned short s) {
  printf("%d", s);
  return *this;
}
COUT& COUT::operator<<(unsigned char c) {
  printf("%08x", c);
  return *this;
}
COUT& COUT::operator<<(const char* s) {
  printf("%s", s);
  return *this;
}
void CPPFUNC() {
  cout<<"Hello Cpp!!"<<endl;
}
extern "C" void cpptest() {
  CPPFUNC();
}
