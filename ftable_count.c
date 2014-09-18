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

#define MAX_PLAYERS 6

static char usage[] =
"usage: ftable_count (-genum) (-not) (-terse) (-sum) (-early_exit)\n"
"  (-exact_countn) (-agg_by_count) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  int ge_num;
  bool bNot;
  bool bTerse;
  bool bSum;
  bool bEarlyExit;
  bool bExactCount;
  int exact_count;
  bool bAggByCount;
  char counts[MAX_PLAYERS];
  FILE *fptr0;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_len;
  int table_count;
  int sum_table_count;
  int hand_count;
  bool bSkip;

  if ((argc < 2) || (argc > 9)) {
    printf(usage);
    return 1;
  }

  ge_num = -1;
  bNot = false;
  bTerse = false;
  bSum = false;
  bEarlyExit = false;
  bExactCount = false;
  bAggByCount = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strncmp(argv[curr_arg],"-ge",3))
      sscanf(&argv[curr_arg][3],"%d",&ge_num);
    else if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-sum"))
      bSum = true;
    else if (!strcmp(argv[curr_arg],"-early_exit"))
      bEarlyExit = true;
    else if (!strncmp(argv[curr_arg],"-exact_count",12)) {
      bExactCount = true;
      sscanf(&argv[curr_arg][12],"%d",&exact_count);
    }
    else if (!strcmp(argv[curr_arg],"-agg_by_count"))
      bAggByCount = true;
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

  getcwd(save_dir,_MAX_PATH);

  if (bSum)
    sum_table_count = 0;

  if (bAggByCount) {
    for (n = 0; n < MAX_PLAYERS; n++)
      counts[n] = 0;
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
    hand_count = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      if (!strncmp(line,"Table '",7)) {
        table_count = 0;
        hand_count++;

        for ( ; ; ) {
          GetLine(fptr,line,&line_len,MAX_LINE_LEN);

          if (feof(fptr))
            break;

          if (!strncmp(line,"*** ",4))
            break;

          if (!strncmp(line,"Seat ",5))
            table_count++;
        }

        if (bSum)
          sum_table_count += table_count;
        else {
          bSkip = false;

          if (bAggByCount) {
            bSkip = true;

            if (table_count > MAX_PLAYERS) {
              printf("%s: table count is too high\n",filename);
              return 4;
            }

            table_count--;
            counts[table_count]++;
          }
          else if (ge_num != -1) {
            if (!bNot) {
              if (table_count < ge_num)
                bSkip = true;
            }
            else {
              if (table_count >= ge_num)
                bSkip = true;
            }
          }
          else if (bExactCount && (table_count != exact_count))
            bSkip = true;

          if (!bSkip) {
            if (bTerse)
              printf("%d\n",table_count);
            else
              printf("%d %s %d\n",table_count,filename,hand_count);
          }
        }

        if (bEarlyExit)
          break;
      }
    }

    fclose(fptr);
  }

  fclose(fptr0);

  if (bSum)
    printf("%d\n",sum_table_count);
  else if (bAggByCount) {
    if (bExactCount)
      printf("%d %s\n",counts[exact_count - 1],save_dir);
    else {
      for (n = MAX_PLAYERS - 1; (n >= 0); n--) {
        if (counts[n])
          printf("%d %d %s\n",n+1,counts[n],save_dir);
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
