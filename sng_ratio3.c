#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_PLACES 6
int place_counts[NUM_PLACES];

static char usage[] = "usage: sng_ratio3 filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  int curr_place;
  double dwork;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  for (n = 0; n < NUM_PLACES; n++)
    place_counts[n] = 0;

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&curr_place);

    if ((curr_place < 1) || (curr_place > NUM_PLACES)) {
      printf("invalid place of %d on line %d\n",curr_place,line_no);
      return 3;
    }

    place_counts[curr_place - 1]++;
  }

  fclose(fptr);

  for (n = 0; n < NUM_PLACES; n++) {
    dwork = (double)place_counts[n] / (double)line_no;

    printf("%d %5d %5.2lf\n",n+1,place_counts[n],dwork);
  }

  printf("\n  %5d\n",line_no);

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
