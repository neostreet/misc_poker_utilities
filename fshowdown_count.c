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
"usage: fshowdown_count (-verbose) (-not) (-show_board) (-show_best_hand) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char show_down[] = "*** SHOW DOWN ***";
#define SHOW_DOWN_LEN (sizeof (show_down) - 1)

static char board[] = "Board [";
#define BOARD_LEN (sizeof (board) - 1)

static char and_won[] = " and won ";
#define AND_WON_LEN (sizeof (and_won) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bVerbose;
  bool bNot;
  bool bShowBoard;
  bool bShowBestHand;
  bool bHaveShowdown;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  int file_no;
  int showdown_count;
  int ix;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bNot = false;
  bShowBoard = false;
  bShowBestHand = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else if (!strcmp(argv[curr_arg],"-show_board")) {
      bShowBoard = true;
      bVerbose = true;
    }
    else if (!strcmp(argv[curr_arg],"-show_best_hand")) {
      bShowBestHand = true;
      bVerbose = true;
    }
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
  showdown_count = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    file_no++;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    bHaveShowdown = false;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (!strncmp(line,show_down,SHOW_DOWN_LEN)) {
        bHaveShowdown = true;

        if (!bNot) {
          if (!bVerbose)
            showdown_count++;
          else if (!bShowBoard)
            printf("%s %d\n",filename,file_no);
        }

        if (!bShowBoard && !bShowBestHand)
          break;
      }
      else if (bHaveShowdown && bShowBoard && !strncmp(line,board,BOARD_LEN)) {
        if (!bNot) {
          line[21] = 0;
          printf("%s %s\n",filename,&line[BOARD_LEN]);
        }
      }
      else if (bHaveShowdown && bShowBestHand && Contains(true,line,line_len,and_won,AND_WON_LEN,&ix)) {
        if (!bNot) {
          line[ix-1] = 0;
          printf("%s %s\n",filename,&line[ix-6]);
        }
      }
    }

    fclose(fptr);

    if (bNot && !bHaveShowdown) {
      if (!bVerbose)
        showdown_count++;
      else
        printf("%s %d\n",filename,file_no);
    }
  }

  fclose(fptr0);

  if (!bVerbose)
    printf("%d\n",showdown_count);

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
