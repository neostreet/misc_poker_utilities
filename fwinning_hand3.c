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
static char max_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fwinning_hand3 (-verbose) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char pokerstars_hand[] = "PokerStars Hand";
#define POKERSTARS_HAND_LEN (sizeof (pokerstars_hand) - 1)
static char dealt_to[] = "Dealt to ";
#define DEALT_TO_LEN (sizeof (dealt_to) - 1)
static char and_won[] = " and won ";
#define AND_WON_LEN (sizeof (and_won) - 1)

static char fmt_str[] = "%10d %s %d\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);

int main(int argc,char **argv)
{
  int m;
  int n;
  int curr_arg;
  int player_name_ix;
  int player_name_len;
  char hole_cards[6];
  bool bVerbose;
  FILE *fptr0;
  int filenamelen;
  FILE *fptr;
  int line_len;
  int line_no;
  int hand;
  int ix;

  if ((argc < 3) && (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
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

  hole_cards[5] = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filenamelen,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    hand = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      if (Contains(true,
        line,line_len,
        pokerstars_hand,POKERSTARS_HAND_LEN,
        &ix)) {
        hand++;
      }
      else if (Contains(true,
        line,line_len,
        argv[player_name_ix],player_name_len,
        &ix) && !strncmp(line,dealt_to,DEALT_TO_LEN)) {
          for (n = 0; n < line_len; n++) {
            if (line[n] == '[')
              break;
          }

          if (n < line_len) {
            n++;

            for (m = n; m < line_len; m++) {
              if (line[m] == ']')
                break;
            }

            if (m < line_len) {
              for (m = 0; m < 5; m++)
                hole_cards[m] = line[n+m];
          }
        }
      }
      else if (Contains(true,
        line,line_len,
        and_won,AND_WON_LEN,
        &ix)) {

        for (n = 0; n < line_len; n++) {
          if (line[n] == '[')
            break;
        }

        if (n < line_len) {
          n++;

          for (m = n; m < line_len; m++) {
            if (line[m] == ']')
              break;
          }

          if (m < line_len) {
            line[m] = 0;
            printf("%s %s",&line[n],hole_cards);

            if (bVerbose)
              printf(" %s %3d\n",filename,hand);
            else
              putchar(0x0a);
          }
        }
      }
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
