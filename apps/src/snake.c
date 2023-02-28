#include <rand.h>
#include <syscall.h>

#define srand mysrand
#define rand myrand
#define _getch getch
#define _sleep sleep
#define W 15
#define H W
#define S (W * H)
int main(int argc, char** argv) {
  char o[S * 2 + W + 3], *c = o;
  int m[S + 1], q[S], l = 0, r = 4, d = 1, p, a, i;
  for (i = l; i <= r; ++i)
    q[i] = S;
  for (i = 0; i < S; c += 2, ++i % W || (*c++ = '\n')) {
    m[i] = !(i / W % (H - 1) && i % W % (W - 1));
    m[i] ? (*c = '[', c[1] = ']') : (*c = c[1] = ' ');
  }
  p = W / 2 * (H + 1) - d, a = p + d, c[-1] = '\0';
  for (srand(RAND()); i = 1; sleep(50)) {
    if (_kbhit()) {
      i = _getch() & 95;
      if (i == 'A' && d != 1)
        d = -1;
      else if (i == 'D' && d != -1)
        d = 1;
      else if (i == 'W' && d != W)
        d = -W;
      else if (i == 'S' && d != -W)
        d = W;
    }
    if (m[p += d] && p != q[l + 1] || i == 27) {
      break;
    }
    if (p == a) {
      for (a = rand() % S; m[a]; a = (a + 1) % S) {
      }
      *(c = o + a * 2 + a / W) = '0', c[1] = '0';
    } else {
      m[i = q[l = (l + 1) % S]] = 0;
      *(c = o + i * 2 + i / W) = ' ', c[1] = ' ';
    }
    m[q[r] = p] = 1, r = (r + 1) % S;
    *(c = o + p * 2 + p / W) = '(', c[1] = ')';
    system("cls"), print(o);
  }
  print("\nGame over!\n");
  return 0;
}