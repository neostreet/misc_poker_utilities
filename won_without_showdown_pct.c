#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#define _MAX_PATH 4096
#include <unistd.h>
#endif
#include "str_misc.h"

static char save_dir[_MAX_PATH];

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: won_without_showdown_pct (-verbose) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char is_the_button[] = " is the button";
static char seat_str[] = "Seat ";
#define SEAT_STR_LEN (sizeof (seat_str) - 1)
static char player_folds_str[128];
static char flop_str[] = "*** FLOP ***";
#define FLOP_STR_LEN (sizeof (flop_str) - 1)
static char summary_str[] = "*** SUMMARY ***";
#define SUMMARY_STR_LEN (sizeof (summary_str) - 1)
static char showed_str[] = " showed ";
static char won_str[] = " won ";
static char folded_str[] = " folded ";
static char collected_str[] = " collected ";
static char mucked_str[] = " mucked ";

#define MAX_SEATS 9
static int seats[MAX_SEATS];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bVerbose;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  static int dbg_line_no;
  int ix;
  int button_seat;
  int button_seat_ix;
  int player_found;
  int player_seat_ix;
  int seat_count;
  int saw_flop;
  int saw_summary;
  int player_folded_before_flop;
  int total_hands;
  static int dbg_hand;
  int dbg;
  int found;
  int pots_won_at_showdown;
  int num_showdowns;
  double won_without_showdown_pct;
  int num_folded;
  int num_mucked;
  int pots_won_without_showdown;
  int player_folds_str_len;
  int total_wins;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
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

  total_hands = 0;
  pots_won_at_showdown = 0;
  num_showdowns = 0;
  num_folded = 0;
  pots_won_without_showdown = 0;
  num_mucked = 0;

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
    seat_count = 0;
    player_seat_ix = -1;
    saw_flop = 0;
    saw_summary = 0;
    player_folded_before_flop = 0;

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      line_no++;

      ix = 0;

      player_found = find_substring(line,&ix,argv[curr_arg],true,false);

      if ((player_found) && (saw_summary)) {
        ix = 0;

        found = find_substring(line,&ix,showed_str,true,false);

        if (found) {
          num_showdowns++;
          ix = 0;

          found = find_substring(line,&ix,won_str,true,false);

          if (found)
            pots_won_at_showdown++;
        }
        else {
          ix = 0;

          found = find_substring(line,&ix,folded_str,true,false);

          if (found)
            num_folded++;
          else {
            ix = 0;

            found = find_substring(line,&ix,collected_str,true,false);

            if (found)
              pots_won_without_showdown++;
            else {
              ix = 0;

              found = find_substring(line,&ix,mucked_str,true,false);

              if (found) {
                num_mucked++;
                num_showdowns++;
              }
            }
          }
        }
      }

      if (line_no == dbg_line_no)
        dbg = 1;

      if (line_no == 2) {
        ix = 0;

        if (find_substring(line,&ix,is_the_button,true,false)) {
          line[ix] = 0;
          sscanf(&line[ix-1],"%d",&button_seat);
        }
        else {
          printf("  %s: couldn't find the button seat\n",filename);
          continue;
        }
      }
      else if ((line_no >= 3) && (line_no <= 11) && !strncmp(line,seat_str,SEAT_STR_LEN)) {
        line[6] = 0;
        sscanf(&line[5],"%d",&seats[seat_count]);

        if (player_found)
          player_seat_ix = seat_count;

        if (seats[seat_count] == button_seat)
          button_seat_ix = seat_count;

        seat_count++;
      }
      else if (!strncmp(line,player_folds_str,player_folds_str_len)) {
        if (!saw_flop)
          player_folded_before_flop = 1;
      }
      else if (!strncmp(line,flop_str,FLOP_STR_LEN))
        saw_flop = 1;
      else if (!strncmp(line,summary_str,SUMMARY_STR_LEN))
        saw_summary = 1;
    }

    fclose(fptr);

    if (player_seat_ix == -1) {
      printf("  %s: couldn't find the player\n",filename);
      continue;
    }

    if (seat_count == 2) {
      continue;
    }

    if (button_seat_ix == player_seat_ix)
      continue;

    if (player_seat_ix < button_seat_ix)
      player_seat_ix += seat_count;
  }

  fclose(fptr0);

  total_wins = pots_won_at_showdown + pots_won_without_showdown;

  if (!total_wins)
    won_without_showdown_pct = (double)0;
  else
    won_without_showdown_pct = (double)pots_won_without_showdown / (double)total_wins;

  printf("%8.6lf ",won_without_showdown_pct);
  printf("(%d %d)",pots_won_without_showdown,total_wins);

  if (!bVerbose)
    putchar(0x0a);
  else
    printf(" %s\n",save_dir);

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
