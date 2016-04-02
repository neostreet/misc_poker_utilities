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

static char usage[] =
"usage: pock_freq (-debug) (-verbose) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char avg_fmt[] = " %9.2lf %9.2lf\n";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_CARDS_IN_SUIT 13
#define NUM_SUITS 4

#define PAIR_PERIODICITY                17.0

char suit_chars[] = "cdhs";
char rank_chars[] = "23456789TJQKA";

static char bad_rank_in_line[] = "bad rank in line %d: %s\n";
static char bad_suit_in_line[] = "bad suit in line %d: %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  int m;
  int n;
  int o;
  FILE *fptr;
  int line_len;
  int rank_ix1;
  int suit_ix1;
  int rank_ix2;
  int suit_ix2;
  int pocks;
  int total_hand_count;
  double pock_freq;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  pocks = 0;
  total_hand_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    for (rank_ix1 = 0; rank_ix1 < NUM_CARDS_IN_SUIT; rank_ix1++) {
      if (line[11] == rank_chars[rank_ix1])
        break;
    }

    if (rank_ix1 == NUM_CARDS_IN_SUIT) {
      printf(bad_rank_in_line,total_hand_count+1,line);
      return 4;
    }

    for (suit_ix1 = 0; suit_ix1 < NUM_SUITS; suit_ix1++) {
      if (line[12] == suit_chars[suit_ix1])
        break;
    }

    if (suit_ix1 == NUM_SUITS) {
      printf(bad_suit_in_line,total_hand_count+1,line);
      return 5;
    }

    for (rank_ix2 = 0; rank_ix2 < NUM_CARDS_IN_SUIT; rank_ix2++) {
      if (line[14] == rank_chars[rank_ix2])
        break;
    }

    if (rank_ix2 == NUM_CARDS_IN_SUIT) {
      printf(bad_rank_in_line,total_hand_count+1,line);
      return 6;
    }

    for (suit_ix2 = 0; suit_ix2 < NUM_SUITS; suit_ix2++) {
      if (line[15] == suit_chars[suit_ix2])
        break;
    }

    if (suit_ix2 == NUM_SUITS) {
      printf(bad_suit_in_line,total_hand_count+1,line);
      return 7;
    }

    if (rank_ix1 == rank_ix2)
      pocks++;

    total_hand_count++;
  }

  fclose(fptr);

  pock_freq = (double)pocks / (double)total_hand_count * PAIR_PERIODICITY;

  if (!bVerbose)
    printf("%lf\n",pock_freq);
  else
    printf("%lf (%d %d)%s\n",pock_freq,pocks,total_hand_count,save_dir);

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
