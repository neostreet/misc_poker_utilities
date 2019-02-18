#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ko_payout_in {
  int base_payout;
  int place;
  int min_bounties;
  int max_bounties;
};

struct ko_payout_out {
  int base_payout;
  int place;
  int num_bounties;
  int total_payout;
};

#define BUY_IN 50000
#define BOUNTY 21250
#define BOUNTY_NO_ENTRY_FEE 25000

struct ko_payout_in ko_payouts_in[] = {
  {95625, 1, 2, 9},
  {57375, 2, 0, 7},
  {38250, 3, 0, 6},
  {0, 0, 0, 5}
};

struct ko_payout_in ko_payouts_no_entry_fee_in[] = {
  {112500, 1, 2, 9},
  {67500, 2, 0, 7},
  {45000, 3, 0, 6},
  {0, 0, 0, 5}
};

#define NUM_BASES (sizeof ko_payouts_in / sizeof (struct ko_payout_in))
#define NUM_PAYOUTS 29

struct ko_payout_out ko_payouts_out[NUM_PAYOUTS];

int compare(const void *elem1,const void *elem2);

int main(int argc,char **argv)
{
  int m;
  int n;
  int p;
  int curr_arg;
  int payout;
  bool bSort;
  bool bDelta;
  bool bPlace;
  bool bNoEntryFee;
  int vals[NUM_PAYOUTS];
  int ixs[NUM_PAYOUTS];

  bSort = false;
  bDelta = false;
  bPlace = false;
  bNoEntryFee = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-sort"))
      bSort = true;
    else if (!strcmp(argv[curr_arg],"-delta"))
      bDelta = true;
    else if (!strcmp(argv[curr_arg],"-place"))
      bPlace = true;
    else if (!strcmp(argv[curr_arg],"-no_entry_fee"))
      bNoEntryFee = true;
    else
      break;
  }

  p = 0;

  for (m = 0; m < NUM_BASES; m++) {
    for (n = ko_payouts_in[m].max_bounties; n >= ko_payouts_in[m].min_bounties; n--) {
      ko_payouts_out[p].base_payout =
        (bNoEntryFee ? ko_payouts_no_entry_fee_in[m].base_payout : ko_payouts_in[m].base_payout);
      ko_payouts_out[p].place = ko_payouts_in[m].place;
      ko_payouts_out[p].num_bounties = n;
      ko_payouts_out[p].total_payout = ko_payouts_in[m].base_payout +
        n * (bNoEntryFee ? BOUNTY_NO_ENTRY_FEE : BOUNTY);

      if (bDelta)
        ko_payouts_out[p].total_payout -= BUY_IN;

      p++;
    }
  }

  for (n = 0; n < NUM_PAYOUTS; n++)
    ixs[n] = n;

  if (bSort)
    qsort(ixs,NUM_PAYOUTS,sizeof (int),compare);

  for (n = 0; n < NUM_PAYOUTS; n++) {
    printf("%d %d %d\n",
      ko_payouts_out[ixs[n]].total_payout,
      (bPlace ? ko_payouts_out[ixs[n]].place : ko_payouts_out[ixs[n]].base_payout),
      ko_payouts_out[ixs[n]].num_bounties);
  }

  return 0;
}

int compare(const void *elem1,const void *elem2)
{
  int ix1;
  int ix2;
  int int1;
  int int2;

  ix1 = *(int *)elem1;
  ix2 = *(int *)elem2;

  int1 = ko_payouts_out[ix1].total_payout;
  int2 = ko_payouts_out[ix2].total_payout;

  return int2 - int1;
}
