#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fwinning_hands3 (-verbose) (-debug) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char summary[] = "*** SUMMARY ***";
#define SUMMARY_LEN (sizeof (summary) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bVerbose;
  bool bDebug;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int dbg_line_no;
  int ix;
  int ix2;
  int file_no;
  int dbg_file_no;
  int num_hands;
  int dbg;
  bool bInSummary;
  int num_winning_rockets;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bDebug = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
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

  file_no = 0;
  dbg_file_no = -1;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    file_no++;

    if (dbg_file_no == file_no)
      dbg = 1;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    num_hands = 0;
    bInSummary = false;
    num_winning_rockets = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (line_no == dbg_line_no)
        dbg = 1;

      if (bDebug)
        printf("line %d %s\n",line_no,line);

      if (!strncmp(line,"Table '",7)) {
        bInSummary = false;
        num_hands++;
      }
      else if (!strncmp(line,summary,SUMMARY_LEN)) {
        if (bDebug)
          printf("line %d SUMMARY line detected; skipping\n",line_no);

        bInSummary = true;
      }
      else if (!strncmp(line,"Seat ",5)) {
        if (bInSummary) {
          if (Contains(true,
            line,line_len,
            (char *)"[",1,
            &ix)) {

            if (Contains(true,
              line,line_len,
              (char *)"won",3,
              &ix2)) {

              if ((line[ix+1] == 'A') && (line[ix+4] == 'A'))
                num_winning_rockets++;
            }
          }
        }
      }
    }

    fclose(fptr);

    printf("%3d (%3d) %s\n",num_winning_rockets,num_hands,filename);
  }

  fclose(fptr0);

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

static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index)
{
  int m;
  int n;
  int tries;
  char chara;

  tries = line_len - string_len + 1;

  if (tries <= 0)
    return false;

  for (m = 0; m < tries; m++) {
    for (n = 0; n < string_len; n++) {
      chara = line[m + n];

      if (!bCaseSens) {
        if ((chara >= 'A') && (chara <= 'Z'))
          chara += 'a' - 'A';
      }

      if (chara != string[n])
        break;
    }

    if (n == string_len) {
      *index = m;
      return true;
    }
  }

  return false;
}
