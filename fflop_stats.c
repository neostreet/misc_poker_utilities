#include <stdio.h>
#include <string.h>
#include "str_misc.h"

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: fflop_stats player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char is_the_button[] = " is the button";
static char seat_str[] = "Seat ";
#define SEAT_STR_LEN (sizeof (seat_str) - 1)
static char player_folds_str[80];
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
  int big_blinds;
  int small_blinds;
  int others;
  int saw_flop;
  int saw_summary;
  int player_folded_before_flop;
  int total_hands;
  int big_blind_flops_seen;
  int small_blind_flops_seen;
  int other_flops_seen;
  int total_flops_seen;
  static int dbg_hand;
  int dbg;
  double dwork;
  int big_blind_flops_seen_pct;
  int small_blind_flops_seen_pct;
  int other_flops_seen_pct;
  int total_flops_seen_pct;
  int found;
  int pots_won_at_showdown;
  int num_showdowns;
  int pots_won_at_showdown_pct;
  int num_folded;
  int num_mucked;
  int pots_won_without_showdown;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sprintf(player_folds_str,"%s: folds ",argv[1]);

  if ((fptr0 = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  big_blinds = 0;
  small_blinds = 0;
  others = 0;

  total_hands = 0;
  big_blind_flops_seen = 0;
  small_blind_flops_seen = 0;
  other_flops_seen = 0;
  pots_won_at_showdown = 0;
  num_showdowns = 0;
  pots_won_without_showdown = 0;
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

      player_found = find_substring(line,&ix,argv[1],true,false);

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
      else if (!strcmp(line,player_folds_str)) {
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
      if (button_seat_ix == player_seat_ix) {
        small_blinds++;

        if (!player_folded_before_flop)
          small_blind_flops_seen++;
      }
      else {
        big_blinds++;

        if (!player_folded_before_flop)
          big_blind_flops_seen++;
      }

      continue;
    }

    if (button_seat_ix == player_seat_ix) {
      others++;

      if (!player_folded_before_flop)
        other_flops_seen++;

      continue;
    }

    if (player_seat_ix < button_seat_ix)
      player_seat_ix += seat_count;

    if (player_seat_ix - button_seat_ix == 2) {
      big_blinds++;

      if (!player_folded_before_flop)
        big_blind_flops_seen++;
    }
    else if (player_seat_ix - button_seat_ix == 1) {
      small_blinds++;

      if (!player_folded_before_flop)
        small_blind_flops_seen++;
    }
    else {
      others++;

      if (!player_folded_before_flop)
        other_flops_seen++;
    }
  }

  fclose(fptr0);

  total_flops_seen = big_blind_flops_seen + small_blind_flops_seen + other_flops_seen;

  dwork = (double)big_blind_flops_seen / (double)big_blinds * (double)100;
  big_blind_flops_seen_pct = (int)dwork;

  dwork = (double)small_blind_flops_seen / (double)small_blinds * (double)100;
  small_blind_flops_seen_pct = (int)dwork;

  dwork = (double)other_flops_seen / (double)others * (double)100;
  other_flops_seen_pct = (int)dwork;

  dwork = (double)total_flops_seen / (double)total_hands * (double)100;
  total_flops_seen_pct = (int)dwork;

  if (!num_showdowns)
    pots_won_at_showdown_pct = 0;
  else {
    dwork = (double)pots_won_at_showdown / (double)num_showdowns * (double)100;
    pots_won_at_showdown_pct = (int)dwork;
  }

  printf("During current Hold'em session you were dealt %d hands and saw flop:\n",
    total_hands);

  printf(" - %d out of %d times while in big blind (%d%%)\n",
    big_blind_flops_seen,big_blinds,big_blind_flops_seen_pct);
  printf(" - %d out of %d times while in small blind (%d%%)\n",
    small_blind_flops_seen,small_blinds,small_blind_flops_seen_pct);
  printf(" - %d out of %d times in other positions (%d%%)\n",
    other_flops_seen,others,other_flops_seen_pct);
  printf(" - a total of %d out of %d (%d%%)\n",
    total_flops_seen,total_hands,total_flops_seen_pct);
  printf(" Pots won at showdown - %d of %d (%d%%)\n",
    pots_won_at_showdown,num_showdowns,pots_won_at_showdown_pct);
  printf(" Pots won without showdown - %d\n",pots_won_without_showdown);

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
