#include <stdio.h>
#include <string.h>

#define BUY_IN 50000
#define BOUNTY 21250
#define BOUNTY_NO_ENTRY_FEE 25000

static int ko_payouts[] {
  95625,
  57375,
  38250
};

static int ko_payouts_no_entry_fee[] {
  112500,
  67500,
  45000
};

static char usage[] = "usage: ko_delta (-no_entry_fee) place num_bounties\n";

int main(int argc,char **argv)
{
  int curr_arg;
  bool bNoEntryFee;
  int place;
  int num_bounties;
  int delta;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bNoEntryFee = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-no_entry_fee"))
      bNoEntryFee = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&place);
  sscanf(argv[curr_arg+1],"%d",&num_bounties);

  if ((place < 1) || (place > 9)) {
    printf("place must be >= 1 and <= 9\n");
    return 2;
  }

  if ((num_bounties < 0) || (num_bounties > 9)) {
    printf("num_bounties must be >= 0 and <= 9\n");
    return 3;
  }

  delta = BUY_IN * -1;

  if ((place >= 1) && (place <= 3))
    delta += (bNoEntryFee ? ko_payouts_no_entry_fee[place - 1] : ko_payouts[place - 1]);

  if (num_bounties)
    delta += num_bounties * (bNoEntryFee ? BOUNTY_NO_ENTRY_FEE : BOUNTY);

  printf("%d\n",delta);

  return 0;
}
