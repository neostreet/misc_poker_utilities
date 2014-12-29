#include <stdio.h>

static char usage[] =
"usage: poker_wif2 invested pot_size call_amount";

int main(int argc,char **argv)
{
  int invested;
  int pot_size;
  int call_amount;

  if (argc != 4) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&invested);
  sscanf(argv[2],"%d",&pot_size);
  sscanf(argv[3],"%d",&call_amount);

  if (pot_size < invested * 2) {
    printf("invested is greater than half of the pot size\n");
    return 2;
  }

  printf("%10d fold\n",
    invested * -1);
  printf("%10d call and lose\n",
    (invested + call_amount) * -1);
  printf("%10d call and win\n",
    invested * -1 + pot_size);

  return 0;
}
