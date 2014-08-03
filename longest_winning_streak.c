#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char save_dir[_MAX_PATH];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: longest_winning_streak (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  FILE *fptr;
  int line_len;
  int line_no;
  int delta;
  int curr_winning_streak;
  int curr_winning_streak_sum_delta;
  int longest_winning_streak;
  int longest_winning_streak_sum_delta;

  if ((argc < 2) || (argc > 3)) {
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

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  if (bVerbose)
    getcwd(save_dir,_MAX_PATH);

  line_no = 0;
  curr_winning_streak = 0;
  curr_winning_streak_sum_delta = 0;
  longest_winning_streak = 0;
  longest_winning_streak_sum_delta = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&delta);

    if (delta > 0) {
      curr_winning_streak++;
      curr_winning_streak_sum_delta += delta;
    }
    else {
      if (curr_winning_streak) {
        if (curr_winning_streak > longest_winning_streak) {
          longest_winning_streak = curr_winning_streak;
          longest_winning_streak_sum_delta = curr_winning_streak_sum_delta;
        }

        curr_winning_streak = 0;
        curr_winning_streak_sum_delta = 0;
      }
    }
  }

  fclose(fptr);

  if (!bVerbose)
    printf("%d (%d)\n",longest_winning_streak,longest_winning_streak_sum_delta);
  else {
    printf("%d (%d) %s\n",longest_winning_streak,longest_winning_streak_sum_delta,
      save_dir);
  }

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
