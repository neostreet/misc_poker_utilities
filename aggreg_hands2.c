#include <stdio.h>
#include <string.h>

static char usage[] =
"usage: aggreg_hands2 (-debug) (-dbg_ixix) (-avgs) filename\n";
static char couldnt_open[] = "couldn't open %s\n";
static char avg_fmt[] = " %9.2lf %9.2lf\n";

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

#define NUM_CARDS_IN_SUIT 13
#define NUM_SUITS 4

#define NUM_SUITED_NONPAIRS ((NUM_CARDS_IN_SUIT * (NUM_CARDS_IN_SUIT - 1)) / 2)
#define NUM_NONSUITED_NONPAIRS NUM_SUITED_NONPAIRS

#define NUM_CARDS_IN_DECK (NUM_SUITS * NUM_CARDS_IN_SUIT)

struct aggreg_info {
  int hand_count;
  int sum_delta;
  int sum_wins;
  int sum_losses;
  int num_wins;
  int num_losses;
  int num_washes;
};

#define NUM_COLLAPSED_HANDS 169
static struct aggreg_info aggreg[NUM_COLLAPSED_HANDS];

static char bad_rank_in_line[] = "bad rank in line %d: %s\n";

char suit_chars[] = "cdhs";
char rank_chars[] = "23456789TJQKA";

static char bad_suit_in_line[] = "bad suit in line %d: %s\n";

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
static int index_of_hand(int rank_ix1,int suit_ix1,int rank_ix2,int suit_ix2);
static void get_permutation_instance(
  int set_size,
  int *m,int *n,
  int instance_ix
);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bAvgs;
  int dbg_ix;
  int dbg;
  int m;
  int n;
  int o;
  FILE *fptr;
  int line_len;
  int line_no;
  int rank_ix1;
  int suit_ix1;
  int rank_ix2;
  int suit_ix2;
  int delta;
  int ix;
  char card_string[4];
  int total_hand_count;
  int total_sum_delta;
  int total_sum_wins;
  int total_sum_losses;
  int total_num_wins;
  int total_num_losses;
  int total_num_washes;
  double win_avg;
  double loss_avg;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bAvgs = false;
  dbg_ix = -1;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-avgs"))
      bAvgs = true;
    else if (!strncmp(argv[curr_arg],"-dbg_ix",7))
      sscanf(&argv[curr_arg][7],"%d",&dbg_ix);
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

  for (n = 0; n < NUM_COLLAPSED_HANDS; n++) {
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

    for (rank_ix1 = 0; rank_ix1 < NUM_CARDS_IN_SUIT; rank_ix1++) {
      if (line[0] == rank_chars[rank_ix1])
        break;
    }

    if (rank_ix1 == NUM_CARDS_IN_SUIT) {
      printf(bad_rank_in_line,line_no,line);
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

    for (rank_ix2 = 0; rank_ix2 < NUM_CARDS_IN_SUIT; rank_ix2++) {
      if (line[3] == rank_chars[rank_ix2])
        break;
    }

    if (rank_ix2 == NUM_CARDS_IN_SUIT) {
      printf(bad_rank_in_line,line_no,line);
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

    sscanf(&line[6],"%d",&delta);

    ix = index_of_hand(rank_ix1,suit_ix1,rank_ix2,suit_ix2);

    if (ix == dbg_ix)
      dbg = 1;

    if (ix > NUM_COLLAPSED_HANDS) {
      printf("error:  ix = %d\n",ix);
      printf("  rank_ix1 = %d\n",rank_ix1);
      printf("  suit_ix1 = %d\n",suit_ix1);
      printf("  rank_ix2 = %d\n",rank_ix2);
      printf("  suit_ix2 = %d\n",suit_ix2);
      ix = 0;
    }

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

  card_string[3] = 0;

  total_hand_count = 0;
  total_sum_delta = 0;
  total_sum_wins = 0;
  total_sum_losses = 0;
  total_num_wins = 0;
  total_num_losses = 0;
  total_num_washes = 0;

  for (o = 0; o < NUM_CARDS_IN_SUIT; o++) {
    for (n = 0; n < 2; n++)
      card_string[n] = rank_chars[o];

    card_string[n] = ' ';

    printf("%-3s %10d %10d %10d %6d %6d %6d %6d",card_string,
      aggreg[o].sum_delta,
      aggreg[o].sum_wins,
      aggreg[o].sum_losses,
      aggreg[o].num_wins,
      aggreg[o].num_losses,
      aggreg[o].num_washes,
      aggreg[o].hand_count);
    total_sum_delta += aggreg[o].sum_delta;
    total_sum_wins += aggreg[o].sum_wins;
    total_sum_losses += aggreg[o].sum_losses;
    total_num_wins += aggreg[o].num_wins;
    total_num_losses += aggreg[o].num_losses;
    total_num_washes += aggreg[o].num_washes;
    total_hand_count += aggreg[o].hand_count;

    if (bAvgs) {
      if (!aggreg[o].num_wins)
        win_avg = (double)0;
      else
        win_avg = (double)aggreg[o].sum_wins / (double)aggreg[o].num_wins;

      if (!aggreg[o].num_losses)
        loss_avg = (double)0;
      else
        loss_avg = (double)aggreg[o].sum_losses / (double)aggreg[o].num_losses;

      printf(avg_fmt,win_avg,loss_avg);
    }
    else
      putchar(0x0a);
  }

  card_string[2] = 's';

  for (o = 0; o < NUM_SUITED_NONPAIRS; o++) {
    get_permutation_instance(
      NUM_CARDS_IN_SUIT,
      &m,&n,o);

    if (m > n) {
      card_string[0] = rank_chars[m];
      card_string[1] = rank_chars[n];
    }
    else {
      card_string[0] = rank_chars[n];
      card_string[1] = rank_chars[m];
    }

    printf("%-3s %10d %10d %10d %6d %6d %6d %6d",card_string,
      aggreg[NUM_CARDS_IN_SUIT+o].sum_delta,
      aggreg[NUM_CARDS_IN_SUIT+o].sum_wins,
      aggreg[NUM_CARDS_IN_SUIT+o].sum_losses,
      aggreg[NUM_CARDS_IN_SUIT+o].num_wins,
      aggreg[NUM_CARDS_IN_SUIT+o].num_losses,
      aggreg[NUM_CARDS_IN_SUIT+o].num_washes,
      aggreg[NUM_CARDS_IN_SUIT+o].hand_count);
    total_sum_delta += aggreg[NUM_CARDS_IN_SUIT+o].sum_delta;
    total_sum_wins += aggreg[NUM_CARDS_IN_SUIT+o].sum_wins;
    total_sum_losses += aggreg[NUM_CARDS_IN_SUIT+o].sum_losses;
    total_num_wins += aggreg[NUM_CARDS_IN_SUIT+o].num_wins;
    total_num_losses += aggreg[NUM_CARDS_IN_SUIT+o].num_losses;
    total_num_washes += aggreg[NUM_CARDS_IN_SUIT+o].num_washes;
    total_hand_count += aggreg[NUM_CARDS_IN_SUIT+o].hand_count;

    if (bAvgs) {
      if (!aggreg[NUM_CARDS_IN_SUIT+o].num_wins)
        win_avg = (double)0;
      else
        win_avg = (double)aggreg[NUM_CARDS_IN_SUIT+o].sum_wins / (double)aggreg[NUM_CARDS_IN_SUIT+o].num_wins;

      if (!aggreg[NUM_CARDS_IN_SUIT+o].num_losses)
        loss_avg = (double)0;
      else
        loss_avg = (double)aggreg[NUM_CARDS_IN_SUIT+o].sum_losses / (double)aggreg[NUM_CARDS_IN_SUIT+o].num_losses;

      printf(avg_fmt,win_avg,loss_avg);
    }
    else
      putchar(0x0a);
  }

  card_string[2] = 'o';

  for (o = 0; o < NUM_NONSUITED_NONPAIRS; o++) {
    get_permutation_instance(
      NUM_CARDS_IN_SUIT,
      &m,&n,o);

    if (m > n) {
      card_string[0] = rank_chars[m];
      card_string[1] = rank_chars[n];
    }
    else {
      card_string[0] = rank_chars[n];
      card_string[1] = rank_chars[m];
    }

    printf("%-3s %10d %10d %10d %6d %6d %6d %6d",card_string,
      aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].sum_delta,
      aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].sum_wins,
      aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].sum_losses,
      aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_wins,
      aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_losses,
      aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_washes,
      aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].hand_count);
    total_sum_delta += aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].sum_delta;
    total_sum_wins += aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].sum_wins;
    total_sum_losses += aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].sum_losses;
    total_num_wins += aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_wins;
    total_num_losses += aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_losses;
    total_num_washes += aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_washes;
    total_hand_count += aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].hand_count;

    if (bAvgs) {
      if (!aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_wins)
        win_avg = (double)0;
      else
        win_avg = (double)aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].sum_wins / (double)aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_wins;

      if (!aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_losses)
        loss_avg = (double)0;
      else
        loss_avg = (double)aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].sum_losses / (double)aggreg[NUM_CARDS_IN_SUIT+NUM_SUITED_NONPAIRS+o].num_losses;

      printf(avg_fmt,win_avg,loss_avg);
    }
    else
      putchar(0x0a);
  }

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

static int index_of_hand(int rank_ix1,int suit_ix1,int rank_ix2,int suit_ix2)
{
  int n;
  int work;
  int index;
  int num_other_cards;

  if (rank_ix1 == rank_ix2)
    return rank_ix1;

  if (suit_ix1 == suit_ix2)
    index = NUM_CARDS_IN_SUIT;
  else
    index = NUM_CARDS_IN_SUIT + NUM_SUITED_NONPAIRS;

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
