#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: is_blue (-starting_amountstarting_amount) (-terse) (-verbose)\n"
"  (-not) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  int starting_amount;
  bool bTerse;
  bool bVerbose;
  bool bNot;
  int balance;
  FILE *fptr;
  int line_len;
  int line_no;
  int blue_count;
  int work;
  int max;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  starting_amount = 0;
  bTerse = false;
  bVerbose = false;
  bNot = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strncmp(argv[curr_arg],"-starting_amount",16))
      sscanf(&argv[curr_arg][16],"%d",&starting_amount);
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bTerse && bVerbose) {
    printf("can only specify one of -terse and -verbose\n");
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;
  blue_count = 0;
  balance = starting_amount;
  max = starting_amount;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d",&work);

    balance += work;

    if (balance > max) {
      blue_count++;
      max = balance;

      if (bTerse) {
        if (!bNot)
          printf("1\n");
        else
          printf("0\n");
      }
      else if (bVerbose) {
        if (!bNot)
          printf("1 %d %s\n",balance,line);
        else
          printf("0 %d %s\n",balance,line);
        }
      else {
        if (!bNot)
          printf("1 %s\n",line);
        else
          printf("0 %s\n",line);
      }
    }
    else {
      if (bTerse) {
        if (!bNot)
          printf("0\n");
        else
          printf("1\n");
      }
      else if (bVerbose) {
        if (!bNot)
          printf("0 %d %s\n",balance,line);
        else
          printf("1 %d %s\n",balance,line);
      }
      else {
        if (!bNot)
          printf("0 %s\n",line);
        else
          printf("1 %s\n",line);
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
