#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

struct tournament_info {
  int buy_in;
  int entry_fee;
  int first_place_prize;
  int second_place_prize;
};

static char usage[] =
"usage: tournament_wif_detail (-verbose) tournament_info_file place_file\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  FILE *fptr;
  int line_len;
  struct tournament_info tournament;
  int place;
  int sng_balance;
  int buy_in_plus_entry_fee;
  int delta;
  int num_firsts;
  int num_seconds;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
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

  sng_balance = 0;
  buy_in_plus_entry_fee = (tournament.buy_in + tournament.entry_fee) * -1;

  if (bVerbose) {
    num_firsts = 0;
    num_seconds = 0;
  }

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d",&place);

    delta = buy_in_plus_entry_fee;

    if (place == 1) {
      delta += tournament.first_place_prize;

      if (bVerbose)
        num_firsts++;
    }
    else if (place == 2) {
      delta += tournament.second_place_prize;

      if (bVerbose)
        num_seconds++;
    }

    sng_balance += delta;

    if (!bVerbose)
      printf("%d\n",sng_balance);
    else
      printf("%10d %6d\n",sng_balance,delta);
  }

  fclose(fptr);

  if (bVerbose) {
    printf("\n%d num_firsts\n",num_firsts);
    printf("%d num_seconds\n",num_seconds);
  }

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
