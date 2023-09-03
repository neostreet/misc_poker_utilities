#include <stdio.h>

enum Suit {
  CLUBS,
  DIAMONDS,
  HEARTS,
  SPADES,
  NUM_SUITS
};

enum Rank {
  TWO,
  THREE,
  FOUR,
  FIVE,
  SIX,
  SEVEN,
  EIGHT,
  NINE,
  TEN,
  JACK,
  QUEEN,
  KING,
  ACE,
  NUM_CARDS_IN_SUIT
};

#define NUM_CARDS_IN_DECK (NUM_SUITS * NUM_CARDS_IN_SUIT)

#define suit_of(card) (card / NUM_CARDS_IN_SUIT)
#define rank_of(card) (card % NUM_CARDS_IN_SUIT)

#define NUM_CARDS_IN_FLOP 3

#define POKER_52_3_PERMUTATIONS 22100

static void get_permutation_instance(
  int set_size,int subset_size,int *m,int *n,int *o,int instance_ix
);

int main(int argc,char **argv)
{
  int m;
  int n;
  int o;
  int p;
  int rainbow_count;
  int flush_count;
  int other_count;
  int flop_suits[NUM_CARDS_IN_FLOP];
  double rainbow_pct;
  double flush_pct;
  double other_pct;

  rainbow_count = 0;
  flush_count = 0;

  for (p = 0; p < POKER_52_3_PERMUTATIONS; p++) {
    get_permutation_instance(
      NUM_CARDS_IN_DECK,NUM_CARDS_IN_FLOP,
      &m,&n,&o,p);

    flop_suits[0] = suit_of(m);
    flop_suits[1] = suit_of(n);
    flop_suits[2] = suit_of(o);

    if ((flop_suits[0] != flop_suits[1]) &&
        (flop_suits[0] != flop_suits[2]) &&
        (flop_suits[1] != flop_suits[2])) {
      rainbow_count++;
    }
    else if ((flop_suits[0] == flop_suits[1]) &&
        (flop_suits[0] == flop_suits[2])) {
      flush_count++;
    }
  }

  other_count = POKER_52_3_PERMUTATIONS - (rainbow_count + flush_count);

  rainbow_pct = (double)rainbow_count / (double)POKER_52_3_PERMUTATIONS;
  flush_pct = (double)flush_count / (double)POKER_52_3_PERMUTATIONS;
  other_pct = (double)other_count / (double)POKER_52_3_PERMUTATIONS;

  printf("rainbows: %5d %5d %lf\n",
    rainbow_count,POKER_52_3_PERMUTATIONS,rainbow_pct);
  printf("flushes:  %5d %5d %lf\n",
    flush_count,POKER_52_3_PERMUTATIONS,flush_pct);
  printf("others:   %5d %5d %lf\n",
    other_count,POKER_52_3_PERMUTATIONS,other_pct);

  return 0;
}

static void get_permutation_instance(
  int set_size,int subset_size,int *m,int *n,int *o,int instance_ix
)
{
  if (instance_ix)
    goto after_return_point;

  for (*m = 0; *m < set_size - subset_size + 1; (*m)++) {
    for (*n = *m + 1; *n < set_size - subset_size + 2; (*n)++) {
      for (*o = *n + 1; *o < set_size - subset_size + 3; (*o)++) {
        return;

        after_return_point:
        ;
      }
    }
  }
}
