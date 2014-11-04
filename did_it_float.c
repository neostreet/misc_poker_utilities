#include <stdio.h>

int main(int argc,char **argv)
{
  int spent;
  int pot;
  int win_pct;
  int winning_delta;
  int loss_pct;
  int money_lost;
  int money_won;

  if (argc != 4) {
    printf("usage: did_it_float spent pot win_pct\n");
    return 1;
  }

  sscanf(argv[1],"%d",&spent);
  sscanf(argv[2],"%d",&pot);
  sscanf(argv[3],"%d",&win_pct);

  if (spent <= 0) {
    printf("spent must be > 0\n");
    return 2;
  }

  if (pot <= 0) {
    printf("pot must be > 0\n");
    return 3;
  }

  if (pot <= spent) {
    printf("pot must be > spent\n");
    return 4;
  }

  if ((win_pct < 1) || (win_pct > 99)) {
    printf("win_pct must be >= 1 and <= 99\n");
    return 5;
  }

  winning_delta = pot - spent;
  loss_pct = 100 - win_pct;

  money_lost = spent * loss_pct;
  money_won = winning_delta * win_pct;

  printf("%d (%d %d)\n",money_won - money_lost,money_won,money_lost);

  return 0;
}
