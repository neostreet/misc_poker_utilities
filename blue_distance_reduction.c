#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: blue_distance_reduction filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

#define DATE_LEN 10

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int last_blue_distance;
  int blue_distance;
  int blue_distance_reduction;
  char date[DATE_LEN + 1];

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

    line_no++;

    sscanf(line,"%d\t%s",&blue_distance,date);

    if (line_no > 1) {
      if (blue_distance < last_blue_distance) {
        blue_distance_reduction = last_blue_distance - blue_distance;
        printf("%d\t%s\n",blue_distance_reduction,date);
      }
    }

    last_blue_distance = blue_distance;
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
