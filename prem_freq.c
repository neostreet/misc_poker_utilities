#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#define MAIN_MODULE
#include "poker_hand.h"

static char usage[] =
"usage: prem_freq total_hands total_premium_hands\n";
static char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int total_hands;
  int total_premium_hands;
  double ratio1;
  double ratio2;
  double dwork;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&total_hands);
  sscanf(argv[2],"%d",&total_premium_hands);

  ratio1 = (double)total_premium_hands / (double)total_hands;
  ratio2 = (double)NUM_PREMIUM_HANDS_IN_DECK / (double)POKER_52_2_PERMUTATIONS;

  dwork = ratio1 / ratio2;
  printf("%lf (%d %d)\n",dwork,total_hands,total_premium_hands);

  return 0;
}
