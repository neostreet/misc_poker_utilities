#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: format_stats poker_flavor filename\n";
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

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
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

        printf("During current %s session you were dealt %d hands and saw flop:\n",
          argv[1],hands);

        break;
      case 1:
        sscanf(line,"%d %d",&numerator,&denominator);

        if (!denominator)
          percentage = 0;
        else {
          percentage = numerator * 100;
          percentage /= denominator;
        }

        printf(" - %d out of %d times while in big blind (%d%%)\n",
          numerator,denominator,percentage);

        break;
      case 2:
        sscanf(line,"%d %d",&numerator,&denominator);

        if (!denominator)
          percentage = 0;
        else {
          percentage = numerator * 100;
          percentage /= denominator;
        }

        printf(" - %d out of %d times while in small blind (%d%%)\n",
          numerator,denominator,percentage);

        break;
      case 3:
        sscanf(line,"%d %d",&numerator,&denominator);

        if (!denominator)
          percentage = 0;
        else {
          percentage = numerator * 100;
          percentage /= denominator;
        }

        printf(" - %d out of %d times in other positions (%d%%)\n",
          numerator,denominator,percentage);

        break;
      case 4:
        sscanf(line,"%d %d",&numerator,&denominator);

        if (!denominator)
          percentage = 0;
        else {
          percentage = numerator * 100;
          percentage /= denominator;
        }

        printf(" - a total of %d out of %d (%d%%)\n",
          numerator,denominator,percentage);

        break;
      case 5:
        sscanf(line,"%d %d",&numerator,&denominator);

        if (!denominator)
          percentage = 0;
        else {
          percentage = numerator * 100;
          percentage /= denominator;
        }

        printf(" Pots won at showdown - %d of %d (%d%%)\n",
          numerator,denominator,percentage);

        break;
      case 6:
        sscanf(line,"%d",&numerator);

        printf(" Pots won without showdown - %d\n",
          numerator);

        break;
    }

    if (line_no == 6)
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
