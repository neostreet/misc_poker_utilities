#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: blue_points (-verbose) (-starting_amountstarting_amount)\n"
"  (-after_blue) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  int starting_amount;
  bool bAfterBlue;
  int balance;
  int winning_sessions;
  int losing_sessions;
  int golden;
  int golden_count;
  FILE *fptr;
  int line_len;
  int line_no;
  int blue_count;
  int work;
  int max;
  double pct;
  int prev_blue;
  int blue_gap;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  starting_amount = 0;
  bAfterBlue = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strncmp(argv[curr_arg],"-starting_amount",16))
      sscanf(&argv[curr_arg][16],"%d",&starting_amount);
    else if (!strcmp(argv[curr_arg],"-after_blue"))
      bAfterBlue = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  line_no = 0;
  blue_count = 0;
  balance = starting_amount;
  max = starting_amount;
  winning_sessions = 0;
  losing_sessions = 0;
  golden_count = 0;
  prev_blue = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d",&work);

    if (work > 0)
      winning_sessions++;
    else
      losing_sessions++;

    balance += work;

    if (balance > max) {
      blue_count++;

      if (losing_sessions * 2 <= winning_sessions) {
        golden = 1;
        golden_count++;
      }
      else
        golden = 0;

      max = balance;

      if (prev_blue != -1) {
        if (!bAfterBlue) {
          if (!bVerbose)
            printf("%d %s\n",balance,line);
          else {
            blue_gap = line_no - prev_blue;
            printf("%6d: %12d %6d %6d (%d) %3d\n",
              line_no,balance,losing_sessions,winning_sessions,golden,blue_gap);
          }
        }
      }
      else {
        if (!bVerbose)
          printf("%d %s\n",balance,line);
        else {
          printf("%6d: %12d %6d %6d (%d)\n",
            line_no,balance,losing_sessions,winning_sessions,golden);
        }
      }

      prev_blue = line_no;
    }

    if (bAfterBlue && (line_no == prev_blue + 1))
      printf("%s\n",line);

    line_no++;
  }

  fclose(fptr);

  if (bVerbose) {
    pct = (double)blue_count / (double)line_no;
    printf("blue: %d of %d (%lf)\n",blue_count,line_no,pct);
    pct = (double)golden_count / (double)blue_count;
    printf("golden when blue: %d of %d (%lf)\n",golden_count,blue_count,pct);
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
