#include <fibo.h>
int fibo(int n) {
  int a;
  int b;
  int c;
  a = 1;
  int i;
  c = 0;
  b = 0;
  for (i = 2; i <= n; i++) {
    c = b;
    b = a;
    a = b + c;
  }
  return a;
}
