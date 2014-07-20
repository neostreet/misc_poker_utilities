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
static char dealt_to_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];
static char dealt_to_line[MAX_LINE_LEN];

static char usage[] =
"usage: fdealt_to3 (-debug) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char dealt_to[] = "Dealt to ";
#define DEALT_TO_LEN (sizeof (dealt_to) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
void do_print_dealt_to(
  char *dealt_to_line,
  int dealt_to_line_line_len,
  bool bDebug,
  int dealt_to_line_hand_number
);
void print_dealt_to(char *str,int len);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  bool bDebug;
  bool bHaveDealtToLine;
  int player_name_ix;
  int player_name_len;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int dealt_to_line_line_len;
  int dealt_to_line_hand_number;
  int line_no;
  int dbg_line_no;
  int dbg;
  int ix;
  int file_no;
  int num_hands;
  int hand_number;

  if ((argc < 3) || (argc > 4)) {
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

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  player_name_ix = curr_arg++;
  player_name_len = strlen(argv[player_name_ix]);

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  file_no = 0;
  num_hands = 0;
  dbg_line_no = -1;

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
    bHaveDealtToLine = false;
    hand_number = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (line_no == dbg_line_no)
        dbg = 1;

      if (!strncmp(line,"PokerStars ",11)) {
        num_hands++;
        hand_number++;

        if (bHaveDealtToLine) {
          do_print_dealt_to(dealt_to_line,dealt_to_line_line_len,bDebug,
            dealt_to_line_hand_number);
          bHaveDealtToLine = false;
        }
      }
      else if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix)) {

        if (!strncmp(line,dealt_to,DEALT_TO_LEN)) {
          strcpy(dealt_to_line,line);

          if (!bHaveDealtToLine) {
            strcpy(dealt_to_filename,filename);
            bHaveDealtToLine = true;
          }

          dealt_to_line_line_len = line_len;
          dealt_to_line_hand_number = hand_number;
        }
      }
    }

    if (bHaveDealtToLine) {
      do_print_dealt_to(dealt_to_line,dealt_to_line_line_len,bDebug,
        dealt_to_line_hand_number);
    }

    fclose(fptr);
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

    if ((chara == 0xef) || (chara == 0xbb) || (chara == 0xbf))
      continue;

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

void do_print_dealt_to(
  char *dealt_to_line,
  int dealt_to_line_line_len,
  bool bDebug,
  int dealt_to_line_hand_number
)
{
  int m;
  int n;

  for (n = 0; n < dealt_to_line_line_len; n++) {
    if (dealt_to_line[n] == '[')
      break;
  }

  if (n < dealt_to_line_line_len) {
    n++;

    for (m = dealt_to_line_line_len - 1; m >= 0; m--) {
      if (dealt_to_line[m] == ']')
        break;
    }

    if (m < dealt_to_line_line_len) {
      print_dealt_to(&dealt_to_line[n],m-n);

      if (!bDebug)
        putchar(0x0a);
      else
        printf(" %s %d\n",dealt_to_filename,dealt_to_line_hand_number);
    }
  }
}

void print_dealt_to(char *str,int len)
{
  int n;
  int chara;

  for (n = 0; n < len; n++) {
    chara = str[n];

    if ((chara != '[') && (chara != ']'))
      putchar(chara);
  }
}
