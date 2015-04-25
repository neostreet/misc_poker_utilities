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

static char usage[] =
"usage: fflop_stats_by_table_count (-verbose) player_name filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char seat_str[] = "Seat ";
#define SEAT_STR_LEN (sizeof (seat_str) - 1)
static char player_folds_str[128];
static char flop_str[] = "*** FLOP ***";
#define FLOP_STR_LEN (sizeof (flop_str) - 1)
static char summary_str[] = "*** SUMMARY ***";
#define SUMMARY_STR_LEN (sizeof (summary_str) - 1)

static char fmt_str[] = "%s (%d)\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

struct flop_info {
  int total_hands;
  int total_flops_seen;
};

#define NUM_TABLE_COUNTS 8

static struct flop_info fflop_stats_by_table_count[NUM_TABLE_COUNTS];

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int line_no;
  static int dbg_line_no;
  int ix;
  int player_found;
  int player_seat_ix;
  int seat_count;
  int saw_flop;
  int saw_summary;
  int player_folded_before_flop;
  int total_hands;
  static int dbg_hand;
  int dbg;
  double dwork;
  double total_flops_seen_pct;
  int total_flops_seen_pct_int;
  int player_folds_str_len;

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

  for (n = 0; n < NUM_TABLE_COUNTS; n++) {
    fflop_stats_by_table_count[n].total_hands = 0;
    fflop_stats_by_table_count[n].total_flops_seen = 0;
  }

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

      if (line_no == dbg_line_no)
        dbg = 1;

      if ((line_no >= 3) && (line_no <= 11) && !strncmp(line,seat_str,SEAT_STR_LEN)) {
        if (player_found)
          player_seat_ix = 0;

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

    ix = seat_count - 2;

    fflop_stats_by_table_count[ix].total_hands++;

    if (saw_flop && !player_folded_before_flop)
      fflop_stats_by_table_count[ix].total_flops_seen++;
  }

  fclose(fptr0);

  for (n = 0; n < NUM_TABLE_COUNTS; n++) {
    if (!fflop_stats_by_table_count[n].total_hands)
      continue;

    total_flops_seen_pct = (double)fflop_stats_by_table_count[n].total_flops_seen /
      (double)fflop_stats_by_table_count[n].total_hands;
    dwork = (double)fflop_stats_by_table_count[n].total_flops_seen /
      (double)fflop_stats_by_table_count[n].total_hands * (double)100;
    total_flops_seen_pct_int = (int)dwork;

    printf("%d: %8.6lf ",n + 2,total_flops_seen_pct);
    printf("(%d %d)",
      fflop_stats_by_table_count[n].total_flops_seen,
      fflop_stats_by_table_count[n].total_hands);

    if (!bVerbose)
      putchar(0x0a);
    else
      printf(" %s\n",save_dir);
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
