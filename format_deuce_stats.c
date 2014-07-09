#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: format_deuce_stats filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int hands;
  int numerator;
  int denominator;
  int percentage;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    switch (line_no) {
      case 0:
        sscanf(line,"%d",&hands);

        printf("During current 2-7 Triple Draw session you were dealt %d hands\n",hands);

        break;
      case 1:
        sscanf(line,"%d %d",&numerator,&denominator);

        percentage = numerator * 100;
        percentage /= denominator;

        printf(" Pots won if drawing - %d of %d (%d%%)\n",
          numerator,denominator,percentage);

        break;
      case 2:
        sscanf(line,"%d %d",&numerator,&denominator);

        percentage = numerator * 100;
        percentage /= denominator;

        printf(" Pots won at showdown - %d of %d (%d%%)\n",
          numerator,denominator,percentage);

        break;
      case 3:
        sscanf(line,"%d",&numerator);

        printf(" Pots won without showdown - %d\n",
          numerator);

        break;
    }

    if (line_no == 3)
      break;

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
