#ifndef __STRING__HPP__
#define __STRING__HPP__
class string {
 public:
  char* str;
  int len;
  string();
  string(char* s);
  string(string& s);
  string(const char* s);
  ~string();
  const char* c_str();
  string& operator=(string& s);
  string& operator=(char* s);
  string& operator+=(string& s);
  string& operator+=(char* s);
  string& operator+=(char c);
  string& operator+=(int i);
  string& operator+=(long l);
  string& operator+=(unsigned int i);
  string& operator+=(unsigned long l);
  string& operator+=(unsigned short s);
  string& operator+=(unsigned char c);
  // string& operator+=(void* p);
  // string& operator+=(char c);
  bool operator==(string& s);
  bool operator==(char* s);
  bool operator!=(string& s);
  bool operator!=(char* s);
  bool operator<(string& s);
  bool operator<(char* s);
  bool operator>(string& s);
  bool operator>(char* s);
  bool operator<=(string& s);
  bool operator<=(char* s);
  bool operator>=(string& s);
  bool operator>=(char* s);
  char operator[](unsigned int i);
  char operator[](unsigned long i);
  string operator+(string& s);
  string operator+(char* s);
  string operator+(char c);
  string operator+(int i);
  string operator+(long l);
  string operator+(unsigned int i);
  string operator+(unsigned long l);
  string operator+(unsigned short s);
  string operator+(unsigned char c);
  string operator+(const char* s);
  size_t length();
};
class COUT {
 public:
  COUT& operator<<(char* s);
  COUT& operator<<(const char* s);
  COUT& operator<<(char c);
  COUT& operator<<(int i);
  COUT& operator<<(long l);
  COUT& operator<<(unsigned int i);
  COUT& operator<<(unsigned long l);
  COUT& operator<<(unsigned short s);
  COUT& operator<<(unsigned char c);
  COUT& operator<<(string& s);
};
static COUT cout;
#define endl "\n"
#endif