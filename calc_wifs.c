#include <stdio.h>

static char usage[] =
"usage: calc_wifs num_tournaments tournament_info_file\n";

struct tournament_info {
  int buy_in;
  int entry_fee;
  int first_place_prize;
  int second_place_prize;
};

int tournament_wif(
  struct tournament_info *tournament_ptr,
  int num_first_places,
  int num_second_places,
  int num_tournaments,
  int *delta);

int main(int argc,char **argv)
{
  int num_tournaments;
  FILE *fptr;
  struct tournament_info tournament;
  int m;
  int n;
  int o;
  int p;
  int q;
  int r;
  int count;
  int delta;
  int retval;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&num_tournaments);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf("couldn't open %s\n",argv[2]);
    return 2;
  }

  fscanf(fptr,"%d",&tournament.buy_in);
  fscanf(fptr,"%d",&tournament.entry_fee);
  fscanf(fptr,"%d",&tournament.first_place_prize);
  fscanf(fptr,"%d",&tournament.second_place_prize);

  fclose(fptr);

  for (m = 0; m <= num_tournaments; m++) {
    for (n = 0; n <= num_tournaments; n++) {
      count = 0;

      for (o = 0; o <= num_tournaments; o++) {
        for (p = 0; p <= num_tournaments; p++) {
          for (q = 0; q <= num_tournaments; q++) {
            for (r = 0; r <= num_tournaments; r++) {
              if (m + n + o + p + q + r <= num_tournaments)
                count++;
            }
          }
        }
      }

      if (count) {
        retval = tournament_wif(&tournament,
          m,n,num_tournaments,&delta);

        if (retval) {
          printf("tournament_wif failed: %d\n",retval);
          printf("m = %d, n = %d\n",m,n);
          return 3;
        }

        printf("%7d (%3d)\n",delta,count);
      }
    }
  }

  return 0;
}

int tournament_wif(
  struct tournament_info *tourn_ptr,
  int num_first_places,
  int num_second_places,
  int num_tournaments,
  int *delta_ptr)
{
  if (num_first_places + num_second_places > num_tournaments) {
    printf("num_first_places + num_second_places > num_tournaments\n");
    return 1;
  }

  *delta_ptr = num_first_places * tourn_ptr->first_place_prize +
    num_second_places * tourn_ptr->second_place_prize -
    num_tournaments * (tourn_ptr->buy_in + tourn_ptr->entry_fee);

  return 0;
}
