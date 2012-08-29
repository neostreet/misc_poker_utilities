#include <stdio.h>

static char usage[] =
"usage: tournament_wif filename\n"
"  num_first_places num_second_places num_tournaments\n";

int main(int argc,char **argv)
{
  FILE *fptr;
  int buy_in;
  int entry_fee;
  int first_place_prize;
  int second_place_prize;
  int num_first_places;
  int num_second_places;
  int num_tournaments;
  int delta;

  if (argc != 5) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf("couldn't open %s\n",argv[1]);
    return 2;
  }

  fscanf(fptr,"%d",&buy_in);
  fscanf(fptr,"%d",&entry_fee);
  fscanf(fptr,"%d",&first_place_prize);
  fscanf(fptr,"%d",&second_place_prize);

  fclose(fptr);

  sscanf(argv[2],"%d",&num_first_places);
  sscanf(argv[3],"%d",&num_second_places);
  sscanf(argv[4],"%d",&num_tournaments);

  if (num_first_places + num_second_places > num_tournaments) {
    printf("num_first_places + num_second_places > num_tournaments\n");
    return 3;
  }

  delta = num_first_places * first_place_prize +
    num_second_places * second_place_prize -
    num_tournaments * (buy_in + entry_fee);

  printf("%d\n",delta);

  return 0;
}
