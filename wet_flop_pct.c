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
bool straight(int *flop_ranks);

int main(int argc,char **argv)
{
  int m;
  int n;
  int o;
  int p;
  int wet_flop_count;
  int flop_suits[NUM_CARDS_IN_FLOP];
  int flop_ranks[NUM_CARDS_IN_FLOP];
  double wet_flop_pct;

  wet_flop_count = 0;

  for (p = 0; p < POKER_52_3_PERMUTATIONS; p++) {
    get_permutation_instance(
      NUM_CARDS_IN_DECK,NUM_CARDS_IN_FLOP,
      &m,&n,&o,p);

    flop_suits[0] = suit_of(m);
    flop_suits[1] = suit_of(n);
    flop_suits[2] = suit_of(o);

    flop_ranks[0] = rank_of(m);
    flop_ranks[1] = rank_of(n);
    flop_ranks[2] = rank_of(o);

    if ((flop_suits[0] == flop_suits[1]) && (flop_suits[0] == flop_suits[2])) {
      // flush
      wet_flop_count++;
    }
    else if (straight(flop_ranks)) {
      // straight
      wet_flop_count++;
    }
    else if ((flop_ranks[0] == flop_ranks[1]) ||
             (flop_ranks[0] == flop_ranks[2]) ||
             (flop_ranks[1] == flop_ranks[2])) {
      // pair or better
      wet_flop_count++;
    }
  }

  wet_flop_pct = (double)wet_flop_count / (double)POKER_52_3_PERMUTATIONS;

  printf("%5d %5d %lf\n",
    wet_flop_count,POKER_52_3_PERMUTATIONS,wet_flop_pct);

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

bool straight(int *flop_ranks)
{
  int n;
  int min_rank;
  int max_rank;
  int wheel_max_rank;

  for (n = 0; n < NUM_CARDS_IN_FLOP; n++) {
    if (!n) {
      min_rank = flop_ranks[n];
      max_rank = flop_ranks[n];
    }
    else {
      if (flop_ranks[n] < min_rank)
        min_rank = flop_ranks[n];
      else if (flop_ranks[n] > max_rank)
        max_rank = flop_ranks[n];
    }
  }

  // handle wheel case
  if (max_rank == 12) {
    wheel_max_rank = 12;

    for (n = 0; n < NUM_CARDS_IN_FLOP; n++) {
      if (wheel_max_rank == 12) {
        if (flop_ranks[n] != 12)
          wheel_max_rank = flop_ranks[n];
      }
      else {
        if (flop_ranks[n] != 12) {
          if (flop_ranks[n] > wheel_max_rank)
            wheel_max_rank = flop_ranks[n];
        }
      }
    }

    if (wheel_max_rank <= 3)
      return true;
  }

  return (max_rank - min_rank <= 4);
}
