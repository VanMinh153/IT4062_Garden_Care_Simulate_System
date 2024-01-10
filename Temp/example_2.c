#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
struct type1 {
  int x;
  char str[100];
};
typedef struct type1 type1;

type1 var1[10];

void func1(type1* var) {
  printf("func1 $ %ld\n", var - var1);
  printf("func1 $ %ld\n", var - &var1[0]);
}
int main() {
  type1* var2 = &var1[5];
  
  printf("main $ %ld\n", var2 - var1);
  func1(var2);

  // var1.x = 1;
  // var1.str[0] = 'a';
  // var2.x = 2;
  // var2.str[0] = 'b';
  // // var1 = var2;
  // printf("%d %d\n", var1.x, var2.x);
  // printf("%c %c\n", var1.str[0], var2.str[0]);
  return 0;
}