#include <stdio.h>

static char usage[] =
"usage: tournament_wif buy_in entry_fee first_place_prize second_place_prize\n"
"  num_first_places num_second_places num_tournaments\n";

int main(int argc,char **argv)
{
  int buy_in;
  int entry_fee;
  int first_place_prize;
  int second_place_prize;
  int num_first_places;
  int num_second_places;
  int num_tournaments;
  int delta;

  if (argc != 8) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&buy_in);
  sscanf(argv[2],"%d",&entry_fee);
  sscanf(argv[3],"%d",&first_place_prize);
  sscanf(argv[4],"%d",&second_place_prize);
  sscanf(argv[5],"%d",&num_first_places);
  sscanf(argv[6],"%d",&num_second_places);
  sscanf(argv[7],"%d",&num_tournaments);

  if (num_first_places + num_second_places > num_tournaments) {
    printf("num_first_places + num_second_places > num_tournaments\n");
    return 2;
  }

  delta = num_first_places * first_place_prize +
    num_second_places * second_place_prize -
    num_tournaments * (buy_in + entry_fee);

  printf("%d\n",delta);

  return 0;
}
