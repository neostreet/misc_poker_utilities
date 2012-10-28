#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: place_streak lowest_place filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int lowest_place;
  FILE *fptr;
  int line_len;
  int line_no;
  int place;
  int streak_start;
  int streak_len;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&lowest_place);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  line_no = 0;
  streak_start = -1;
  streak_len = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d",&place);

    if (place <= lowest_place) {
      if (!streak_len)
        streak_start = line_no;

      streak_len++;
    }
    else if ((streak_start != -1) && (streak_len != 0) &&
      (streak_start + streak_len == line_no)) {
      printf("%3d: %2d\n",streak_start,streak_len);
      streak_len = 0;
    }

    line_no++;
  }

  if (streak_len)
    printf("%3d: %2d\n",streak_start,streak_len);

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
