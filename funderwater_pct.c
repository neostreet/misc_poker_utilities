#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char filename[MAX_LINE_LEN];
static char line[MAX_LINE_LEN];

static char usage[] = "usage: funderwater_pct (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  FILE *fptr0;
  FILE *fptr;
  int line_len;
  int hand_count;
  int underwater_count;
  int balance;
  int delta;
  double underwater_pct;

  if ((argc < 2) || (argc > 3)) {
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

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  for ( ; ; ) {
    GetLine(fptr0,filename,&line_len,MAX_LINE_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      return 4;
    }

    hand_count = 0;
    underwater_count = 0;
    balance = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      hand_count++;

      sscanf(line,"%d",&delta);

      balance += delta;

      if (balance < 0)
        underwater_count++;

      if (bVerbose) {
        if (hand_count == 1)
          printf("%s\n",filename);

        printf("  %d (%d)\n",(balance < 0),hand_count);
      }
    }

    fclose(fptr);

    underwater_pct = (double)underwater_count / (double)hand_count;

    printf("%lf (%d %d) %s\n",underwater_pct,underwater_count,hand_count,filename);
  }

  fclose(fptr0);

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
