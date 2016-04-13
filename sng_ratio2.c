#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: sng_ratio2 place filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int place;
  FILE *fptr;
  int line_len;
  int line_no;
  int curr_place;
  int num;
  int denom;
  double sng_ratio;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&place);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  num = 0;
  denom = 0;
  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&curr_place);

    if ((curr_place < 1) || (curr_place > 6)) {
      printf("invalid place of %d on line %d\n",curr_place,line_no);
      return 3;
    }

    denom++;

    if (curr_place == place)
      num++;

    printf("%d",curr_place);

    sng_ratio = (double)num / (double)denom;
    printf(" %lf\n",sng_ratio);
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
