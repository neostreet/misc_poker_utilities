#include <stdio.h>

static char usage[] = "usage: hand_counts filename\n";
static char couldnt_open[] = "couldn't open %s\n";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_CARDS_IN_SUIT 13
#define NUM_SUITS 4

#define NUM_CARDS_IN_DECK (NUM_SUITS * NUM_CARDS_IN_SUIT)
#define NUM_HOLE_CARDS_IN_HOLDEM_HAND 2

#define POKER_52_2_PERMUTATIONS      1326
static int hand_counts[POKER_52_2_PERMUTATIONS];

static char bad_denom_in_line[] = "bad denomination in line %d: %s\n";

char suit_chars[] = "cdhs";
char rank_chars[] = "23456789TJQKA";

static char bad_suit_in_line[] = "bad suit in line %d: %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int index_of_hand(int card_index1,int card_index2);
static void get_permutation_instance(
  int set_size,int subset_size,
  int *m,int *n,
  int instance_ix
);
int card_string_from_card_value(int card_value,char *card_string);

int main(int argc,char **argv)
{
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
  char card_string[3];

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  for (n = 0; n < POKER_52_2_PERMUTATIONS; n++)
    hand_counts[n] = 0;

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    for (denom_ix1 = 0; denom_ix1 < NUM_CARDS_IN_SUIT; denom_ix1++) {
      if (line[0] == rank_chars[denom_ix1])
        break;
    }

    if (denom_ix1 == NUM_CARDS_IN_SUIT) {
      printf(bad_denom_in_line,line_no,line);
      return 3;
    }

    for (suit_ix1 = 0; suit_ix1 < NUM_SUITS; suit_ix1++) {
      if (line[1] == suit_chars[suit_ix1])
        break;
    }

    if (suit_ix1 == NUM_SUITS) {
      printf(bad_suit_in_line,line_no,line);
      return 4;
    }

    card_ix1 = suit_ix1 * NUM_CARDS_IN_SUIT + denom_ix1;

    for (denom_ix2 = 0; denom_ix2 < NUM_CARDS_IN_SUIT; denom_ix2++) {
      if (line[3] == rank_chars[denom_ix2])
        break;
    }

    if (denom_ix2 == NUM_CARDS_IN_SUIT) {
      printf(bad_denom_in_line,line_no,line);
      return 5;
    }

    for (suit_ix2 = 0; suit_ix2 < NUM_SUITS; suit_ix2++) {
      if (line[4] == suit_chars[suit_ix2])
        break;
    }

    if (suit_ix2 == NUM_SUITS) {
      printf(bad_suit_in_line,line_no,line);
      return 6;
    }

    card_ix2 = suit_ix2 * NUM_CARDS_IN_SUIT + denom_ix2;

    ix = index_of_hand(card_ix1,card_ix2);

    hand_counts[ix]++;
  }

  fclose(fptr);

  card_string[2] = 0;

  for (o = 0; o < POKER_52_2_PERMUTATIONS; o++) {
    get_permutation_instance(
      NUM_CARDS_IN_DECK,NUM_HOLE_CARDS_IN_HOLDEM_HAND,
      &m,&n,o);

    printf("%10d ",hand_counts[o]);
    card_string_from_card_value(m,card_string);
    printf("%s ",card_string);
    card_string_from_card_value(n,card_string);
    printf("%s\n",card_string);
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
  int set_size,int subset_size,
  int *m,int *n,
  int instance_ix
)
{
  if (instance_ix)
    goto after_return_point;

  for (*m = 0; *m < set_size - subset_size + 1; (*m)++) {
    for (*n = *m + 1; *n < set_size - subset_size + 2; (*n)++) {
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
