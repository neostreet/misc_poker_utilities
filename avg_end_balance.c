#include <stdio.h>
#include <stdlib.h>
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
"usage: avg_end_balance (-debug) (-verbose) (-only_winning) (-only_losing) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_cwd(char *cwd,char **date_string_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bOnlyWinning;
  bool bOnlyLosing;
  FILE *fptr;
  int retval;
  char *date_string;
  int line_len;
  int line_no;
  int delta;
  int balance;
  int sum_end_balances;
  double dwork;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bOnlyWinning = false;
  bOnlyLosing = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
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

  if (bOnlyWinning && bOnlyLosing) {
    printf("can't specify both -only_winning and -only_losing\n");
    return 3;
  }

  balance = 0;
  sum_end_balances = 0;

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  getcwd(save_dir,_MAX_PATH);

  retval = get_date_from_cwd(save_dir,&date_string);

  if (retval) {
    printf("get_date_from_cwd() failed: %d\n",retval);
    return 5;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&delta);

    balance += delta;
    sum_end_balances += balance;

    if (bDebug)
      printf("%10d %10d %10d\n",delta,balance,sum_end_balances);
  }

  fclose(fptr);

  if (!bOnlyWinning || (balance > 0)) {
    if (!bOnlyLosing || (balance < 0)) {
      dwork = (double)sum_end_balances / (double)line_no;

      if (!bVerbose)
        printf("%lf %s\n",dwork,date_string);
      else
        printf("%lf (%d %d) %s\n",dwork,sum_end_balances,line_no,date_string);
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

static char sql_date_string[11];

static int get_date_from_cwd(char *cwd,char **date_string_ptr)
{
  int n;
  int len;
  int slash_count;

  len = strlen(cwd);
  slash_count = 0;

  for (n = len - 1; (n >= 0); n--) {
    if (cwd[n] == '/') {
      slash_count++;

      if (slash_count == 2)
        break;
    }
  }

  if (slash_count != 2)
    return 1;

  if (cwd[n+5] != '/')
    return 2;

  strncpy(sql_date_string,&cwd[n+1],4);
  sql_date_string[4] = '-';
  strncpy(&sql_date_string[5],&cwd[n+6],2);
  sql_date_string[7] = '-';
  strncpy(&sql_date_string[8],&cwd[n+8],2);
  sql_date_string[10] = 0;

  *date_string_ptr = sql_date_string;

  return 0;
}
