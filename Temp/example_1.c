#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

int main() {
  // int var;
  // char var;
  // int var = 0;
  // char var = 'a';
  // printf("%d\n", var);
  // printf("%c\n", var);
  return 0;
}

// int main() {
//   int connCtrl[20] = {1,0,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1,1,0,0};
//   // int connCtrl[20];
//   int conninfo[20];
//   int numConnect = 7;
//   int end_idx = 17;
//   // memset(connCtrl, 0, sizeof(connCtrl));
//   memset(conninfo, 55, sizeof(conninfo));

// // printf("size of connCtrl: %d\n", sizeof(connCtrl)); return 0;

//   // for (int i = 0; i < 20; i++) {
//   //   printf("%d | ", connCtrl[i]);
//   // }
//   // return 0;
//   if (end_idx != numConnect - 1) {
//     for (int i = 0; i <= end_idx; i++) {
//       if (connCtrl[i] == 0) {
//         conninfo[i] = conninfo[end_idx];
//         connCtrl[end_idx] = 0;
//         connCtrl[i] = 1;
//         for (int j = end_idx; j > i; j--) {
//           if (connCtrl[j] == 1) {
//             end_idx = j;
//             break;
//           }
//         }        
//       }
//     }
//   }
//   for (int i = 0; i < 20; i++) {
//     printf("%d | ", connCtrl[i]);
//   }
//   return 0;
// }