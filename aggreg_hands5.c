#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char usage[] =
"usage: aggreg_hands5 (-debug) (-sort_by_freq) (-sort_by_total) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

#define MAX_FILENAME_LEN 1024
static char filename[MAX_FILENAME_LEN];

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
  double freq_factor;
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
  bool bSortByFreq;
  bool bSortByTotal;
  int dbg_ix;
  int dbg;
  int m;
  int n;
  int o;
  FILE *fptr0;
  int filename_len;
  FILE *fptr;
  int line_len;
  int rank_ix1;
  int suit_ix1;
  int rank_ix2;
  int suit_ix2;
  hand_type handtype;
  int ix;
  int total_hand_count;
  int num_collapsed_hands;
  int ixs[NUM_COLLAPSED_HANDS];

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bSortByFreq = false;
  bSortByTotal = false;

  dbg_ix = -1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-sort_by_freq"))
      bSortByFreq = true;
    else if (!strcmp(argv[curr_arg],"-sort_by_total"))
      bSortByTotal = true;
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

  num_collapsed_hands = NUM_COLLAPSED_HANDS;

  for (n = 0; n < num_collapsed_hands; n++)
    aggreg[n].hand_count = 0;

  total_hand_count = 0;

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 4;
  }

  for ( ; ; ) {
    GetLine(fptr0,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    if ((fptr = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    for ( ; ; ) {
      GetLine(fptr,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr))
        break;

      for (rank_ix1 = 0; rank_ix1 < NUM_CARDS_IN_SUIT; rank_ix1++) {
        if (line[0] == rank_chars[rank_ix1])
          break;
      }

      if (rank_ix1 == NUM_CARDS_IN_SUIT) {
        printf(bad_rank_in_line,total_hand_count+1,line);
        return 5;
      }

      for (suit_ix1 = 0; suit_ix1 < NUM_SUITS; suit_ix1++) {
        if (line[1] == suit_chars[suit_ix1])
          break;
      }

      if (suit_ix1 == NUM_SUITS) {
        printf(bad_suit_in_line,total_hand_count+1,line);
        return 6;
      }

      for (rank_ix2 = 0; rank_ix2 < NUM_CARDS_IN_SUIT; rank_ix2++) {
        if (line[3] == rank_chars[rank_ix2])
          break;
      }

      if (rank_ix2 == NUM_CARDS_IN_SUIT) {
        printf(bad_rank_in_line,total_hand_count+1,line);
        return 7;
      }

      for (suit_ix2 = 0; suit_ix2 < NUM_SUITS; suit_ix2++) {
        if (line[4] == suit_chars[suit_ix2])
          break;
      }

      if (suit_ix2 == NUM_SUITS) {
        printf(bad_suit_in_line,total_hand_count+1,line);
        return 8;
      }

      total_hand_count++;

      ix = index_of_hand(rank_ix1,suit_ix1,rank_ix2,suit_ix2,&handtype);

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
    }

    fclose(fptr);
  }

  fclose(fptr0);

  for (o = 0; o < NUM_COLLAPSED_HANDS; o++) {
    aggreg[o].freq_factor = (double)aggreg[o].hand_count * periodicities[aggreg[o].handtype] /
      (double)total_hand_count;

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
  else if (bSortByTotal)
    qsort(ixs,NUM_COLLAPSED_HANDS,sizeof (int),compare2);

  for (o = 0; o < NUM_COLLAPSED_HANDS; o++) {
    ix = ixs[o];

    printf("%-3s %6d %9.2lf %6d %11.4lf\n",
      aggreg[ix].card_string,
      aggreg[ix].hand_count,
      periodicities[aggreg[ix].handtype],
      total_hand_count,
      aggreg[ix].freq_factor);
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

  if (aggreg[int2].hand_count < aggreg[int1].hand_count)
    return -1;
  else
    return 1;
}
