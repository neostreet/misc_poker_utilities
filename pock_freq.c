#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#define MAIN_MODULE
#include "poker_hand.h"

static char usage[] =
"usage: pock_freq (-specific) total_hands total_pocks\n";
static char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int curr_arg;
  bool bSpecific;
  int total_hands;
  int total_pocks;
  double ratio1;
  double ratio2;
  double dwork;

  if ((argc < 3) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bSpecific = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-specific"))
      bSpecific = true;
    else
      break;
  }

  if (argc - curr_arg != 2) {
    printf(usage);
    return 2;
  }

  sscanf(argv[curr_arg],"%d",&total_hands);
  sscanf(argv[curr_arg+1],"%d",&total_pocks);

  ratio1 = (double)total_pocks / (double)total_hands;

  if (!bSpecific)
    ratio2 = (double)NUM_POCKS_IN_DECK / (double)POKER_52_2_PERMUTATIONS;
  else
    ratio2 = (double)NUM_POCKS_PER_DENOM / (double)POKER_52_2_PERMUTATIONS;

  dwork = ratio1 / ratio2;

  printf("%lf (%d %d)\n",dwork,total_hands,total_pocks);

  return 0;
}
