#include "syscall.h"
#define THIS "aaa"
#define THAT "bbb"
const int N = 10; // Choose it large enough!
void puts(char *s)
{
char *p; for (p = s; *p != '\0'; p++) PutChar(*p);
}
void f(void *s)
{
int i; for (i = 0; i < N; i++) puts((char *)s); UserThreadExit();
}
int main() {
  //int i;
  UserThreadCreate(f, (void *) THIS);
  f((void*) THAT);
  return 0;
}
