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

static char usage[] = "usage: big_goodbye (-verbose) (-neg) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  bool bNeg;
  FILE *fptr;
  int line_len;
  int line_no;
  int work;
  int abs_work;
  int max_abs;
  int max_abs_sign;
  int max_abs_line_no;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bNeg = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-neg"))
      bNeg = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&work);

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
    if ((!bNeg && (max_abs_sign == 1)) || (bNeg && (max_abs_sign == -1))) {
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
