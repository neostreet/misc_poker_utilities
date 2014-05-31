#include <stdio.h>
#include <string.h>

#define MAX_STR_LEN 256

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: blue_jumps (-verbose) (-initial_bal) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  int initial_bal;
  FILE *fptr;
  int line_len;
  int line_no;
  char str[MAX_STR_LEN];
  int delta;
  int balance;
  int max_balance;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  initial_bal = 0;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strncmp(argv[curr_arg],"-initial_bal",12))
      sscanf(&argv[curr_arg][12],"%d",&initial_bal);
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

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%s\t%d",str,&delta);

    if (!line_no) {
      balance = initial_bal + delta;
      max_balance = balance;
    }
    else {
      balance += delta;

      if (balance > max_balance) {
        if (!bVerbose)
          printf("%d\t%s\n",balance - max_balance,line);
        else
          printf("%6d %10d %s\n",balance - max_balance,balance,line);

        max_balance = balance;
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
