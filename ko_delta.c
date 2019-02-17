#include <stdio.h>

#define BUY_IN 50000
#define BOUNTY 21250

static int ko_payouts[] {
  95625,
  57375,
  38250
};

static char usage[] = "usage: ko_delta place num_bounties\n";

int main(int argc,char **argv)
{
  int place;
  int num_bounties;
  int delta;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&place);
  sscanf(argv[2],"%d",&num_bounties);

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
    delta += ko_payouts[place - 1];

  if (num_bounties)
    delta += num_bounties * BOUNTY;

  printf("%d\n",delta);

  return 0;
}
