#include <stdio.h>
#include <string.h>

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: fflop_pct player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char pokerstars[] = "PokerStars";
static char folded_before_flop_str[] = "folded before Flop";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int num_hands;
  int folded_before_flop;
  int folded_before_flop_count;
  int saw_flop_count;
  double dwork;
  int total_num_hands;
  int total_saw_flop_count;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  if ((fptr0 = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  total_num_hands = 0;
  total_saw_flop_count = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
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

    total_saw_flop_count += saw_flop_count;
    total_num_hands += num_hands;

    if (!num_hands)
      dwork = (double)0;
    else
      dwork = (double)saw_flop_count / (double)num_hands * (double)100;

    printf("%6.2lf%% (saw flop %d times in %d hands) %s\n",dwork,saw_flop_count,num_hands,filename);
  }

  fclose(fptr0);

  if (!total_num_hands)
    dwork = (double)0;
  else
    dwork = (double)total_saw_flop_count / (double)total_num_hands * (double)100;

  printf("\n%6.2lf%% (saw flop %d times in %d total hands)\n",dwork,total_saw_flop_count,total_num_hands);

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
