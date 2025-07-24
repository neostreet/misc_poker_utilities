#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: flop_pct player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char pokerstars[] = "PokerStars";
static char folded_before_flop_str[] = "folded before Flop";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int num_hands;
  int folded_before_flop;
  int folded_before_flop_count;
  int saw_flop_count;
  double dwork;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  folded_before_flop = 0;
  folded_before_flop_count = 0;
  line_no = 0;
  num_hands = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if (strstr(line,pokerstars)) {
      num_hands++;

      if (folded_before_flop == 1)
        folded_before_flop_count++;

      folded_before_flop = 0;
    }
    else if (strstr(line,argv[1]) && strstr(line,folded_before_flop_str))
      folded_before_flop = 1;
  }

  if (folded_before_flop == 1)
    folded_before_flop_count++;

  fclose(fptr);

  saw_flop_count = num_hands - folded_before_flop_count;

  if (!num_hands)
    dwork = (double)0;
  else
    dwork = (double)saw_flop_count / (double)num_hands * (double)100;

  printf("%6.2lf%% (saw flop %d times in %d hands)\n",dwork,saw_flop_count,num_hands);

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
