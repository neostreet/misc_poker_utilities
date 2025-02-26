#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

#define MAIN_MODULE
#include "poker_hand.h"

static char usage[] =
"usage: aggreg_hands4 (-debug) (-verbose) (-terse) (-sort_by_freq) (-sort_by_count) (-only_missing) (-not)\n"
"  (-only_premium) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

static char fbf_str[] = "fbf";
#define FBF_STR_LEN (sizeof (fbf_str) - 1)

static char sf_str[] = "sf";
#define SF_STR_LEN (sizeof (sf_str) - 1)

static char ws_str[] = "ws";
#define WS_STR_LEN (sizeof (ws_str) - 1)

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define PAIR_PERIODICITY               221.0
#define SUITED_NONPAIR_PERIODICITY     331.5
#define NONSUITED_NONPAIR_PERIODICITY  110.5

static double periodicities[] = {
  (double)PAIR_PERIODICITY,
  (double)SUITED_NONPAIR_PERIODICITY,
  (double)NONSUITED_NONPAIR_PERIODICITY
};

enum hand_type {
  HAND_TYPE_PAIR,
  HAND_TYPE_SUITED_NONPAIR,
  HAND_TYPE_NONSUITED_NONPAIR
};

struct aggreg_info {
  hand_type handtype;
  char card_string[4];
  int hand_count;
  int num_fbfs;
  int num_flops_seen;
  int num_wins;
  int num_losses;
  double freq_factor;
  double fbf_pct;
  double flops_seen_pct;
  double win_pct;
};

#define NUM_COLLAPSED_HANDS 169
static struct aggreg_info aggreg[NUM_COLLAPSED_HANDS];

static char bad_rank_in_line[] = "bad rank in line %d: %s\n";

static char bad_suit_in_line[] = "bad suit in line %d: %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static bool Contains(bool bCaseSens,char *line,int line_len,
  char *string,int string_len,int *index);
static int index_of_hand(int rank_ix1,int suit_ix1,int rank_ix2,int suit_ix2,hand_type *handtype_pt);
static void get_permutation_instance(
  int set_size,
  int *m,int *n,
  int instance_ix
);
int compare(const void *elem1,const void *elem2);
int compare2(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bTerse;
  bool bSortByFreq;
  bool bSortByCount;
  bool bOnlyMissing;
  bool bPrint;
  bool bNot;
  bool bOnlyPremium;
  int premium_ix;
  int dbg_ix;
  int dbg;
  int m;
  int n;
  int o;
  FILE *fptr;
  int line_len;
  int rank_ix1;
  int suit_ix1;
  int rank_ix2;
  int suit_ix2;
  hand_type handtype;
  int ix;
  int ix2;
  int total_hand_count;
  int num_collapsed_hands;
  int ixs[NUM_COLLAPSED_HANDS];

  if ((argc < 2) || (argc > 10)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bTerse = false;
  bSortByFreq = false;
  bSortByCount = false;
  bOnlyMissing = false;
  bNot = false;
  bOnlyPremium = false;

  dbg_ix = -1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-sort_by_freq"))
      bSortByFreq = true;
    else if (!strcmp(argv[curr_arg],"-sort_by_count"))
      bSortByCount = true;
    else if (!strcmp(argv[curr_arg],"-only_missing"))
      bOnlyMissing = true;
    else if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else if (!strcmp(argv[curr_arg],"-only_premium"))
      bOnlyPremium = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bVerbose && bTerse) {
    printf("can't specify both -verbose and -terse\n");
    return 3;
  }

  if (bSortByFreq && bSortByCount) {
    printf("can't specify both -sort_by_freq and -sort_by_count\n");
    return 4;
  }

  num_collapsed_hands = NUM_COLLAPSED_HANDS;

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 5;
  }

  for (n = 0; n < num_collapsed_hands; n++) {
    if (n < NUM_CARDS_IN_SUIT)
      aggreg[n].handtype = HAND_TYPE_PAIR;
    else if (n < NUM_CARDS_IN_SUIT + NUM_SUITED_NONPAIRS)
      aggreg[n].handtype = HAND_TYPE_SUITED_NONPAIR;
    else
      aggreg[n].handtype = HAND_TYPE_NONSUITED_NONPAIR;

    aggreg[n].hand_count = 0;
    aggreg[n].num_fbfs = 0;
    aggreg[n].num_flops_seen = 0;
    aggreg[n].num_wins = 0;
    aggreg[n].num_losses = 0;
  }

  total_hand_count = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if ((line[0] >= 'a') && (line[0] <= 'z'))
      line[0] -= 'a' - 'A';

    for (rank_ix1 = 0; rank_ix1 < NUM_CARDS_IN_SUIT; rank_ix1++) {
      if (line[0] == rank_chars[rank_ix1])
        break;
    }

    if (rank_ix1 == NUM_CARDS_IN_SUIT) {
      printf(bad_rank_in_line,total_hand_count+1,line);
      return 6;
    }

    for (suit_ix1 = 0; suit_ix1 < NUM_SUITS; suit_ix1++) {
      if (line[1] == suit_chars[suit_ix1])
        break;
    }

    if (suit_ix1 == NUM_SUITS) {
      printf(bad_suit_in_line,total_hand_count+1,line);
      return 7;
    }

    if ((line[3] >= 'a') && (line[3] <= 'z'))
      line[3] -= 'a' - 'A';

    for (rank_ix2 = 0; rank_ix2 < NUM_CARDS_IN_SUIT; rank_ix2++) {
      if (line[3] == rank_chars[rank_ix2])
        break;
    }

    if (rank_ix2 == NUM_CARDS_IN_SUIT) {
      printf(bad_rank_in_line,total_hand_count+1,line);
      return 8;
    }

    for (suit_ix2 = 0; suit_ix2 < NUM_SUITS; suit_ix2++) {
      if (line[4] == suit_chars[suit_ix2])
        break;
    }

    if (suit_ix2 == NUM_SUITS) {
      printf(bad_suit_in_line,total_hand_count+1,line);
      return 9;
    }

    total_hand_count++;

    ix = index_of_hand(rank_ix1,suit_ix1,rank_ix2,suit_ix2,&handtype);

    if (bDebug) {
      printf("index_of_hand(): %d %d %d %d: ix = %d, handtype = %d\n",rank_ix1,suit_ix1,rank_ix2,suit_ix2,ix,handtype);
    }

    if (ix == dbg_ix)
      dbg = 1;

    if (ix > num_collapsed_hands) {
      printf("error:  ix = %d\n",ix);
      printf("  rank_ix1 = %d\n",rank_ix1);
      printf("  suit_ix1 = %d\n",suit_ix1);
      printf("  rank_ix2 = %d\n",rank_ix2);
      printf("  suit_ix2 = %d\n",suit_ix2);
      ix = 0;
    }

    aggreg[ix].hand_count++;

    if (Contains(true,
      line,line_len,
      fbf_str,FBF_STR_LEN,
      &ix2)) {

      aggreg[ix].num_fbfs++;
    }

    if (Contains(true,
      line,line_len,
      sf_str,SF_STR_LEN,
      &ix2)) {

      aggreg[ix].num_flops_seen++;
    }

    if (Contains(true,
      line,line_len,
      ws_str,WS_STR_LEN,
      &ix2)) {

      aggreg[ix].num_wins++;
    }
    else
      aggreg[ix].num_losses++;
  }

  fclose(fptr);

  for (o = 0; o < NUM_COLLAPSED_HANDS; o++) {
    aggreg[o].freq_factor = (double)aggreg[o].hand_count * periodicities[aggreg[o].handtype] /
      (double)total_hand_count;
    if (aggreg[o].hand_count) {
      aggreg[o].fbf_pct = (double)aggreg[o].num_fbfs / (double)aggreg[o].hand_count * (double)100;
      aggreg[o].flops_seen_pct = (double)aggreg[o].num_flops_seen / (double)aggreg[o].hand_count * (double)100;
      aggreg[o].win_pct = (double)aggreg[o].num_wins / (double)aggreg[o].hand_count * (double)100;
    }
    else {
      aggreg[o].flops_seen_pct = (double)0;
      aggreg[o].win_pct = (double)0;
    }

    if (o < NUM_CARDS_IN_SUIT) {
      for (n = 0; n < 2; n++)
        aggreg[o].card_string[n] = rank_chars[o];

      aggreg[o].card_string[n++] = ' ';
      aggreg[o].card_string[n] = 0;
    }
    else if (o < NUM_CARDS_IN_SUIT + NUM_SUITED_NONPAIRS) {
      aggreg[o].card_string[2] = 's';

      get_permutation_instance(
        NUM_CARDS_IN_SUIT,
        &m,&n,o - NUM_CARDS_IN_SUIT);

      aggreg[o].card_string[0] = rank_chars[n];

      aggreg[o].card_string[1] = rank_chars[m];
    }
    else {
      aggreg[o].card_string[2] = 'o';

      get_permutation_instance(
        NUM_CARDS_IN_SUIT,
        &m,&n,o - (NUM_CARDS_IN_SUIT + NUM_SUITED_NONPAIRS));

      aggreg[o].card_string[0] = rank_chars[n];

      aggreg[o].card_string[1] = rank_chars[m];
    }
  }

  for (o = 0; o < NUM_COLLAPSED_HANDS; o++)
    ixs[o] = o;

  if (bSortByFreq)
    qsort(ixs,NUM_COLLAPSED_HANDS,sizeof (int),compare);
  else if (bSortByCount)
    qsort(ixs,NUM_COLLAPSED_HANDS,sizeof (int),compare2);

  for (o = 0; o < NUM_COLLAPSED_HANDS; o++) {
    ix = ixs[o];
    bPrint = true;

    if (bOnlyMissing) {
      if (!bNot) {
        if (aggreg[ix].hand_count)
          bPrint = false;
      }
      else {
        if (!aggreg[ix].hand_count)
          bPrint = false;
      }
    }

    if (bPrint && bOnlyPremium) {
      if (!is_premium_hand(aggreg[ix].card_string,&premium_ix))
        bPrint = false;
    }

    if (bPrint) {
      if (bTerse) {
        printf("%d %s\n",aggreg[ix].hand_count,aggreg[ix].card_string);
      }
      else if (bVerbose) {
        printf("%-3s %6d %6d %6d %6.2lf %6d %11.4lf\n",
          aggreg[ix].card_string,
          aggreg[ix].num_wins,
          aggreg[ix].num_losses,
          aggreg[ix].hand_count,
          aggreg[ix].win_pct,
          total_hand_count,
          aggreg[ix].freq_factor);
      }
      else {
        printf("%-3s %6d\n",
          aggreg[ix].card_string,
          aggreg[ix].hand_count);
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

static bool Contains(bool bCaseSens,char *line,int line_len,
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

static int index_of_hand(int rank_ix1,int suit_ix1,int rank_ix2,int suit_ix2,hand_type *handtype_pt)
{
  int n;
  int work;
  int index;
  int num_other_cards;

  if (rank_ix1 == rank_ix2) {
    *handtype_pt = HAND_TYPE_PAIR;
    return rank_ix1;
  }

  if (suit_ix1 == suit_ix2) {
    *handtype_pt = HAND_TYPE_SUITED_NONPAIR;
    index = NUM_CARDS_IN_SUIT;
  }
  else {
    *handtype_pt = HAND_TYPE_NONSUITED_NONPAIR;
    index = NUM_CARDS_IN_SUIT + NUM_SUITED_NONPAIRS;
  }

  if (rank_ix1 > rank_ix2) {
    work = rank_ix1;
    rank_ix1 = rank_ix2;
    rank_ix2 = work;
  }

  num_other_cards = NUM_CARDS_IN_SUIT - 1;

  for (n = 0; n < rank_ix1; n++) {
    index += num_other_cards;
    num_other_cards--;
  }

  index += rank_ix2 - rank_ix1 - 1;

  return index;
}

static void get_permutation_instance(
  int set_size,
  int *m,int *n,
  int instance_ix
)
{
  if (instance_ix)
    goto after_return_point;

  for (*m = 0; *m < set_size - 1; (*m)++) {
    for (*n = *m + 1; *n < set_size; (*n)++) {
      return;

      after_return_point:
      ;
    }
  }
}

int compare(const void *elem1,const void *elem2)
{
  int int1;
  int int2;

  int1 = *(int *)elem1;
  int2 = *(int *)elem2;

  if (aggreg[int2].freq_factor < aggreg[int1].freq_factor)
    return -1;
  else if (aggreg[int2].freq_factor > aggreg[int1].freq_factor)
    return 1;
  else
    return 0;
}

int compare2(const void *elem1,const void *elem2)
{
  int int1;
  int int2;

  int1 = *(int *)elem1;
  int2 = *(int *)elem2;

  return aggreg[int2].hand_count - aggreg[int1].hand_count;
}
