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
"usage: max_underwater_depth (-debug) (-verbose)\n"
"  (-only_winning) (-only_losing) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bOnlyWinning;
  bool bOnlyLosing;
  int val;
  FILE *fptr;
  int line_len;
  int line_no;
  int work;
  int runtot;
  int max_depth;
  int max_depth_ix;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bOnlyWinning = false;
  bOnlyLosing = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-only_winning"))
      bOnlyWinning = true;
    else if (!strcmp(argv[curr_arg],"-only_losing"))
      bOnlyLosing = true;
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

  if (bOnlyWinning && bOnlyLosing) {
    printf("can't specify both -only_winning and -only_losing\n");
    return 4;
  }

  line_no = 0;
  runtot = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&work);

    runtot += work;

    if ((line_no == 1) || (runtot < max_depth)) {
      max_depth = runtot;
      max_depth_ix = line_no - 1;
    }
  }

  fclose(fptr);

  if (!bOnlyWinning || (runtot > 0)) {
    if (!bOnlyLosing || (runtot < 0)) {
      if (max_depth < 0) {
        if (!bDebug)
          printf("%d\n",max_depth);
        else {
          printf("%d %s hand %d\n",max_depth,
            save_dir,max_depth_ix + 1);
        }
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
