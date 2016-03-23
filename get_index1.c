#include <stdio.h>

#define NUM_CARDS 52

int get_index(int card1,int card2);

int main(int argc,char **argv)
{
  int n;
  int cards[2];
  int ix;

  if (argc != 3) {
    printf("usage: get_index1 card1 card2\n");
    return 1;
  }

  for (n = 0; n < 2; n++)
    sscanf(argv[1+n],"%d",&cards[n]);

  ix = get_index(cards[0],cards[1]);

  printf("%d\n",ix);

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
