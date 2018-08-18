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
"usage: first_hand_win (-not) (-terse) (-no_others) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bNot;
  bool bTerse;
  bool bNoOthers;
  bool bMetCriteria;
  FILE *fptr;
  int line_no;
  int line_len;
  int retval;
  char *date_string;
  int val;
  int first_hand_win_val;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bNot = false;
  bTerse = false;
  bNoOthers = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-no_others"))
      bNoOthers = true;
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

  getcwd(save_dir,_MAX_PATH);

  retval = get_date_from_path(save_dir,'/',2,&date_string);

  if (retval) {
    printf("get_date_from_path() failed: %d\n",retval);
    return 4;
  }

  line_no = 0;
  bMetCriteria = false;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&val);

    if (line_no == 1) {
      if ((!bNot && (val > 0)) || (bNot && (val < 0))) {
        first_hand_win_val = val;
        bMetCriteria = true;
      }
    }
    else {
      if ((!bNot && (val > 0)) || (bNot && (val < 0)))
        bMetCriteria = false;
    }

    if (!bNoOthers)
      break;
  }

  fclose(fptr);

  if (bMetCriteria) {
    if (!bTerse)
      printf("%10d %s\n",first_hand_win_val,date_string);
    else
      printf("%s\n",date_string);
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
