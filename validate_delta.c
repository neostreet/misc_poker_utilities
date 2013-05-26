#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

static char usage[] = "usage: validate_delta (-debug) sessions_filename\n";
static char fmt_str[] = "%s\n";
static char outfile1[] = "fdeltt.out";
static char outfile2[] = "fdeltt.out.addf_int";
static char outfile3[] = "grep.out";
static char couldnt_open[] = "couldn't open %s\n";

static char save_dir[_MAX_PATH];

#define BUF_LEN 1024
static char buf[BUF_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
int get_year_month_day(char *dir,char *year_str,char *month_str,char *day_str);
int get_delta(char *line,int line_len,int *delta_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  FILE *fptr;
  int line_len;
  int delta1;
  int delta2;
  int retval;
  char year_str[5];
  char month_str[3];
  char day_str[3];

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDebug = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  sprintf(buf,"fdelta -terse neostreet hands.lst > %s",outfile1);

  if (bDebug)
    printf(fmt_str,buf);

  system(buf);

  sprintf(buf,"addf_int %s > %s",outfile1,outfile2);

  if (bDebug)
    printf(fmt_str,buf);

  system(buf);

  if ((fptr = fopen(outfile2,"r")) == NULL) {
    printf(couldnt_open,outfile2);
    return 3;
  }

  fscanf(fptr,"%d",&delta1);

  fclose(fptr);

  getcwd(save_dir,_MAX_PATH);

  retval = get_year_month_day(save_dir,year_str,month_str,day_str);

  if (retval) {
    printf("get_year_month_day() failed: %d\n",retval);
    return 4;
  }

  sprintf(buf,"grep %s-%s-%s %s > %s",
    year_str,month_str,day_str,argv[curr_arg],outfile3);

  if (bDebug)
    printf(fmt_str,buf);

  system(buf);

  if ((fptr = fopen(outfile3,"r")) == NULL) {
    printf(couldnt_open,outfile3);
    return 5;
  }

  GetLine(fptr,line,&line_len,MAX_LINE_LEN);

  fclose(fptr);

  retval = get_delta(line,line_len,&delta2);

  if (retval) {
    printf("get_delta() failed: %d\n",retval);
    return 6;
  }

  if (delta1 != delta2)
    printf("%10d %10d %s\n",delta1,delta2,save_dir);

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

int get_year_month_day(char *dir,char *year_str,char *month_str,char *day_str)
{
  int m;
  int n;
  int p;
  int len;

  len = strlen(dir);

  for (m = 0, n = len - 1; (n >= 0); n--) {
    if (dir[n] == '/') {
      m++;

      if (m == 2)
        break;
    }
  }

  if (m != 2)
    return 1;

  n++;

  for (p = 0; p < 4; p++)
    year_str[p] = dir[n+p];

  year_str[p] = 0;

  n += 5;

  for (p = 0; p < 2; p++)
    month_str[p] = dir[n+p];

  month_str[p] = 0;

  n += 2;

  for (p = 0; p < 2; p++)
    day_str[p] = dir[n+p];

  day_str[p] = 0;

  return 0;
}

int get_delta(char *line,int line_len,int *delta_ptr)
{
  int m;
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == ',')
      break;
  }

  if (n == line_len)
    return 1;

  n++;

  for (m = n; m < line_len; m++) {
    if (line[m] == ',')
      break;
  }

  if (m == line_len)
    return 2;

  line[m] = 0;

  sscanf(&line[n],"%d",delta_ptr);

  return 0;
}
