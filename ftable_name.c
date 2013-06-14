#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include "str_list.h"

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define MAX_TABLE_NAME_LEN 64
static char table_name[MAX_TABLE_NAME_LEN+1];

static char usage[] =
"usage: ftable_name (-debug) (-verbose) (-per_file) filename\n";
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
  bool bPerFile;
  FILE *fptr0;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_len;
  struct info_list tables;
  struct info_list_elem *work_elem;
  int ix;
  int retval;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bPerFile = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-per_file"))
      bPerFile = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  tables.num_elems = 0;

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

    if (bPerFile)
      free_info_list(&tables);

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      if (!strncmp(line,"Table '",7)) {
        retval = get_table_name(line,line_len,table_name,MAX_TABLE_NAME_LEN);

        if (retval) {
          printf("get_table_name() failed on line %d: %d\n",line_len,retval);
          return 4;
        }

        if (bDebug)
          printf("%s\n",table_name);

        if (member_of_info_list(&tables,table_name,&ix)) {
          if (get_info_list_elem(&tables,ix,&work_elem))
            work_elem->int1++;
        }
        else {
          add_info_list_elem(&tables,table_name,1,0,0,true);

          if (!bDebug) {
            if (!bVerbose)
              printf("%s\n",table_name);
            else
              printf("%s %s %s\n",filename,table_name,line);
          }
        }
      }
    }

    fclose(fptr);
  }

  free_info_list(&tables);

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
