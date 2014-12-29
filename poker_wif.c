#include <stdio.h>

static char usage[] =
"usage: poker_wif invested pot_size call_amount win_pct";

int main(int argc,char **argv)
{
  int invested;
  int pot_size;
  int call_amount;
  int win_pct;
  int loss_pct;
  int fold_aggregate_delta;
  int call_aggregate_delta;

  if (argc != 5) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&invested);
  sscanf(argv[2],"%d",&pot_size);
  sscanf(argv[3],"%d",&call_amount);
  sscanf(argv[4],"%d",&win_pct);

  if (pot_size < invested * 2) {
    printf("invested is greater than half of the pot size\n");
    return 2;
  }

  if ((win_pct < 1) || (win_pct > 99)) {
    printf("win_pct must be >= 1 and <= 99\n");
    return 3;
  }

  loss_pct = 100 - win_pct;

  fold_aggregate_delta = 100 * invested * -1;

  call_aggregate_delta = fold_aggregate_delta +
    loss_pct * call_amount * -1 +
    win_pct * pot_size;

  printf("%10d fold_aggregate_delta\n",fold_aggregate_delta);
  printf("%10d call_aggregate_delta\n",call_aggregate_delta);
  printf("%10d call profit\n",
    call_aggregate_delta - fold_aggregate_delta);

  return 0;
}
