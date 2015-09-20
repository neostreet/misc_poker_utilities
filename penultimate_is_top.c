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
"usage: penultimate_is_top (-verbose) (-date_string) (-final_delta) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  bool bDateString;
  bool bFinalDelta;
  FILE *fptr;
  int line_len;
  int line_no;
  int penultimate_line_no;
  int retval;
  char *date_string;
  int val;
  int ending_balance;
  int max_ending_balance;
  int penultimate_ending_balance;
  int final_delta;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bDateString = false;
  bFinalDelta = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-date_string"))
      bDateString = true;
    else if (!strcmp(argv[curr_arg],"-final_delta"))
      bFinalDelta = true;
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

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
  }

  penultimate_line_no = line_no - 1;

  fseek(fptr,0L,SEEK_SET);

  line_no = 0;
  ending_balance = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;
    sscanf(line,"%d",&val);

    if (line_no == 1) {
      ending_balance = val;
      max_ending_balance = val;
    }
    else {
      ending_balance += val;

      if (ending_balance > max_ending_balance)
        max_ending_balance = ending_balance;

      if (line_no == penultimate_line_no)
        penultimate_ending_balance = ending_balance;
      else if (line_no == penultimate_line_no + 1)
        final_delta = val;
    }
  }

  fclose(fptr);

  if (penultimate_ending_balance == max_ending_balance) {
    if (!bVerbose) {
      if (!bDateString)
        printf("%s/%s",save_dir,argv[curr_arg]);
      else
        printf("%s",date_string);
    }
    else {
      if (!bDateString)
        printf("%10d %s/%s",ending_balance,save_dir,argv[curr_arg]);
      else
        printf("%10d %s",ending_balance,date_string);
    }

    if (!bFinalDelta)
      putchar(0x0a);
    else
      printf(" (%10d)\n",final_delta);
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
