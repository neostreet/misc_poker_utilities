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

static char usage[] =
"usage: fmaxdecs player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char in_chips[] = " in chips";
#define IN_CHIPS_LEN (sizeof (in_chips) - 1)
static char summary[] = "*** SUMMARY ***";
#define SUMMARY_LEN (sizeof (summary) - 1)
static char dealt_to[] = "Dealt to ";
#define DEALT_TO_LEN (sizeof (dealt_to) - 1)
static char folds[] = " folds ";
#define FOLDS_LEN (sizeof (folds) - 1)
static char bets[] = " bets ";
#define BETS_LEN (sizeof (bets) - 1)
static char calls[] = " calls ";
#define CALLS_LEN (sizeof (calls) - 1)
static char raises[] = " raises ";
#define RAISES_LEN (sizeof (raises) - 1)
static char checks[] = " checks ";
#define CHECKS_LEN (sizeof (checks) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int ix;
  int file_no;
  int dbg_file_no;
  int dbg;
  int numdecs;
  int maxdecs;
  int maxdecs_file_no;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  getcwd(save_dir,_MAX_PATH);

  player_name_ix = 1;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  file_no = 0;
  dbg_file_no = -1;
  maxdecs = 0;

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
    numdecs = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (Contains(true,
          line,line_len,
          folds,FOLDS_LEN,
          &ix)) {
          numdecs++;

          break;
        }
        else if (Contains(true,
          line,line_len,
          bets,BETS_LEN,
          &ix)) {
          numdecs++;
        }
        else if (Contains(true,
          line,line_len,
          calls,CALLS_LEN,
          &ix)) {
          numdecs++;
        }
        else if (Contains(true,
          line,line_len,
          raises,RAISES_LEN,
          &ix)) {
          numdecs++;
        }
        else if (Contains(true,
          line,line_len,
          checks,CHECKS_LEN,
          &ix)) {
          numdecs++;
        }
      }
      else {
        if (!strncmp(line,summary,SUMMARY_LEN))
          break;
      }
    }

    fclose(fptr);

    if (numdecs > maxdecs) {
      maxdecs = numdecs;
      maxdecs_file_no = file_no;
    }
  }

  fclose(fptr0);

  printf("%d %s %d\n",maxdecs,save_dir,maxdecs_file_no);

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
