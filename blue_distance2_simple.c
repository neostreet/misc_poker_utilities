#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: blue_distance2_simple (-pct) (-verbose) initial_bal filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bPct;
  bool bVerbose;
  int initial_bal;
  FILE *fptr;
  int line_len;
  int line_no;
  char str[MAX_STR_LEN];
  int delta;
  int balance;
  int max_balance;
  int blue_distance;
  double dwork;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bPct = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-pct"))
      bPct = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&initial_bal);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 2;
  }

  line_no = 0;
  balance = initial_bal;
  max_balance = initial_bal;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%s\t%d",str,&delta);

    balance += delta;

    if (balance > max_balance)
      max_balance = balance;

    if (max_balance > 0)
      blue_distance = max_balance - balance;
    else
      blue_distance = max_balance * -1;

    if (!bPct)
      printf("%d\t%s\n",blue_distance,line);
    else {
      if (blue_distance) {
        dwork = (double)blue_distance / (double)max_balance;

        if (!bVerbose)
          printf("%lf\t%s\n",dwork,line);
        else
          printf("%lf (%d %d)\t%s\n",dwork,blue_distance,max_balance,line);
      }
    }

    line_no++;
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
