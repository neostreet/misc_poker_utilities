#include <stdio.h>
#include <string.h>

struct tournament_info {
  int buy_in;
  int entry_fee;
  int first_place_prize;
  int second_place_prize;
};

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: tournament_wif (-verbose) (-right_justify) tournament_info_file places_file\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
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
  FILE *fptr;
  int line_len;
  int line_no;
  struct tournament_info tournament;
  int num_first_places;
  int num_second_places;
  int num_tournaments;
  int delta;
  int retval;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bRightJustify = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-right_justify"))
      bRightJustify = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  fscanf(fptr,"%d",&tournament.buy_in);
  fscanf(fptr,"%d",&tournament.entry_fee);
  fscanf(fptr,"%d",&tournament.first_place_prize);
  fscanf(fptr,"%d",&tournament.second_place_prize);

  fclose(fptr);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 4;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %d %d",
      &num_first_places,&num_second_places,&num_tournaments);

    retval = tournament_wif(&tournament,
      num_first_places,num_second_places,num_tournaments,&delta);

    if (retval) {
      printf("tournament_wif failed: %d\n",retval);
      return 5;
    }

    if (!bVerbose) {
      if (!bRightJustify)
        printf("%d\n",delta);
      else
        printf("%7d\n",delta);
    }
    else
      printf("%7d %s\n",delta,line);
  }

  fclose(fptr);

  return 0;
}

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
{
  int chara;
  int local_line_len;

  local_line_len = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == '\n')
      break;

    if (local_line_len < maxllen - 1)
      line[local_line_len++] = (char)chara;
  }

  line[local_line_len] = 0;
  *line_len = local_line_len;
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
