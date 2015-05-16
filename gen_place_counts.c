#include <stdio.h>

int main(int argc,char **argv)
{
  int m;
  int n;

  for (m = 0; m <= 6; m++) {
    for (n = 0; n <= 6 - m; n++) {
      printf("%d %d 6\n",m,n);
    }
  }

  return 0;
}
