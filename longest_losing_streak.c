#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: skeleton filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  FILE *fptr;
  int line_len;
  int line_no;
  int delta;
  int curr_losing_streak;
  int longest_losing_streak;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;
  curr_losing_streak = 0;
  longest_losing_streak = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&delta);

    if (delta < 0)
      curr_losing_streak++;
    else {
      if (curr_losing_streak) {
        if (curr_losing_streak > longest_losing_streak)
          longest_losing_streak = curr_losing_streak;

        curr_losing_streak = 0;
      }
    }
  }

  fclose(fptr);

  printf("%d\n",longest_losing_streak);

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
