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

static char usage[] = "usage: tgotl (-debug) (-verbose) (-silent) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bSilent;
  int retval;
  char *date_string;
  FILE *fptr;
  int work;
  int total_winning_delta;
  int total_losing_delta;
  double tgotl;
  int line_len;
  int line_no;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bSilent = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug")) {
      bDebug = true;
      getcwd(save_dir,_MAX_PATH);

      retval = get_date_from_path(save_dir,'/',2,&date_string);

      if (retval) {
        printf("get_date_from_path() failed: %d\n",retval);
        return 2;
      }
    }
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-silent"))
      bSilent = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  line_no = 0;
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

    if (work > 0)
      total_winning_delta += work;
    else {
      work *= -1;
      total_losing_delta += work;
    }

    if (bVerbose) {
      tgotl = (double)total_winning_delta / total_losing_delta;

      if (!bDebug)
        printf("%lf\n",tgotl);
      else {
        printf("%10.4lf %10d %10d\n",tgotl,
          total_winning_delta,total_losing_delta);
      }
    }
  }

  fclose(fptr);

  if (!total_losing_delta) {
    if (!bSilent) {
      printf("no losing hands\n");
      return 5;
    }
    else
      return 0;
  }

  if (!bVerbose) {
    tgotl = (double)total_winning_delta / total_losing_delta;

    if (!bDebug)
      printf("%lf\t%s\n",tgotl,date_string);
    else {
      printf("%10.4lf %10d %10d\t%s\n",tgotl,
        total_winning_delta,total_losing_delta,date_string);
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

static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr)
{
  int n;
  int len;
  int slash_count;

  len = strlen(path);
  slash_count = 0;

  for (n = len - 1; (n >= 0); n--) {
    if (path[n] == slash_char) {
      slash_count++;

      if (slash_count == num_slashes)
        break;
    }
  }

  if (slash_count != num_slashes)
    return 1;

  if (path[n+5] != slash_char)
    return 2;

  strncpy(sql_date_string,&path[n+1],4);
  sql_date_string[4] = '-';
  strncpy(&sql_date_string[5],&path[n+6],2);
  sql_date_string[7] = '-';
  strncpy(&sql_date_string[8],&path[n+8],2);
  sql_date_string[10] = 0;

  *date_string_ptr = sql_date_string;

  return 0;
}
