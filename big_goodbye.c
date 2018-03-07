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
"usage: big_goodbye (-debug) (-verbose) (-neg) (-either) (-highbye) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bNeg;
  bool bEither;
  bool bHighBye;
  bool bHit;
  FILE *fptr;
  int line_len;
  int line_no;
  int work;
  int abs_work;
  int max_abs;
  int max_abs_sign;
  int max_abs_line_no;
  int runtot;
  int max_runtot;
  int max_runtot_line_no;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bNeg = false;
  bEither = false;
  bHighBye = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-neg"))
      bNeg = true;
    else if (!strcmp(argv[curr_arg],"-either"))
      bEither = true;
    else if (!strcmp(argv[curr_arg],"-highbye"))
      bHighBye = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bNeg && bEither) {
    printf("can's specify both -neg and -either\n");
    return 3;
  }

  if (bHighBye && bEither) {
    printf("can's specify both -highbye and -either\n");
    return 4;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 5;
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

    if ((line_no == 1) || (runtot > max_runtot)) {
      max_runtot = runtot;
      max_runtot_line_no = line_no;
    }

    if (work < 0)
      abs_work = work * -1;
    else
      abs_work = work;

    if ((line_no == 1) || (abs_work > max_abs)) {
      max_abs = abs_work;
      max_abs_sign = ((work < 0) ? -1 : 1);
      max_abs_line_no = line_no;
    }
  }

  fclose(fptr);

  if (max_abs_line_no == line_no) {
    bHit = false;

    if (bHighBye) {
      if (max_runtot_line_no == line_no - 1) {
        bHit = true;

        if (bDebug) {
          printf("line_no = %d\n",line_no);
          printf("max_runtot = %d\n",max_runtot);
          printf("max_runtot_line_no = %d\n",max_runtot_line_no);
        }
      }
    }
    else if (bEither || (!bNeg && (max_abs_sign == 1)) || (bNeg && (max_abs_sign == -1)))
      bHit = true;

    if (bHit) {
      if (!bVerbose)
        printf("%d\n",max_abs * max_abs_sign);
      else
        printf("%d %d %s\n",max_abs * max_abs_sign,max_abs_line_no,save_dir);
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
