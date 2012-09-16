#include <stdio.h>

#define NUM_PLACES      6
#define NUM_TOURNAMENTS 6

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

  if (argc != 2) {
    printf("usage: calc_wifs tournament_info_file\n");
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf("couldn't open %s\n",argv[1]);
    return 2;
  }

  fscanf(fptr,"%d",&tournament.buy_in);
  fscanf(fptr,"%d",&tournament.entry_fee);
  fscanf(fptr,"%d",&tournament.first_place_prize);
  fscanf(fptr,"%d",&tournament.second_place_prize);

  fclose(fptr);

  for (m = 0; m <= NUM_PLACES; m++) {
    for (n = 0; n <= NUM_PLACES; n++) {
      count = 0;

      for (o = 0; o <= NUM_PLACES; o++) {
        for (p = 0; p <= NUM_PLACES; p++) {
          for (q = 0; q <= NUM_PLACES; q++) {
            for (r = 0; r <= NUM_PLACES; r++) {
              if (m + n + o + p + q + r <= NUM_PLACES)
                count++;
            }
          }
        }
      }

      if (count) {
        retval = tournament_wif(&tournament,
          m,n,NUM_TOURNAMENTS,&delta);

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
