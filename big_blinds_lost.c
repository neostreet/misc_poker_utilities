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
char line[MAX_LINE_LEN];

static char usage[] =
"usage: big_blinds_lost (-debug) (-verbose) (-get_date_from_cwd) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_cwd(char *cwd,char **date_string_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  char *date_string;
  bool bGetDateFromCwd;
  int retval;
  FILE *fptr;
  int linelen;
  int line_no;
  int work;
  int total;
  int curr_big_blind;
  int last_big_blind;
  double big_blinds_lost;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bGetDateFromCwd = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-get_date_from_cwd"))
      bGetDateFromCwd = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bDebug || bGetDateFromCwd)
    getcwd(save_dir,_MAX_PATH);

  if (bGetDateFromCwd) {
    retval = get_date_from_cwd(save_dir,&date_string);

    if (retval) {
      printf("get_date_from_cwd() failed: %d\n",retval);
      return 3;
    }
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;

  total = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&linelen,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %d",&work,&curr_big_blind);

    if (line_no > 1) {
      if (curr_big_blind != last_big_blind) {
        printf("big_blind mismatch\n");
        return 5;
      }
    }

    last_big_blind = curr_big_blind;

    if (work < 0) {
      total += work * - 1;

      if (bVerbose)
        printf("%d %s\n",total,line);
    }
  }

  fclose(fptr);

  big_blinds_lost = (double)total / (double)last_big_blind;

  if (!bVerbose) {
    if (!bDebug) {
      if (!bGetDateFromCwd)
        printf("%lf (%d)\n",big_blinds_lost,line_no);
      else
        printf("%lf\t%s (%d)\n",big_blinds_lost,date_string,line_no);
    }
    else {
      if (!bGetDateFromCwd)
        printf("%lf %s/%s (%d)\n",big_blinds_lost,save_dir,argv[curr_arg],line_no);
      else
        printf("%lf\t%s %s/%s (%d)\n",big_blinds_lost,date_string,save_dir,argv[curr_arg],line_no);
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
