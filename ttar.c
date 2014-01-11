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

static int *ints;

static char usage[] = "usage: ttar (-second_pos) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char malloc_failed[] = "malloc of %d ints failed\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
int elem_compare(const void *elem1,const void *elem2);
static int get_date_from_cwd(char *cwd,char **date_string_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bSecondPos;
  FILE *fptr;
  int retval;
  char *date_string;
  int line_len;
  int num_ints;
  int ix;
  double ttar;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bSecondPos = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-second_pos"))
      bSecondPos = true;
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

  retval = get_date_from_cwd(save_dir,&date_string);

  if (retval) {
    printf("get_date_from_cwd() failed: %d\n",retval);
    return 4;
  }

  num_ints = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    num_ints++;
  }

  fseek(fptr,0L,SEEK_SET);

  if ((ints = (int *)malloc(
    num_ints * sizeof (int))) == NULL) {
    printf(malloc_failed,num_ints);
    fclose(fptr);
    return 5;
  }

  ix = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    sscanf(line,"%d",&ints[ix++]);
  }

  fclose(fptr);

  qsort(ints,num_ints,sizeof (int),elem_compare);

  if ((num_ints >= 2) && (ints[0] > 0 ) && (!bSecondPos || (ints[1] > 0)) && (ints[1] != 0)) {
    ttar = (double)ints[0] / (double)ints[1];

    printf("%lf\t%s\n",ttar,date_string);
  }

  free(ints);

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

int elem_compare(const void *elem1,const void *elem2)
{
  int int1;
  int int2;

  int1 = *(int *)elem1;

  if (int1 < 0)
    int1 *= -1;

  int2 = *(int *)elem2;

  if (int2 < 0)
    int2 *= -1;

  return int2 - int1;
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
