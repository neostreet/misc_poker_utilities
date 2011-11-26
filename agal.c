#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: agal filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int work;
  int num_winning_hands;
  int num_losing_hands;
  int total_winning_delta;
  int total_losing_delta;
  double avg_gain;
  double avg_loss;
  double agal;
  int line_len;
  int line_no;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  num_winning_hands = 0;
  num_losing_hands = 0;
  total_winning_delta = 0;
  total_losing_delta = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&work);

    if (!work)
      continue;

    if (work > 0) {
      num_winning_hands++;
      total_winning_delta += work;
    }
    else {
      work *= -1;
      num_losing_hands++;
      total_losing_delta += work;
    }
  }

  if (!num_winning_hands) {
    printf("no winning hands\n");
    return 3;
  }

  if (!num_losing_hands) {
    printf("no losing hands\n");
    return 3;
  }

  avg_gain = (double)total_winning_delta / (double)num_winning_hands;
  avg_loss = (double)total_losing_delta / (double)num_losing_hands;

  agal = avg_gain / avg_loss;

  printf("%lf\n",agal);

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
