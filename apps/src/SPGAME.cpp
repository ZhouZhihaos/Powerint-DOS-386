#include <stdio.h>
#include <syscall.h>

class A {
 public:
  virtual void func() { printf("call father\n"); }
};

class B : public A {
 public:
  virtual void func() { printf("call kid\n"); }
};

int main() {
  A obj1;

  obj1.func();  // 父类

  A* obj2 = new A();

  obj2->func();  // 父类

  A* obj3 = new B();

  obj3->func();  // 子类

  B obj4_temp;

  A* obj4 = &obj4_temp;

  obj4->func();  // 子类

  B obj5_temp;

  A& obj5 = obj5_temp;

  obj5.func();  // 子类

  return 0;
}