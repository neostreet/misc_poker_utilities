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
"usage: outlier_factor (-debug) (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  FILE *fptr;
  int num_winning_hands;
  int total_winning_deltas;
  int max_winning_delta;
  int work;
  double avg_gain;
  double outlier_factor;
  int line_len;
  int line_no;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-verbose"))
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

  line_no = 0;
  num_winning_hands = 0;
  total_winning_deltas = 0;
  max_winning_delta = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&work);

    if (work > 0) {
      num_winning_hands++;
      total_winning_deltas += work;

      if (work > max_winning_delta)
        max_winning_delta = work;
    }
  }

  fclose(fptr);

  if (num_winning_hands > 1) {
    total_winning_deltas -= max_winning_delta;
    avg_gain = (double)total_winning_deltas / (double)(num_winning_hands - 1);
    outlier_factor = (double)max_winning_delta / avg_gain;

    if (!bDebug) {
      if (!bVerbose)
        printf("%lf\n",outlier_factor);
      else
        printf("%lf (%d %lf)\n",outlier_factor,max_winning_delta,avg_gain);
    }
    else {
      if (!bVerbose)
        printf("%lf %s\n",outlier_factor,save_dir);
      else {
        printf("%lf (%d %lf) %s\n",outlier_factor,max_winning_delta,
          avg_gain,save_dir);
      }
    }
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
