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
"usage: pock_freq (-debug) (-verbose) (-detail) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char avg_fmt[] = " %9.2lf %9.2lf\n";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_CARDS_IN_SUIT 13
#define NUM_SUITS 4

#define PAIR_PERIODICITY               221.0
#define ANY_PAIR_PERIODICITY            17.0

char rank_chars[] = "23456789TJQKA";

static char bad_rank_in_line[] = "bad rank in line %d: %s\n";

static char pock_count[NUM_CARDS_IN_SUIT];

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bDetail;
  FILE *fptr;
  int line_len;
  int rank_ix1;
  int rank_ix2;
  int any_pock_count;
  int total_hand_count;
  double pock_freq;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bDetail = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose")) {
      bVerbose = true;
      getcwd(save_dir,_MAX_PATH);
    }
    else if (!strcmp(argv[curr_arg],"-detail"))
      bDetail = true;
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

  if (!bDetail)
    any_pock_count = 0;
  else {
    for (n = 0; n < NUM_CARDS_IN_SUIT; n++)
      pock_count[n] = 0;
  }

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

    for (rank_ix2 = 0; rank_ix2 < NUM_CARDS_IN_SUIT; rank_ix2++) {
      if (line[14] == rank_chars[rank_ix2])
        break;
    }

    if (rank_ix2 == NUM_CARDS_IN_SUIT) {
      printf(bad_rank_in_line,total_hand_count+1,line);
      return 5;
    }

    if (rank_ix1 == rank_ix2) {
      if (!bDetail)
        any_pock_count++;
      else
        pock_count[rank_ix1]++;
    }

    total_hand_count++;
  }

  fclose(fptr);

  if (!bDetail) {
    pock_freq = (double)any_pock_count / (double)total_hand_count * ANY_PAIR_PERIODICITY;

    if (!bVerbose)
      printf("%lf\n",pock_freq);
    else
      printf("%lf (%d %d) %s\n",pock_freq,any_pock_count,total_hand_count,save_dir);
  }
  else {
    for (n = 0; n < NUM_CARDS_IN_SUIT; n++) {
      if (pock_count[n]) {
        pock_freq = (double)pock_count[n] / (double)total_hand_count * PAIR_PERIODICITY;

        if (!bVerbose)
          printf("%c%c %lf\n",rank_chars[n],rank_chars[n],pock_freq);
        else {
          printf("%c%c %lf (%d %d) %s\n",rank_chars[n],rank_chars[n],pock_freq,
            pock_count[n],total_hand_count,save_dir);
        }
      }
    }
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
