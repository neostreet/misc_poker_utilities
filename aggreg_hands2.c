#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char usage[] =
"usage: aggreg_hands2 (-debug) (-verbose) (-dbg_ixix) (-totals) (-avgs)\n"
"  (-pairs_only) (-s_or_o_between) (-denomdenom) (-sort_by_freq) (-sort_by_total)\n"
"  (-only_missing) (-not) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char avg_fmt[] = " %9.2lf %9.2lf\n";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_CARDS_IN_SUIT 13
#define NUM_SUITS 4

#define NUM_SUITED_NONPAIRS ((NUM_CARDS_IN_SUIT * (NUM_CARDS_IN_SUIT - 1)) / 2)
#define NUM_NONSUITED_NONPAIRS NUM_SUITED_NONPAIRS

#define NUM_CARDS_IN_DECK (NUM_SUITS * NUM_CARDS_IN_SUIT)

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
  int sum_delta;
  int sum_wins;
  int sum_losses;
  int num_wins;
  int num_losses;
  int num_washes;
  double freq_factor;
  double win_pct;
};

#define NUM_COLLAPSED_HANDS 169
static struct aggreg_info aggreg[NUM_COLLAPSED_HANDS];

static char bad_rank_in_line[] = "bad rank in line %d: %s\n";

char suit_chars[] = "cdhs";
char rank_chars[] = "23456789TJQKA";

static char bad_suit_in_line[] = "bad suit in line %d: %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
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
  bool bTotals;
  bool bAvgs;
  bool bPairsOnly;
  bool bSorOBetween;
  bool bDenom;
  char denom;
  bool bSortByFreq;
  bool bSortByTotal;
  bool bOnlyMissing;
  bool bPrint;
  bool bNot;
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
  int delta;
  int ix;
  int total_hand_count;
  int total_sum_delta;
  int total_sum_wins;
  int total_sum_losses;
  int total_num_wins;
  int total_num_losses;
  int total_num_washes;
  double win_avg;
  double loss_avg;
  int num_collapsed_hands;
  int ixs[NUM_COLLAPSED_HANDS];

  if ((argc < 2) || (argc > 14)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bTotals = false;
  bAvgs = false;
  bPairsOnly = false;
  bSorOBetween = false;
  bDenom = false;
  bSortByFreq = false;
  bSortByTotal = false;
  bOnlyMissing = false;
  bNot = false;
  dbg_ix = -1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-totals"))
      bTotals = true;
    else if (!strcmp(argv[curr_arg],"-avgs"))
      bAvgs = true;
    else if (!strncmp(argv[curr_arg],"-dbg_ix",7))
      sscanf(&argv[curr_arg][7],"%d",&dbg_ix);
    else if (!strcmp(argv[curr_arg],"-pairs_only"))
      bPairsOnly = true;
    else if (!strcmp(argv[curr_arg],"-s_or_o_between"))
      bSorOBetween = true;
    else if (!strncmp(argv[curr_arg],"-denom",6)) {
      bDenom = true;
      denom = argv[curr_arg][6];
    }
    else if (!strcmp(argv[curr_arg],"-sort_by_freq"))
      bSortByFreq = true;
    else if (!strcmp(argv[curr_arg],"-sort_by_total"))
      bSortByTotal = true;
    else if (!strcmp(argv[curr_arg],"-only_missing"))
      bOnlyMissing = true;
    else if (!strcmp(argv[curr_arg],"-not"))
      bNot = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bSortByFreq && bSortByTotal) {
    printf("can't specify both -sort_by_freq and -sort_by_total\n");
    return 3;
  }

  if (bPairsOnly)
    num_collapsed_hands = NUM_CARDS_IN_SUIT;
  else
    num_collapsed_hands = NUM_COLLAPSED_HANDS;

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  for (n = 0; n < num_collapsed_hands; n++) {
    aggreg[n].hand_count = 0;
    aggreg[n].sum_delta = 0;
    aggreg[n].sum_wins = 0;
    aggreg[n].sum_losses = 0;
    aggreg[n].num_wins = 0;
    aggreg[n].num_losses = 0;
    aggreg[n].num_washes = 0;
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
      return 5;
    }

    for (suit_ix1 = 0; suit_ix1 < NUM_SUITS; suit_ix1++) {
      if (line[12] == suit_chars[suit_ix1])
        break;
    }

    if (suit_ix1 == NUM_SUITS) {
      printf(bad_suit_in_line,total_hand_count+1,line);
      return 6;
    }

    for (rank_ix2 = 0; rank_ix2 < NUM_CARDS_IN_SUIT; rank_ix2++) {
      if (line[14] == rank_chars[rank_ix2])
        break;
    }

    if (rank_ix2 == NUM_CARDS_IN_SUIT) {
      printf(bad_rank_in_line,total_hand_count+1,line);
      return 7;
    }

    for (suit_ix2 = 0; suit_ix2 < NUM_SUITS; suit_ix2++) {
      if (line[15] == suit_chars[suit_ix2])
        break;
    }

    if (suit_ix2 == NUM_SUITS) {
      printf(bad_suit_in_line,total_hand_count+1,line);
      return 8;
    }

    if (bPairsOnly) {
      if (rank_ix1 != rank_ix2)
        continue;
    }

    total_hand_count++;

    sscanf(&line[0],"%d",&delta);

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

    aggreg[ix].handtype = handtype;
    aggreg[ix].hand_count++;
    aggreg[ix].sum_delta += delta;

    if (delta > 0) {
      aggreg[ix].sum_wins += delta;
      aggreg[ix].num_wins++;
    }
    else if (delta < 0) {
      aggreg[ix].sum_losses += delta;
      aggreg[ix].num_losses++;
    }
    else
      aggreg[ix].num_washes++;
  }

  fclose(fptr);

  for (o = 0; o < NUM_COLLAPSED_HANDS; o++) {
    aggreg[o].freq_factor = (double)aggreg[o].hand_count * periodicities[aggreg[o].handtype] /
      (double)total_hand_count;
    if (aggreg[o].hand_count)
      aggreg[o].win_pct = (double)aggreg[o].num_wins / (double)aggreg[o].hand_count;
    else
      aggreg[o].win_pct = (double)0;

    if (o < NUM_CARDS_IN_SUIT) {
      for (n = 0; n < 2; n++)
        aggreg[o].card_string[n] = rank_chars[o];

      aggreg[o].card_string[n++] = ' ';
      aggreg[o].card_string[n] = 0;
    }
    else if (o < NUM_CARDS_IN_SUIT + NUM_SUITED_NONPAIRS) {
      if (!bSorOBetween)
        aggreg[o].card_string[2] = 's';
      else
        aggreg[o].card_string[1] = 's';

      get_permutation_instance(
        NUM_CARDS_IN_SUIT,
        &m,&n,o - NUM_CARDS_IN_SUIT);

      aggreg[o].card_string[0] = rank_chars[n];

      if (!bSorOBetween)
        aggreg[o].card_string[1] = rank_chars[m];
      else
        aggreg[o].card_string[2] = rank_chars[m];
    }
    else {
      if (!bSorOBetween)
        aggreg[o].card_string[2] = 'o';
      else
        aggreg[o].card_string[1] = 'o';

      get_permutation_instance(
        NUM_CARDS_IN_SUIT,
        &m,&n,o - (NUM_CARDS_IN_SUIT + NUM_SUITED_NONPAIRS));

      aggreg[o].card_string[0] = rank_chars[n];

      if (!bSorOBetween)
        aggreg[o].card_string[1] = rank_chars[m];
      else
        aggreg[o].card_string[2] = rank_chars[m];
    }
  }

  total_sum_delta = 0;
  total_sum_wins = 0;
  total_sum_losses = 0;
  total_num_wins = 0;
  total_num_losses = 0;
  total_num_washes = 0;

  for (o = 0; o < NUM_COLLAPSED_HANDS; o++)
    ixs[o] = o;

  if (bSortByFreq)
    qsort(ixs,NUM_COLLAPSED_HANDS,sizeof (int),compare);
  else if (bSortByTotal)
    qsort(ixs,NUM_COLLAPSED_HANDS,sizeof (int),compare2);

  for (o = 0; o < NUM_COLLAPSED_HANDS; o++) {
    ix = ixs[o];

    if (bDenom && (aggreg[ix].card_string[0] != denom))
      continue;

    bPrint = false;

    if (bVerbose) {
      if (bOnlyMissing) {
        if (!bNot) {
          if (!aggreg[ix].hand_count)
            bPrint = true;
        }
        else {
          if (aggreg[ix].hand_count)
            bPrint = true;
        }
      }
      else
        bPrint = true;

      if (bPrint) {
        printf("%-3s %10d %10d %10d %6d %6d %6d %6d %d %9.2lf %6d %11.4lf %6.4lf",
          aggreg[ix].card_string,
          aggreg[ix].sum_delta,
          aggreg[ix].sum_wins,
          aggreg[ix].sum_losses,
          aggreg[ix].num_wins,
          aggreg[ix].num_losses,
          aggreg[ix].num_washes,
          aggreg[ix].hand_count,
          aggreg[ix].handtype,
          periodicities[aggreg[ix].handtype],
          total_hand_count,
          aggreg[ix].freq_factor,
          aggreg[ix].win_pct);
      }
    }
    else {
      if (bOnlyMissing) {
        if (!bNot) {
          if (!aggreg[ix].hand_count)
            bPrint = true;
        }
        else {
          if (aggreg[ix].hand_count)
            bPrint = true;
        }
      }
      else
        bPrint = true;

      if (bPrint) {
        printf("%-3s %10d %10d %10d %6d %6d %6d %6d %6d %11.4lf %6.4lf",
          aggreg[ix].card_string,
          aggreg[ix].sum_delta,
          aggreg[ix].sum_wins,
          aggreg[ix].sum_losses,
          aggreg[ix].num_wins,
          aggreg[ix].num_losses,
          aggreg[ix].num_washes,
          aggreg[ix].hand_count,
          total_hand_count,
          aggreg[ix].freq_factor,
          aggreg[ix].win_pct);
      }
    }

    total_sum_delta += aggreg[ix].sum_delta;
    total_sum_wins += aggreg[ix].sum_wins;
    total_sum_losses += aggreg[ix].sum_losses;
    total_num_wins += aggreg[ix].num_wins;
    total_num_losses += aggreg[ix].num_losses;
    total_num_washes += aggreg[ix].num_washes;

    if (bAvgs && bPrint) {
      if (!aggreg[ix].num_wins)
        win_avg = (double)0;
      else
        win_avg = (double)aggreg[ix].sum_wins / (double)aggreg[ix].num_wins;

      if (!aggreg[ix].num_losses)
        loss_avg = (double)0;
      else
        loss_avg = (double)aggreg[ix].sum_losses / (double)aggreg[ix].num_losses;

      printf(avg_fmt,win_avg,loss_avg);
    }
    else {
      if (bPrint)
        putchar(0x0a);
    }
  }

  if (bTotals) {
    printf("\n    %10d %10d %10d %6d %6d %6d %6d",
      total_sum_delta,
      total_sum_wins,
      total_sum_losses,
      total_num_wins,
      total_num_losses,
      total_num_washes,
      total_hand_count);

    if (bAvgs) {
      if (!total_num_wins)
        win_avg = (double)0;
      else
        win_avg = (double)total_sum_wins / (double)total_num_wins;

      if (!total_num_losses)
        loss_avg = (double)0;
      else
        loss_avg = (double)total_sum_losses / (double)total_num_losses;

      printf(avg_fmt,win_avg,loss_avg);
    }
    else
      putchar(0x0a);
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
  else
    return 1;
}

int compare2(const void *elem1,const void *elem2)
{
  int int1;
  int int2;

  int1 = *(int *)elem1;
  int2 = *(int *)elem2;

  if (aggreg[int2].sum_delta < aggreg[int1].sum_delta)
    return -1;
  else
    return 1;
}
