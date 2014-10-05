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

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: fheads_up_pct (-terse) (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char street_marker[] = "*** ";
#define STREET_MARKER_LEN (sizeof (street_marker) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_cwd(char *cwd,char **date_string_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bTerse;
  bool bVerbose;
  FILE *fptr0;
  int retval;
  char *date_string;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_len;
  int table_count;
  int num_hands;
  int num_heads_up_hands;
  double heads_up_pct;
  int curr_file_num_hands;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bTerse && bVerbose) {
    printf("can't specify both -terse and -verbose\n");
    return 3;
  }

  if (!bTerse) {
    getcwd(save_dir,_MAX_PATH);

    retval = get_date_from_cwd(save_dir,&date_string);

    if (retval) {
      printf("get_date_from_cwd() failed: %d\n",retval);
      return 4;
    }
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 5;
  }

  num_files = 0;
  num_hands = 0;
  num_heads_up_hands = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    curr_file_num_hands = 0;
    num_files++;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      if (!strncmp(line,"Table '",7)) {
        table_count = 0;
        curr_file_num_hands++;

        for ( ; ; ) {
          GetLine(fptr,line,&line_len,MAX_LINE_LEN);

          if (feof(fptr))
            break;

          if (!strncmp(line,street_marker,STREET_MARKER_LEN))
            break;

          if (!strncmp(line,"Seat ",5))
            table_count++;
        }

        if (table_count == 2) {
          num_heads_up_hands++;

          if (bVerbose)
            printf("%s %d\n",filename,curr_file_num_hands);
        }
      }
    }

    fclose(fptr);

    num_hands += curr_file_num_hands;
  }

  fclose(fptr0);

  heads_up_pct = (double)num_heads_up_hands / (double)num_hands;

  if (bTerse)
    printf("%lf\n",heads_up_pct);
  else
    printf("%lf (%d %d) %s\n",heads_up_pct,num_heads_up_hands,num_hands,date_string);

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
