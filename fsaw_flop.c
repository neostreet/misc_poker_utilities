#include <stdio.h>
#include <string.h>
#include "str_misc.h"

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: fsaw_flop (-debug) (-not) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char dealt_to[] = "Dealt to ";
#define DEALT_TO_LEN (sizeof (dealt_to) - 1)
static char player_folds_str[128];
static char flop_str[] = "*** FLOP ***";
#define FLOP_STR_LEN (sizeof (flop_str) - 1)
static char summary_str[] = "*** SUMMARY ***";
#define SUMMARY_STR_LEN (sizeof (summary_str) - 1)

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  bool bDebug;
  bool bNot;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  static int dbg_line_no;
  char hole_cards[6];
  int ix;
  int player_found;
  bool bSawFlop;
  bool bFolded;
  int total_hands;
  static int dbg_hand;
  int dbg;
  int player_folds_str_len;

  if ((argc < 3) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bNot = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sprintf(player_folds_str,"%s: folds ",argv[curr_arg]);
  player_folds_str_len = strlen(player_folds_str);

  if ((fptr0 = fopen(argv[curr_arg+1],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg+1]);
    return 3;
  }

  hole_cards[5] = 0;

  total_hands = 0;

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    total_hands++;

    if (total_hands == dbg_hand)
      dbg = 1;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    line_no = 0;
    bSawFlop = false;
    bFolded = false;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      ix = 0;

      player_found = find_substring(line,&ix,argv[curr_arg],true,false);

      if (line_no == dbg_line_no)
        dbg = 1;

      if (!strncmp(line,dealt_to,DEALT_TO_LEN)) {
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
            for (p = 0; p < 5; p++)
              hole_cards[p] = line[n+p];
          }
        }
      }
      else if (!strncmp(line,flop_str,FLOP_STR_LEN)) {
        bSawFlop = true;

        if (!bNot) {
          if (!bFolded) {
            if (!bDebug)
              printf("%s\n",hole_cards);
            else
              printf("%s (%d)\n",hole_cards,total_hands);
          }
        }
      }
      else if (!strncmp(line,player_folds_str,player_folds_str_len)) {
        bFolded = true;

        if (bNot) {
          if (!bSawFlop) {
            if (!bDebug)
              printf("%s\n",hole_cards);
            else
              printf("%s (%d)\n",hole_cards,total_hands);
          }
        }
      }
      else if (!strncmp(line,summary_str,SUMMARY_STR_LEN)) {
        if (bNot && !bFolded) {
          if (!bSawFlop) {
            if (!bDebug)
              printf("%s\n",hole_cards);
            else
              printf("%s (%d)\n",hole_cards,total_hands);
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
