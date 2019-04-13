#include <stdio.h>
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
"usage: underwater_count (-debug) (-verbose) (-terse) (-diffval) (-reverse)\n"
"  (-only_none) (-only_all) (-only_winning) (-only_losing) (-exact_countn)\n"
"  (-le_countn) (-ge_countn) (-last_one_counts) (-get_date_from_path)\n"
"  (-avg_loss) (-consecutive) (-count_first) (-no_pct) (-skip_zero)\n"
"  (-zero_is_under) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char fmt_str1[] = "%s\n";
static char fmt_str2[] = "%lf %3d %3d %s\n";
static char fmt_str3[] = "%7.2lf %d %d %s\n";
static char fmt_str4[] = "%d %d %lf %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_date_from_path(char *path,char slash_char,int num_slashes,char **date_string_ptr);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bTerse;
  bool bDiff;
  bool bReverse;
  bool bOnlyNone;
  bool bOnlyAll;
  bool bOnlyWinning;
  bool bOnlyLosing;
  bool bExactCount;
  bool bLeCount;
  bool bGeCount;
  bool bLastOneCounts;
  bool bGetDateFromPath;
  bool bAvgLoss;
  bool bConsecutive;
  bool bCountFirst;
  bool bNoPct;
  bool bSkipZero;
  bool bZeroIsUnder;
  int exact_count;
  int le_count;
  int ge_count;
  bool bCurrentOneCounts;
  int val;
  FILE *fptr;
  int line_len;
  int line_no;
  int retval;
  char *date_string;
  int count;
  int max_consecutive_count;
  int work;
  double pct;
  double avg_loss;

  if ((argc < 2) || (argc > 22)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bTerse = false;
  bDiff = false;
  bReverse = false;
  bOnlyNone = false;
  bOnlyAll = false;
  bOnlyWinning = false;
  bOnlyLosing = false;
  bExactCount = false;
  bLeCount = false;
  bGeCount = false;
  bLastOneCounts = false;
  bGetDateFromPath = false;
  bAvgLoss = false;
  bConsecutive = false;
  bCountFirst = false;
  bNoPct = false;
  bSkipZero = false;
  bZeroIsUnder = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strncmp(argv[curr_arg],"-diff",5)) {
      bDiff = true;
      sscanf(&argv[curr_arg][5],"%d",&val);
    }
    else if (!strcmp(argv[curr_arg],"-reverse"))
      bReverse = true;
    else if (!strcmp(argv[curr_arg],"-only_none"))
      bOnlyNone = true;
    else if (!strcmp(argv[curr_arg],"-only_all"))
      bOnlyAll = true;
    else if (!strcmp(argv[curr_arg],"-only_winning"))
      bOnlyWinning = true;
    else if (!strcmp(argv[curr_arg],"-only_losing"))
      bOnlyLosing = true;
    else if (!strncmp(argv[curr_arg],"-exact_count",12)) {
      bExactCount = true;
      sscanf(&argv[curr_arg][12],"%d",&exact_count);
    }
    else if (!strncmp(argv[curr_arg],"-le_count",9)) {
      bLeCount = true;
      sscanf(&argv[curr_arg][9],"%d",&le_count);
    }
    else if (!strncmp(argv[curr_arg],"-ge_count",9)) {
      bGeCount = true;
      sscanf(&argv[curr_arg][9],"%d",&ge_count);
    }
    else if (!strcmp(argv[curr_arg],"-last_one_counts"))
      bLastOneCounts = true;
    else if (!strcmp(argv[curr_arg],"-get_date_from_path"))
      bGetDateFromPath = true;
    else if (!strcmp(argv[curr_arg],"-avg_loss"))
      bAvgLoss = true;
    else if (!strcmp(argv[curr_arg],"-consecutive"))
      bConsecutive = true;
    else if (!strcmp(argv[curr_arg],"-count_first"))
      bCountFirst = true;
    else if (!strcmp(argv[curr_arg],"-no_pct"))
      bNoPct = true;
    else if (!strcmp(argv[curr_arg],"-skip_zero"))
      bSkipZero = true;
    else if (!strcmp(argv[curr_arg],"-zero_is_under"))
      bZeroIsUnder = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bAvgLoss)
    bOnlyAll = true;

  getcwd(save_dir,_MAX_PATH);

  if (bGetDateFromPath) {
    retval = get_date_from_path(save_dir,'/',2,&date_string);

    if (retval) {
      printf("get_date_from_path() on %s failed: %d\n",save_dir,retval);
      return 3;
    }
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  if (bOnlyNone && bOnlyAll) {
    printf("can't specify both -only_none and -only_all\n");
    return 5;
  }

  if (bOnlyWinning && bOnlyLosing) {
    printf("can't specify both -only_winning and -only_losing\n");
    return 6;
  }

  if (bOnlyNone && bOnlyLosing) {
    printf("can't specify both -only_none and -only_losing\n");
    return 7;
  }

  if (bOnlyAll && bOnlyWinning) {
    printf("can't specify both -only_all and -only_winning\n");
    return 8;
  }

  if (bExactCount && bLeCount) {
    printf("can't specify both -exact_countn and -le_countn\n");
    return 9;
  }

  if (bExactCount && bGeCount) {
    printf("can't specify both -exact_countn and -ge_countn\n");
    return 10;
  }

  if (bLeCount && bGeCount && (le_count < ge_count)) {
    printf("le_count must be >= ge_count\n");
    return 11;
  }

  line_no = 0;
  count = 0;

  if (bConsecutive)
    max_consecutive_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d",&work);
    bCurrentOneCounts = false;

    if (!bReverse) {
      if ((!bZeroIsUnder && work < 0) || (bZeroIsUnder && work <= 0)) {
        if (bVerbose)
          printf("%d (%d)\n",work,line_no);

        count++;
        bCurrentOneCounts = true;
      }
      else if (bConsecutive) {
        if (count > max_consecutive_count)
          max_consecutive_count = count;

        if (count)
          count = 0;
      }
    }
    else {
      if (work > 0) {
        if (bVerbose)
          printf("%d (%d)\n",work,line_no);

        count++;
        bCurrentOneCounts = true;
      }
      else if (bConsecutive) {
        if (count > max_consecutive_count)
          max_consecutive_count = count;

        if (count)
          count = 0;
      }
    }
  }

  fclose(fptr);

  if (bConsecutive)
    count = max_consecutive_count;

  pct = (double)count / (double)line_no;

  if (!bDiff || (line_no - count == val)) {
    if (!bOnlyNone || (count == 0)) {
      if (!bOnlyAll || (count == line_no)) {
        if (!bOnlyWinning || (work > 0)) {
          if (!bOnlyLosing || (work < 0)) {
            if (!bExactCount || (count == exact_count)) {
              if (!bLeCount || (count <= le_count)) {
                if (!bGeCount || (count >= ge_count)) {
                  if (!bLastOneCounts || bCurrentOneCounts) {
                    if (!bSkipZero || count) {
                      if (bAvgLoss)
                        avg_loss = (double)work / (double)line_no;

                      if (bTerse) {
                        if (!bGetDateFromPath)
                          printf(fmt_str1,save_dir);
                        else
                          printf(fmt_str1,date_string);
                      }
                      else if (!bDebug) {
                        if (bNoPct)
                          printf("%3d %3d\n",count,line_no);
                        else
                          printf("%lf %3d %3d\n",pct,count,line_no);
                      }
                      else {
                        if (!bGetDateFromPath) {
                          if (!bAvgLoss) {
                            if (!bCountFirst)
                              printf(fmt_str2,pct,count,line_no,save_dir);
                            else
                              printf(fmt_str4,count,line_no,pct,save_dir);
                          }
                          else
                            printf(fmt_str3,avg_loss,work,line_no,save_dir);
                        }
                        else {
                          if (!bAvgLoss)
                            printf(fmt_str2,pct,count,line_no,date_string);
                          else
                            printf(fmt_str3,avg_loss,work,line_no,date_string);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
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
