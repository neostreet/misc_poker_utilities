#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[2][MAX_LINE_LEN];

static char usage[] = "usage: add_bounties file bounty_file\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr[2];
  int line_len[2];
  int line_no;
  int buy_in;
  int entry_fee;
  int num_players;
  int num_hands;
  int num_entries;
  int places_paid;
  int place;
  int winnings;
  int bounty;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  for (n = 0; n < 2; n++) {
    if ((fptr[n] = fopen(argv[1+n],"r")) == NULL) {
      printf(couldnt_open,argv[1+n]);
      return 2;
    }
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr[0],line[0],&line_len[0],MAX_LINE_LEN);

    if (feof(fptr[0]))
      break;

    line_no++;

    if (line_no <= 2)
      printf("%s\n",line[0]);
    else {
      GetLine(fptr[1],line[1],&line_len[1],MAX_LINE_LEN);

      if (feof(fptr[1]))
        break;

      line[0][12] = 0;

      sscanf(&line[0][13],"%d %d %d %d %d %d %d %d",
        &buy_in,&entry_fee,&num_players,
        &num_hands,&num_entries,&places_paid,&place,&winnings);

      sscanf(line[1],"%d",&bounty);

      printf("%s %6d %5d %7d %5d %7d %4d %5d %8d\n",
        line[0],buy_in,entry_fee,num_players,
        num_hands,num_entries,places_paid,place,winnings+bounty);
    }
  }

  for (n = 0; n < 2; n++)
    fclose(fptr[n]);

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
