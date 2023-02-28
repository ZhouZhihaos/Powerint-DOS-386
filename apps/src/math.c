#define accuracy 0.000001
#define pai 3.1415926
float intpower(float a, int n) {  //非负整数次方
  if (n == 0) {
    return 1;
  } else {
    int i;
    float s = 1;
    for (i = 0; i < n; i++) {
      s = s * a;
    }
    return s;
  }
}
float ln(float x) {  //自然对数
  float s = 0;
  int E = 50;  //精度
  if (x < 1) {
    int i, sygn = 1;
    for (i = 1; i < E + 1; i++) {
      s = s + sygn * intpower(x - 1, i) / i;
      sygn = -sygn;
    }
  } else {
    x = 1 / x;
    int i, sygn = 1;
    for (i = 1; i < E + 1; i++) {
      s = s + sygn * intpower(x - 1, i) / i;
      sygn = -sygn;
    }
    s = -s;
  }

  return s;
}
int fact(int n) {
  int s = 1;
  if (n == 0) {
    return 1;
  } else {
    int i;
    for (i = 1; i <= n; i++) {
      s = s * i;
    }
    return s;
  }
}
extern __inline__ double sin(double x) {
  double res;
  __asm__("fsin" : "=t"(res) : "0"(x));
  return res;
}

extern __inline__ double cos(double x) {
  double res;
  __asm__("fcos" : "=t"(res) : "0"(x));
  return res;
}

extern __inline__ double sqrt(double x) {
  double res;
  __asm__("fsqrt" : "=t"(res) : "0"(x));
  return res;
}

extern __inline__ double abs(double x) {
  double res;
  __asm__("fabs" : "=t"(res) : "0"(x));
  return res;
}
double fabs(double x) {
  double res;
  __asm__("fabs" : "=t"(res) : "0"(x));
  return res;
}
double absolute(double x) {
  if (x < 0)
    x = -x;
  return x;
}

double Factorial(int x) {
  if (x == 1 || x == 0)
    return 1;
  else
    return 1.0 * x * Factorial(x - 1);
}

double nth(double x, int n) {
  if (n > 0) {
    return x * nth(x, n - 1);
  }
  if (n == 0) {
    return 1;
  }
  if (n < 0) {
    return (1 / x) * nth(x, n + 1);
  }
}
double Bernoulli(int x) {
  int k = x;
  double B = 0;
  if (x == 0)
    return 1;
  else if (x > 1 && x % 2 == 1) {
    return 0;
  } else {
    while (k) {
      k--;
      B += -1.0 * (Factorial(x) * Bernoulli(k)) /
           (Factorial(x - k) * Factorial(k) * (x - k + 1));
    }
    return B;
  }
}

double tan(double x) {
  int i = 1;
  double e = 1, sum = 0;
  while (x < (-pai / 2))
    x += pai;
  while (x > (pai / 2))
    x -= pai;
  if (x == (pai / 2)) {
    return 0;
  }
  while (absolute(e) > accuracy && i <= 24)  //考虑计算速度，控制Bernoulli()
  {
    e = 1.0 *
        (nth(-1, i - 1) * nth(2, 2 * i) * (nth(2, 2 * i) - 1.0) *
         Bernoulli(2 * i) * nth(x, 2 * i - 1)) /
        (Factorial(2 * i));
    sum += e;
    i++;
  }
  return sum;
}


double acos(double x) {
  return pai / 2 - x - intpower(x, 3) / 6 - (3 * intpower(x, 5)) / 40 -
         (5 * intpower(x, 7)) / 112 - (35 * intpower(x, 9)) / 1152 -
         (63 * intpower(x, 11)) / 2816 - (231 * intpower(x, 13)) / 13312 -
         (143 * intpower(x, 15)) / 10240 - (6435 * intpower(x, 17)) / 557056 -
         (12155 * intpower(x, 19)) / 1245184 -
         (46189 * intpower(x, 21)) / 5505024 -
         (88179 * intpower(x, 23)) / 12058624 -
         (676039 * intpower(x, 25)) / 104857600 -
         (1300075 * intpower(x, 27)) / 226492416 -
         (5014575 * intpower(x, 29)) / 973078528 -
         (9694845 * intpower(x, 31)) / 2080374784 -
         (100180065 * intpower(x, 33)) / 23622320128 -
         (116680311 * intpower(x, 35)) / 30064771072 -
         (2268783825 * intpower(x, 37)) / 635655159808 -
         (1472719325 * intpower(x, 39)) / 446676598784;
}
double log(double a) {
  int N = 15;  //我们取了前15+1项来估算
  int k, nk;
  double x, xx, y;
  x = (a - 1) / (a + 1);
  xx = x * x;
  nk = 2 * N + 1;
  y = 1.0 / nk;
  for (k = N; k > 0; k--) {
    nk = nk - 2;
    y = 1.0 / nk + xx * y;
  }
  return 2.0 * x * y;
}
