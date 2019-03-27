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
"usage: zero_crossings (-verbose) (-debug) (-date_string) (-pct)\n"
"  (-exact_countn) (-is_exact_countn) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);
static bool zero_crossing(bool *bBelow,int val,bool bDebug);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  bool bDebug;
  bool bDateString;
  bool bPct;
  bool bExactCount;
  int exact_count;
  bool bIsExactCount;
  FILE *fptr;
  int line_len;
  int retval;
  int val;
  bool bBelow;
  char *date_string;
  int zero_crossings;
  int nobs;
  double dwork;
  int curr_min;
  int curr_max;
  int abs_traveled;

  if ((argc < 2) || (argc > 8)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bDebug = false;
  bDateString = false;
  bPct = false;
  bExactCount = false;
  bIsExactCount = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-date_string"))
      bDateString = true;
    else if (!strcmp(argv[curr_arg],"-pct"))
      bPct = true;
    else if (!strncmp(argv[curr_arg],"-exact_count",12)) {
      bExactCount = true;
      sscanf(&argv[curr_arg][12],"%d",&exact_count);
    }
    else if (!strncmp(argv[curr_arg],"-is_exact_count",15)) {
      bIsExactCount = true;
      sscanf(&argv[curr_arg][15],"%d",&exact_count);
    }
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

  if (bDateString) {
    retval = get_date_from_path(save_dir,'/',2,&date_string);

    if (retval) {
      printf("get_date_from_path() failed: %d\n",retval);
      return 4;
    }

    bVerbose = true;
  }

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr)) {
      printf("couldn't set initial value of bBelow\n");
      return 5;
    }

    sscanf(line,"%d",&val);

    if (val > 0) {
      bBelow = false;
      break;
    }
    else if (val < 0) {
      bBelow = true;
      break;
    }
  }

  fseek(fptr,0L,SEEK_SET);

  zero_crossings = 0;
  nobs = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    nobs++;

    sscanf(line,"%d",&val);

    if (zero_crossing(&bBelow,val,bDebug))
      zero_crossings++;
  }

  fclose(fptr);

  if (bExactCount) {
    if (zero_crossings != exact_count) {
      return 0;
    }
  }

  if (bPct) {
    dwork = (double)zero_crossings / (double)nobs;
  }

  if (!bVerbose) {
    if (bIsExactCount) {
      printf("%d\n",(zero_crossings == exact_count));
    }
    else if (!bPct)
      printf("%d\n",zero_crossings);
    else
      printf("%lf\n",dwork);
  }
  else {
    if (!bDateString) {
      if (bIsExactCount) {
        printf("%3d (%3d) %s/%s\n",(zero_crossings == exact_count),nobs,save_dir,argv[curr_arg]);
      }
      else if (!bPct) {
        printf("%3d (%3d) %s/%s\n",zero_crossings,nobs,save_dir,argv[curr_arg]);
      }
      else {
        printf("%lf (%d %d) %s/%s\n",dwork,zero_crossings,nobs,
          save_dir,argv[curr_arg]);
      }
    }
    else {
      if (bIsExactCount)
        printf("%3d %s\n",(zero_crossings == exact_count),date_string);
      else if (!bPct)
        printf("%3d %s\n",zero_crossings,date_string);
      else
        printf("%lf (%d %d) %s\n",dwork,zero_crossings,nobs,date_string);
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

static bool zero_crossing(bool *bBelow,int val,bool bDebug)
{
  bool retval;

  if (!(*bBelow) && (val < 0)) {
    retval = true;
    *bBelow = true;
  }
  else if ((*bBelow) && (val > 0)) {
    retval = true;
    *bBelow = false;
  }
  else
    retval = false;

  if (bDebug)
    printf("val = %d, retval = %d\n",val,retval);

  return retval;
}
