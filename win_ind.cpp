#include <vector>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

using namespace std;

static char save_dir[_MAX_PATH];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: win_ind (-verbose) ind_val filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  double ind_val;
  FILE *fptr;
  int line_len;
  int num_hands;
  bool bVerbose;
  int num_wins;
  int win_count;
  int delta;
  int sum_delta;
  vector<int> deltas;
  int win_target;
  int numerator;
  int denominator;
  double win_ind;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%lf",&ind_val);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 3;
  }

  num_hands = 0;
  num_wins = 0;
  sum_delta = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    num_hands++;

    sscanf(line,"%d",&delta);

    sum_delta += delta;

    if (delta > 0)
      num_wins++;

    deltas.push_back(delta);
  }

  fclose(fptr);

  win_target = (num_wins / 2) + 1;

  win_count = 0;

  for (numerator = 0; numerator < num_hands; numerator++) {
    if (deltas[numerator] > 0) {
      win_count++;

      if (win_count == win_target)
        break;
    }
  }

  denominator = (num_hands + 1) / 2;

  win_ind = (double)numerator / (double)denominator;

  if (win_ind >= ind_val) {
    printf("%8.6lf ",win_ind);
    printf("(%d %d)",numerator,denominator);

    if (sum_delta < 0)
      printf(" false positive");

    if (!bVerbose)
      putchar(0x0a);
    else
      printf(" %s\n",save_dir);
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
