#include <stdio.h>
#include <string.h>

struct tournament_info {
  int buy_in;
  int entry_fee;
  int first_place_prize;
  int second_place_prize;
};

static char usage[] =
"usage: tournament_wif (-verbose) (-right_justify) (-until_profit)\n"
"  tournament_info_file num_first_places num_second_places num_tournaments\n";

int tournament_wif(
  struct tournament_info *tournament_ptr,
  int num_first_places,
  int num_second_places,
  int num_tournaments,
  int *delta);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  bool bRightJustify;
  bool bUntilProfit;
  FILE *fptr;
  struct tournament_info tournament;
  int num_first_places;
  int num_second_places;
  int num_tournaments;
  int delta;
  int retval;

  if ((argc < 5) || (argc > 8)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bRightJustify = false;
  bUntilProfit = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-right_justify"))
      bRightJustify = true;
    else if (!strcmp(argv[curr_arg],"-until_profit"))
      bUntilProfit = true;
    else
      break;
  }

  if (argc - curr_arg != 4) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf("couldn't open %s\n",argv[curr_arg]);
    return 3;
  }

  fscanf(fptr,"%d",&tournament.buy_in);
  fscanf(fptr,"%d",&tournament.entry_fee);
  fscanf(fptr,"%d",&tournament.first_place_prize);
  fscanf(fptr,"%d",&tournament.second_place_prize);

  fclose(fptr);

  sscanf(argv[curr_arg+1],"%d",&num_first_places);
  sscanf(argv[curr_arg+2],"%d",&num_second_places);
  sscanf(argv[curr_arg+3],"%d",&num_tournaments);

  for ( ; ; ) {
    retval = tournament_wif(&tournament,
      num_first_places,num_second_places,num_tournaments,&delta);

    if (retval) {
      printf("tournament_wif failed: %d\n",retval);
      return 4;
    }

    if (!bVerbose) {
      if (!bRightJustify)
        printf("%d\n",delta);
      else
        printf("%7d\n",delta);
    }
    else
      printf("%7d %2d %2d %2d\n",delta,num_first_places,num_second_places,num_tournaments);

    if (!bUntilProfit || (delta > 0))
      break;

    num_first_places++;
    num_second_places++;
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
