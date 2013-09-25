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
"usage: agoal (-debug) (-verbose) (-mark_winning_session) (-mark_losing_session) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bMarkWinningSession;
  bool bMarkLosingSession;
  bool bWinningSession;
  FILE *fptr;
  int work;
  int num_winning_hands;
  int num_losing_hands;
  int total_winning_delta;
  int total_losing_delta;
  double avg_gain;
  double avg_loss;
  double agoal;
  int line_len;
  int line_no;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bMarkWinningSession = false;
  bMarkLosingSession = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-mark_winning_session"))
      bMarkWinningSession = true;
    else if (!strcmp(argv[curr_arg],"-mark_losing_session"))
      bMarkLosingSession = true;
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
  num_losing_hands = 0;
  total_winning_delta = 0;
  total_losing_delta = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&work);

    if (!work)
      continue;

    if (work > 0) {
      num_winning_hands++;
      total_winning_delta += work;
    }
    else {
      work *= -1;
      num_losing_hands++;
      total_losing_delta += work;
    }
  }

  if (bMarkWinningSession || bMarkLosingSession) {
    if (total_winning_delta > total_losing_delta)
      bWinningSession = true;
    else
      bWinningSession = false;
  }

  if (!num_winning_hands) {
    printf("no winning hands\n");
    return 4;
  }

  if (!num_losing_hands) {
    printf("no losing hands\n");
    return 5;
  }

  avg_gain = (double)total_winning_delta / (double)num_winning_hands;
  avg_loss = (double)total_losing_delta / (double)num_losing_hands;

  agoal = avg_gain / avg_loss;

  if (!bVerbose) {
    if (!bDebug) {
      if (bMarkWinningSession && bWinningSession)
        printf("%lf (w) (%d)\n",agoal,line_no);
      else if (bMarkLosingSession && !bWinningSession)
        printf("%lf (l) (%d)\n",agoal,line_no);
      else
        printf("%lf (%d)\n",agoal,line_no);
    }
    else {
      if (bMarkWinningSession && bWinningSession)
        printf("%s %lf (w) (%d)\n",save_dir,agoal,line_no);
      else if (bMarkLosingSession && !bWinningSession)
        printf("%s %lf (l) (%d)\n",save_dir,agoal,line_no);
      else
        printf("%s %lf (%d)\n",save_dir,agoal,line_no);
    }
  }
  else {
    printf("   %7d total_winning_delta\n",total_winning_delta);
    printf("   %7d num_winning_hands\n",num_winning_hands);
    printf("%10.2lf avg_gain\n\n",avg_gain);

    printf("   %7d total_losing_delta\n",total_losing_delta);
    printf("   %7d num_losing_hands\n",num_losing_hands);
    printf("%10.2lf avg_loss\n\n",avg_loss);

    if (bMarkWinningSession && bWinningSession)
      printf("%10.2lf agoal (w)\n\n",agoal);
    else if (bMarkLosingSession && !bWinningSession)
      printf("%10.2lf agoal (l)\n\n",agoal);
    else
      printf("%10.2lf agoal\n\n",agoal);
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
