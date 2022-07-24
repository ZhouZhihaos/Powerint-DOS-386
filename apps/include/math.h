// PowerintDOS提供的数学头文件
// @ Copyright (C) 2022
// @ Author: min0911_
#ifndef MATH_H
#define MATH_H
float intpower(float a, int n);
float ln(float x);
int fact(int n);
extern __inline__ double sin(double x);
extern __inline__ double cos(double x);
extern __inline__ double sqrt(double x);
extern __inline__ double ABS(double x);
double Bernoulli(int x);//伯努利数
double log(double a);
double nth(double x,int n);
double absolute(double x);
double Factorial(int x);
double tan(double x);
double atan(double x, double y);
double asin(double x);
double acos(double x);
#define _E 2.71828182845904523536028747135266249775724709369995
#define pai 3.1415926535897932
#endif