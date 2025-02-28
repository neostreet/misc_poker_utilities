#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: missing_months filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  int prev_year;
  int prev_month;
  int year;
  int month;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  prev_year = -1;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d-%d",&year,&month);

    if (prev_year == -1)
      ;
    else if (year != prev_year) {
      for (n = prev_month + 1; n <= 12; n++)
        printf("%d-%02d\n",prev_year,n);

      for (n = prev_year + 1; n < year; n++) {
        for (m = 1; m <= 12; m++)
          printf("%d-%02d\n",n,m);
      }

      for (n = 1; n < month; n++)
        printf("%d-%02d\n",year,n);
    }
    else if (month > prev_month + 1) {
      for (n = prev_month + 1; n < month; n++)
        printf("%d-%02d\n",year,n);
    }

    prev_year = year;
    prev_month = month;
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
