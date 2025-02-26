#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char usage[] =
"usage: aggreg_hands_lite (-debug) (-not_missing) (-sort_by_count)\n"
"  (-descending) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_CARDS_IN_SUIT 13
#define NUM_SUITS 4

#define NUM_CARDS_IN_DECK (NUM_SUITS * NUM_CARDS_IN_SUIT)

#define rank_of(card) (card % NUM_CARDS_IN_SUIT)

static bool bDescending;

struct aggreg_info {
  int card_values[2];
  int hand_count;
};

#define POKER_52_2_PERMUTATIONS      1326
static struct aggreg_info aggreg[POKER_52_2_PERMUTATIONS];
static int ixs[POKER_52_2_PERMUTATIONS];

static char bad_denom_in_line[] = "bad denomination in line %d: %s\n";

char suit_chars[] = "cdhs";
char rank_chars[] = "23456789TJQKA";

static char bad_suit_in_line[] = "bad suit in line %d: %s\n";

static char fmt[] = "%s %s ";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int index_of_hand(int card_index1,int card_index2);
static void get_permutation_instance(
  int set_size,
  int *m,int *n,
  int instance_ix
);
int card_string_from_card_value(int card_value,char *card_string);
int compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bNotMissing;
  bool bSortByCount;
  int m;
  int n;
  int o;
  FILE *fptr;
  int line_len;
  int line_no;
  int denom_ix1;
  int suit_ix1;
  int card_ix1;
  int denom_ix2;
  int suit_ix2;
  int card_ix2;
  int ix;
  char card_string[2][3];
  int total_hand_count;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bNotMissing = false;
  bSortByCount = false;
  bDescending = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-not_missing"))
      bNotMissing = true;
    else if (!strcmp(argv[curr_arg],"-sort_by_count"))
      bSortByCount = true;
    else if (!strcmp(argv[curr_arg],"-descending"))
      bDescending = true;
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

  for (o = 0; o < POKER_52_2_PERMUTATIONS; o++) {
    get_permutation_instance(
      NUM_CARDS_IN_DECK,
      &m,&n,o);

    aggreg[o].card_values[0] = m;
    aggreg[o].card_values[1] = n;
    aggreg[o].hand_count = 0;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    if ((line[0] >= 'a') && (line[0] <= 'z'))
      line[0] -= 'a' - 'A';

    for (denom_ix1 = 0; denom_ix1 < NUM_CARDS_IN_SUIT; denom_ix1++) {
      if (line[0] == rank_chars[denom_ix1])
        break;
    }

    if (denom_ix1 == NUM_CARDS_IN_SUIT) {
      printf(bad_denom_in_line,line_no,line);
      return 4;
    }

    for (suit_ix1 = 0; suit_ix1 < NUM_SUITS; suit_ix1++) {
      if (line[1] == suit_chars[suit_ix1])
        break;
    }

    if (suit_ix1 == NUM_SUITS) {
      printf(bad_suit_in_line,line_no,line);
      return 5;
    }

    card_ix1 = suit_ix1 * NUM_CARDS_IN_SUIT + denom_ix1;

    if ((line[3] >= 'a') && (line[3] <= 'z'))
      line[3] -= 'a' - 'A';

    for (denom_ix2 = 0; denom_ix2 < NUM_CARDS_IN_SUIT; denom_ix2++) {
      if (line[3] == rank_chars[denom_ix2])
        break;
    }

    if (denom_ix2 == NUM_CARDS_IN_SUIT) {
      printf(bad_denom_in_line,line_no,line);
      return 6;
    }

    for (suit_ix2 = 0; suit_ix2 < NUM_SUITS; suit_ix2++) {
      if (line[4] == suit_chars[suit_ix2])
        break;
    }

    if (suit_ix2 == NUM_SUITS) {
      printf(bad_suit_in_line,line_no,line);
      return 7;
    }

    card_ix2 = suit_ix2 * NUM_CARDS_IN_SUIT + denom_ix2;

    ix = index_of_hand(card_ix1,card_ix2);

    aggreg[ix].hand_count++;
  }

  fclose(fptr);

  total_hand_count = 0;

  for (n = 0; n < 2; n++)
    card_string[n][2] = 0;

  for (n = 0; n < POKER_52_2_PERMUTATIONS; n++)
    ixs[n] = n;

  if (bSortByCount)
    qsort(ixs,POKER_52_2_PERMUTATIONS,sizeof (int),compare);

  for (n = 0; n < POKER_52_2_PERMUTATIONS; n++) {
    ix = ixs[n];

    if (bNotMissing){
      if (!aggreg[ix].hand_count)
        continue;
    }

    card_string_from_card_value(aggreg[ix].card_values[0],card_string[0]);
    card_string_from_card_value(aggreg[ix].card_values[1],card_string[1]);

    if (rank_of(aggreg[ix].card_values[0]) >= rank_of(aggreg[ix].card_values[1]))
      printf(fmt,card_string[0],card_string[1]);
    else
      printf(fmt,card_string[1],card_string[0]);

    printf("%6d\n",aggreg[ix].hand_count);

    total_hand_count += aggreg[ix].hand_count;
  }

  printf("\n      %6d\n",total_hand_count);

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

static int index_of_hand(int card_index1,int card_index2)
{
  int n;
  int work;
  int index;
  int num_other_cards;

  if (card_index1 > card_index2) {
    work = card_index1;
    card_index1 = card_index2;
    card_index2 = work;
  }

  index = 0;
  num_other_cards = NUM_CARDS_IN_DECK - 1;

  for (n = 0; n < card_index1; n++) {
    index += num_other_cards;
    num_other_cards--;
  }

  index += card_index2 - card_index1 - 1;

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

int card_string_from_card_value(int card_value,char *card_string)
{
  int rank_ix;
  int suit_ix;

  if (card_value >= NUM_CARDS_IN_DECK)
    return 1;

  rank_ix = card_value % NUM_CARDS_IN_SUIT;
  suit_ix = card_value / NUM_CARDS_IN_SUIT;

  card_string[0] = rank_chars[rank_ix];
  card_string[1] = suit_chars[suit_ix];

  return 0;
}

int compare(const void *elem1,const void *elem2)
{
  int int1;
  int int2;

  int1 = *(int *)elem1;
  int2 = *(int *)elem2;

  if (!bDescending)
    return aggreg[int1].hand_count - aggreg[int2].hand_count;
  else
    return aggreg[int2].hand_count - aggreg[int1].hand_count;
}
