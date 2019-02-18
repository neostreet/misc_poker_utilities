#include <stdio.h>

static int ko_payouts[] {
  95625,
  57375,
  38250
};
#define NUM_PAYING_PLACES (sizeof ko_payouts / sizeof (int))

int main(int argc,char **argv)
{
  int n;
  int total_payouts;
  double dwork;

  total_payouts = 0;

  for (n = 0; n < NUM_PAYING_PLACES; n++)
    total_payouts += ko_payouts[n];

  for (n = 0; n < NUM_PAYING_PLACES; n++) {
    dwork = (double)ko_payouts[n] / (double)total_payouts;
    printf("%d %lf\n",ko_payouts[n],dwork);
  }

  return 0;
}
