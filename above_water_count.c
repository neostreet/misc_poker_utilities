#include <stdio.h>
#include <string.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: above_water_count (-verbose) (-not) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  bool bNot;
  FILE *fptr;
  int line_len;
  int val;
  int total;
  int count;
  int above_water_count;
  double work;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bNot = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
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

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 2;
  }

  total = 0;
  count = 0;
  above_water_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    count++;

    sscanf(line,"%d",&val);

    total += val;

    if ((!bNot && (total > 0)) || (bNot && (total <= 0))) {
      above_water_count++;

      if (bVerbose)
        printf("%d %s\n",total,line);
    }
  }

  fclose(fptr);

  work = (double)above_water_count / (double)count;

  if (!bVerbose)
    printf("%d of %d (%lf)\n",above_water_count,count,work);
  else
    printf("\n%lf (%d of %d)\n",work,above_water_count,count);

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
