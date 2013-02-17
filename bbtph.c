#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: bbtph bb_amount filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int bb_amount;
  FILE *fptr;
  int line_len;
  int traversed;
  int num_hands;
  int val;
  double bbtph;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&bb_amount);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  traversed = 0;
  num_hands = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    num_hands++;

    sscanf(line,"%d",&val);

    if (val < 0)
      val *= -1;

    traversed += val;
  }

  fclose(fptr);

  bbtph = (double)traversed / (double)num_hands / (double)bb_amount;

  printf("%lf bbtph\n",bbtph);

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
