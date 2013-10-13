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

#define MAX_BIG_BLIND_AMOUNTS 10
static int big_blind_amount[MAX_BIG_BLIND_AMOUNTS];

static char usage[] =
"usage: fbig_blind (-debug) (-verbose) (-per_file) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int get_big_blind_amount(
  char *line,
  int line_len,
  int *big_blind_amount
);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bPerFile;
  FILE *fptr0;
  int filename_len;
  int num_files;
  FILE *fptr;
  int line_len;
  int ix;
  int retval;
  int num_big_blind_amounts;
  int curr_big_blind_amount;

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

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  num_files = 0;
  num_big_blind_amounts = 0;

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
      num_big_blind_amounts = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      if (!strncmp(line,"PokerStars ",11)) {
        retval = get_big_blind_amount(line,line_len,&curr_big_blind_amount);

        if (retval) {
          printf("get_big_blind_amount() failed on line %d: %d\n",line_len,retval);
          return 4;
        }

        if (bDebug)
          printf("%s\n",big_blind_amount[num_big_blind_amounts - 1]);

        for (n = 0; n < num_big_blind_amounts; n++) {
          if (big_blind_amount[n] == curr_big_blind_amount)
            break;
        }

        if (n == num_big_blind_amounts) {
          if (num_big_blind_amounts == MAX_BIG_BLIND_AMOUNTS) {
            printf("MAX_BIG_BLIND_AMOUNTS value of %d exceeded\n",
              MAX_BIG_BLIND_AMOUNTS);
            return 5;
          }

          big_blind_amount[num_big_blind_amounts++] = curr_big_blind_amount;

          if (!bDebug) {
            if (!bVerbose)
              printf("%d\n",curr_big_blind_amount);
            else
              printf("%d %s %s\n",curr_big_blind_amount,filename,line);
          }
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

static int get_big_blind_amount(
  char *line,
  int line_len,
  int *big_blind_amount
)
{
  int n;

  for (n = 0; n < line_len; n++) {
    if (line[n] == ')')
      break;
  }

  if (n == line_len)
    return 1;

  for (n--; (n >= 0); n--) {
    if (line[n] == '/')
      break;
  }

  if (n < 0)
    return 2;

  n++;

  sscanf(&line[n],"%d",big_blind_amount);

  return 0;
}
