#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_TABLE_NAME_LEN 64
static char table_name[MAX_TABLE_NAME_LEN+1];
static char prev_table_name[MAX_TABLE_NAME_LEN+1];

static char usage[] =
"usage: ftable_name (-debug) (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_table_name(
  char *line,
  int line_len,
  char *table_name,
  int max_table_name_len
);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  FILE *fptr0;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_no;
  bool bHavePrevTableName;
  int line_len;
  int ix;
  int retval;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  num_files = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    num_files++;
    line_no = 0;
    bHavePrevTableName = false;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (!strncmp(line,"Table '",7)) {
        retval = get_table_name(line,line_len,table_name,MAX_TABLE_NAME_LEN);

        if (retval) {
          printf("get_table_name() failed on line %d: %d\n",line_len,retval);
          return 4;
        }

        if (!bHavePrevTableName || strcmp(prev_table_name,table_name)) {
          if (!bVerbose)
            printf("%s\n",table_name);
          else
            printf("%s %s %s\n",filename,table_name,line);

          strcpy(prev_table_name,table_name);
          bHavePrevTableName = true;
        }
      }
    }

    fclose(fptr);
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

static int get_table_name(
  char *line,
  int line_len,
  char *table_name,
  int max_table_name_len
)
{
  int m;
  int n;

  for (m = 0, n = 7; n < line_len; n++) {
    if (line[n] == '\'')
      break;

    table_name[m++] = line[n];
  }

  if (n == line_len)
    return 1;

  table_name[m] = 0;

  return 0;
}
