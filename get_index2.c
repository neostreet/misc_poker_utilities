#include <stdio.h>

#define NUM_CARDS 52

int get_index(int card1,int card2);

int main(int argc,char **argv)
{
  int m;
  int n;
  int ix;

  for (m = 0; m < NUM_CARDS - 1; m++) {
    for (n = m + 1; n < NUM_CARDS; n++) {
      printf("%d\n",get_index(m,n));
    }
  }

  return 0;
}

int get_index(int card1,int card2)
{
  int m;
  int n;
  int p;

  p = 0;

  for (m = 0; m < NUM_CARDS - 1; m++) {
    for (n = m + 1; n < NUM_CARDS; n++) {
      if ((m == card1) && (n == card2))
        return p;

      p++;
    }
  }

  return -1;
}
