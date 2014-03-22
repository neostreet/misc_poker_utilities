#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];
static char save_line[MAX_LINE_LEN];
static char max_streak_save_line[MAX_LINE_LEN];

static char usage[] =
"usage: place_streak (-verbose) lowest_place filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  int lowest_place;
  FILE *fptr;
  int line_len;
  int line_no;
  int place;
  int streak_start;
  int streak_len;
  int max_streak_len;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&lowest_place);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 3;
  }

  line_no = 0;
  streak_start = -1;
  streak_len = 0;

  if (bVerbose)
    max_streak_len = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d",&place);

    if (place <= lowest_place) {
      if (!streak_len)
        streak_start = line_no;

      streak_len++;
      strcpy(save_line,line);

      if (bVerbose) {
        if (streak_len > max_streak_len) {
          max_streak_len = streak_len;
          strcpy(max_streak_save_line,line);
        }
      }
    }
    else if ((streak_start != -1) && (streak_len != 0) &&
      (streak_start + streak_len == line_no)) {

      if (!bVerbose)
        printf("%d\n",streak_len);
      else
        printf("%2d %s\n",streak_len,save_line);

      streak_len = 0;
    }

    line_no++;
  }

  if (streak_len) {
    if (!bVerbose)
      printf("%d\n",streak_len);
    else
      printf("%2d %s\n",streak_len,save_line);
  }

  fclose(fptr);

  if (bVerbose)
    printf("\n%2d %s\n",max_streak_len,max_streak_save_line);

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
