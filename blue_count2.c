#include <stdio.h>

#define FALSE 0
#define TRUE  1

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: blue_count2 (-verbose) filename value\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  int bVerbose;
  FILE *fptr;
  int value;
  int line_len;
  int line_no;
  int instance_count;
  int blue_count;
  int work;
  int work2;
  int max;
  double blue_pct;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = FALSE;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = TRUE;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  sscanf(argv[curr_arg+1],"%d",&value);

  line_no = 0;
  max = -1;
  instance_count = 0;
  blue_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %d",&work,&work2);

    if (work2 == value)
      instance_count++;

    if (work > max) {
      if ((line_no > 1) && (work2 == value)) {
        blue_count++;

        if (bVerbose)
          printf("%d %d\n",instance_count - 1,work);
      }

      max = work;
    }
  }

  fclose(fptr);

  blue_pct = (double)blue_count / (double)instance_count;

  printf("%d of %d (%lf)\n",blue_count,instance_count,blue_pct);

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
