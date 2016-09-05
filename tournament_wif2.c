#include <stdio.h>

static char usage[] =
"tournament_wif2 num_players initial_stake num_firsts num_tournaments\n";

int main(int argc,char **argv)
{
  int num_players;
  int initial_stake;
  int num_firsts;
  int num_tournaments;
  int first_winnings;
  int balance;

  if (argc != 5) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&num_players);
  sscanf(argv[2],"%d",&initial_stake);
  sscanf(argv[3],"%d",&num_firsts);
  sscanf(argv[4],"%d",&num_tournaments);

  if (num_firsts > num_tournaments) {
    printf("num_firsts must be <= num_tournaments\n");
    return 2;
  }

  first_winnings = num_players * initial_stake;

  balance = initial_stake * num_tournaments * -1;
  balance += num_firsts * first_winnings;

  printf("%d\n",balance);

  return 0;
}
