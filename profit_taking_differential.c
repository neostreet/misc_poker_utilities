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
"usage: profit_taking_differential (-verbose) pct in_chips_file delta_file\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  int pct;
  double dpct;
  FILE *fptr;
  int line_len;
  int line_no;
  int starting_stack;
  int stack;
  int work;
  double dcutoff;
  int cutoff;
  int profit_taking_amount;
  int profit_taking_line_no;
  int session_delta;

  if ((argc < 4) || (argc > 5)) {
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

  if (argc - curr_arg != 3) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&pct);

  if ((fptr = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 3;
  }

  GetLine(fptr,line,&line_len,MAX_LINE_LEN);

  if (feof(fptr)) {
    printf("no lines in in_chips_file %s\n",argv[curr_arg+1]);
    fclose(fptr);
    return 4;
  }

  fclose(fptr);

  sscanf(line,"%d",&starting_stack);

  dcutoff = (double)starting_stack * ((double)1 + (double)pct / (double)100);
  cutoff = dcutoff;

  if ((fptr = fopen(argv[curr_arg+2],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+2]);
    return 5;
  }

  getcwd(save_dir,_MAX_PATH);
  dpct = (double)pct;

  line_no = 0;
  profit_taking_line_no = 0;
  stack = starting_stack;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
    sscanf(line,"%d",&work);
    stack += work;

    if ((line_no > 1) && !profit_taking_line_no) {
      if (stack >= cutoff) {
        profit_taking_amount = stack - starting_stack;
        profit_taking_line_no = line_no;
      }
    }
  }

  fclose(fptr);

  if (profit_taking_line_no) {
    session_delta = stack - starting_stack;

    if (!bVerbose) {
      printf("%10d %s (%d)\n",
        profit_taking_amount - session_delta,save_dir,profit_taking_line_no);
    }
    else {
      printf("%10d (%10d %10d) %s (%d)\n",
        profit_taking_amount - session_delta,
        profit_taking_amount,session_delta,
        save_dir,profit_taking_line_no);
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
