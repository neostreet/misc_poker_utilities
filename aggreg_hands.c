#include <stdio.h>
#include <string.h>

static char usage[] =
"usage: aggreg_hands (-debug) (-reverse) (-not_missing) filename\n";
static char couldnt_open[] = "couldn't open %s\n";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_CARDS_IN_SUIT 13
#define NUM_SUITS 4

#define NUM_CARDS_IN_DECK (NUM_SUITS * NUM_CARDS_IN_SUIT)

#define rank_of(card) (card % NUM_CARDS_IN_SUIT)

struct aggreg_info {
  int hand_count;
  int sum_delta;
  int sum_wins;
  int sum_losses;
  int num_wins;
  int num_losses;
  int num_washes;
};

#define POKER_52_2_PERMUTATIONS      1326
static struct aggreg_info aggreg[POKER_52_2_PERMUTATIONS];

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

static void get_permutation_instance_reverse(
  int set_size,
  int *m,int *n,
  int instance_ix
);
int card_string_from_card_value(int card_value,char *card_string);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bReverse;
  bool bNotMissing;
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
  int delta;
  int ix;
  char card_string[2][3];
  int total_hand_count;
  int total_sum_delta;
  int total_sum_wins;
  int total_sum_losses;
  int total_num_wins;
  int total_num_losses;
  int total_num_washes;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bReverse = false;
  bNotMissing = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-reverse"))
      bReverse = true;
    else if (!strcmp(argv[curr_arg],"-not_missing"))
      bNotMissing = true;
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

  for (n = 0; n < POKER_52_2_PERMUTATIONS; n++) {
    aggreg[n].hand_count = 0;
    aggreg[n].sum_delta = 0;
    aggreg[n].sum_wins = 0;
    aggreg[n].sum_losses = 0;
    aggreg[n].num_wins = 0;
    aggreg[n].num_losses = 0;
    aggreg[n].num_washes = 0;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    for (denom_ix1 = 0; denom_ix1 < NUM_CARDS_IN_SUIT; denom_ix1++) {
      if (line[11] == rank_chars[denom_ix1])
        break;
    }

    if (denom_ix1 == NUM_CARDS_IN_SUIT) {
      printf(bad_denom_in_line,line_no,line);
      return 4;
    }

    for (suit_ix1 = 0; suit_ix1 < NUM_SUITS; suit_ix1++) {
      if (line[12] == suit_chars[suit_ix1])
        break;
    }

    if (suit_ix1 == NUM_SUITS) {
      printf(bad_suit_in_line,line_no,line);
      return 5;
    }

    card_ix1 = suit_ix1 * NUM_CARDS_IN_SUIT + denom_ix1;

    for (denom_ix2 = 0; denom_ix2 < NUM_CARDS_IN_SUIT; denom_ix2++) {
      if (line[14] == rank_chars[denom_ix2])
        break;
    }

    if (denom_ix2 == NUM_CARDS_IN_SUIT) {
      printf(bad_denom_in_line,line_no,line);
      return 6;
    }

    for (suit_ix2 = 0; suit_ix2 < NUM_SUITS; suit_ix2++) {
      if (line[15] == suit_chars[suit_ix2])
        break;
    }

    if (suit_ix2 == NUM_SUITS) {
      printf(bad_suit_in_line,line_no,line);
      return 7;
    }

    card_ix2 = suit_ix2 * NUM_CARDS_IN_SUIT + denom_ix2;

    sscanf(&line[0],"%d",&delta);

    ix = index_of_hand(card_ix1,card_ix2);

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

  total_hand_count = 0;
  total_sum_delta = 0;
  total_sum_wins = 0;
  total_sum_losses = 0;
  total_num_wins = 0;
  total_num_losses = 0;
  total_num_washes = 0;

  for (n = 0; n < 2; n++)
    card_string[n][2] = 0;

  for (o = 0; o < POKER_52_2_PERMUTATIONS; o++) {
    if (!bReverse) {
      get_permutation_instance(
        NUM_CARDS_IN_DECK,
        &m,&n,o);
    }
    else {
      get_permutation_instance_reverse(
        NUM_CARDS_IN_DECK,
        &m,&n,o);
    }

    if (bNotMissing){
      if (!aggreg[o].hand_count)
        continue;
    }

    card_string_from_card_value(m,card_string[0]);
    card_string_from_card_value(n,card_string[1]);

    if (rank_of(m) >= rank_of(n))
      printf(fmt,card_string[0],card_string[1]);
    else
      printf(fmt,card_string[1],card_string[0]);

    printf("%10d %10d %10d %6d %6d %6d %6d\n",
      aggreg[o].sum_delta,
      aggreg[o].sum_wins,
      aggreg[o].sum_losses,
      aggreg[o].num_wins,
      aggreg[o].num_losses,
      aggreg[o].num_washes,
      aggreg[o].hand_count);
    total_hand_count += aggreg[o].hand_count;
    total_sum_delta += aggreg[o].sum_delta;
    total_sum_wins += aggreg[o].sum_wins;
    total_sum_losses += aggreg[o].sum_losses;
    total_num_wins += aggreg[o].num_wins;

    if (bDebug)
      printf("debug: total_num_wins = %6d\n",total_num_wins);

    total_num_losses += aggreg[o].num_losses;
    total_num_washes += aggreg[o].num_washes;
  }

  printf("\n      %10d %10d %10d %6d %6d %6d %6d\n",
    total_sum_delta,
    total_sum_wins,
    total_sum_losses,
    total_num_wins,
    total_num_losses,
    total_num_washes,
    total_hand_count);

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

static void get_permutation_instance_reverse(
  int set_size,
  int *m,int *n,
  int instance_ix
)
{
  if (instance_ix)
    goto after_return_point;

  for (*m = set_size - 1; *m > 0; (*m)--) {
    for (*n = *m - 1; *n >= 0; (*n)--) {
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
